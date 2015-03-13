/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "simulator_fsx.h"
#include "simconnect_datadefinition.h"
#include "blacksim/fscommon/bcdconversions.h"
#include "blacksim/fsx/simconnectutilities.h"
#include "blacksim/fsx/fsxsimulatorsetup.h"
#include "blacksim/simulatorplugininfo.h"
#include "blackmisc/simulation/aircraftmodel.h"
#include "blackmisc/project.h"
#include "blackmisc/avairportlist.h"
#include "blackmisc/logmessage.h"
#include "blackmisc/nwaircraftmappinglist.h"
#include "blackcore/interpolator_linear.h"

#include <QTimer>
#include <type_traits>

using namespace BlackMisc;
using namespace BlackMisc::Aviation;
using namespace BlackMisc::PhysicalQuantities;
using namespace BlackMisc::Geo;
using namespace BlackMisc::Network;
using namespace BlackMisc::Simulation;
using namespace BlackSim;
using namespace BlackSim::FsCommon;
using namespace BlackSim::Fsx;
using namespace BlackCore;

namespace BlackSimPlugin
{
    namespace Fsx
    {
        CSimulatorFsx::CSimulatorFsx(IOwnAircraftProvider *ownAircraftProvider, IRemoteAircraftProvider *remoteAircraftProvider, QObject *parent) :
            CSimulatorFsCommon(CSimulatorPluginInfo::FSX(), ownAircraftProvider, remoteAircraftProvider, parent)
        {
            Q_ASSERT(ownAircraftProvider);
            Q_ASSERT(remoteAircraftProvider);
            CFsxSimulatorSetup setup;
            setup.init(); // this fetches important settings on local side
            m_useFsuipc = false; // do not use FSUIPC at the moment with FSX
            this->m_interpolator = new CInterpolatorLinear(remoteAircraftProvider, this);
            this->m_interpolator->start();
            this->m_simulatorInfo.setSimulatorSetup(setup.getSettings());
        }

        CSimulatorFsx::~CSimulatorFsx()
        {
            disconnectFrom();
        }

        bool CSimulatorFsx::isConnected() const
        {
            return m_simConnected;
        }

        bool CSimulatorFsx::isSimulating() const
        {
            return m_simRunning;
        }

        bool CSimulatorFsx::connectTo()
        {
            if (m_simConnected) { return true; }
            if (FAILED(SimConnect_Open(&m_hSimConnect, BlackMisc::CProject::systemNameAndVersionChar(), nullptr, 0, 0, 0)))
            {
                emitSimulatorCombinedStatus();
                return false;
            }
            else
            {
                if (m_useFsuipc) { this->m_fsuipc->connect(); } // connect FSUIPC too
            }

            initWhenConnected();
            m_simconnectTimerId = startTimer(10);
            m_simConnected = true;

            emitSimulatorCombinedStatus();
            return true;
        }

        void CSimulatorFsx::asyncConnectTo()
        {
            connect(&m_watcherConnect, SIGNAL(finished()), this, SLOT(ps_connectToFinished()));

            // simplified connect, timers and signals not in different thread
            auto asyncConnectFunc = [&]() -> bool
            {
                if (FAILED(SimConnect_Open(&m_hSimConnect, BlackMisc::CProject::systemNameAndVersionChar(), nullptr, 0, 0, 0))) { return false; }
                if (m_useFsuipc) { this->m_fsuipc->connect(); } // FSUIPC too
                return true;
            };

            QFuture<bool> result = QtConcurrent::run(asyncConnectFunc);
            m_watcherConnect.setFuture(result);
        }

        bool CSimulatorFsx::disconnectFrom()
        {
            if (!m_simConnected) { return true; }
            if (m_hSimConnect)
            {
                SimConnect_Close(m_hSimConnect);
                this->m_fsuipc->disconnect();
            }

            if (m_simconnectTimerId)
            {
                killTimer(m_simconnectTimerId);
            }

            m_hSimConnect = nullptr;
            m_simconnectTimerId = -1;
            m_simConnected = false;

            emitSimulatorCombinedStatus();
            return true;
        }

        bool CSimulatorFsx::canConnect() const
        {
            if (m_simConnected) { return true; }
            HANDLE hSimConnect; // temporary handle
            bool connect = SUCCEEDED(SimConnect_Open(&hSimConnect, BlackMisc::CProject::systemNameAndVersionChar(), nullptr, 0, 0, 0));
            SimConnect_Close(hSimConnect);
            return connect;
        }

        bool CSimulatorFsx::addRemoteAircraft(const Simulation::CSimulatedAircraft &newRemoteAircraft)
        {
            CCallsign callsign = newRemoteAircraft.getCallsign();
            Q_ASSERT(!callsign.isEmpty());
            if (callsign.isEmpty()) { return false; }

            bool aircraftAlreadyExistsInSim = this->m_simConnectObjects.contains(callsign);
            if (aircraftAlreadyExistsInSim)
            {
                // remove first
                this->removeRemoteAircraft(callsign);
                CLogMessage(this).warning("Have to remove aircraft %1 before I can add it") << callsign;
            }

            CSimulatedAircraft newRemoteAircraftCopy(newRemoteAircraft);
            this->setInitialAircraftSituationAndParts(newRemoteAircraftCopy);
            SIMCONNECT_DATA_INITPOSITION initialPosition = aircraftSituationToFsxInitPosition(newRemoteAircraftCopy.getSituation());

            CSimConnectObject simObj;
            simObj.setCallsign(callsign);
            simObj.setRequestId(m_nextObjID);
            simObj.setObjectId(0);
            ++m_nextObjID;

            // matched models
            CAircraftModel aircraftModel = modelMatching(newRemoteAircraftCopy);
            Q_ASSERT(newRemoteAircraft.getCallsign() == aircraftModel.getCallsign());
            this->providerUpdateAircraftModel(newRemoteAircraft.getCallsign(), aircraftModel, simulatorOriginator());
            CSimulatedAircraft aircraftAfterModelApplied = remoteAircraft().findFirstByCallsign(newRemoteAircraft.getCallsign());
            aircraftAfterModelApplied.setRendered(true);
            emit modelMatchingCompleted(aircraftAfterModelApplied);

            // create AI
            if (isSimulating())
            {
                //! \todo if exists, recreate (new model?, new ICAO code)
                QByteArray m = aircraftModel.getModelString().toLocal8Bit();
                HRESULT hr = SimConnect_AICreateNonATCAircraft(m_hSimConnect, m.constData(), qPrintable(callsign.toQString().left(12)), initialPosition, simObj.getRequestId());
                if (hr != S_OK) { CLogMessage(this).error("SimConnect, can not create AI traffic"); }
                m_simConnectObjects.insert(callsign, simObj);
                CLogMessage(this).info("FSX: Added aircraft %1") << callsign.toQString();
                return true;
            }
            else
            {
                CLogMessage(this).warning("FSX: Not connected, not added aircraft %1") << callsign.toQString();
                return false;
            }
        }

        bool CSimulatorFsx::updateOwnSimulatorCockpit(const CAircraft &ownAircraft, const QString &originator)
        {
            if (originator == this->simulatorOriginator()) { return false; }
            if (!this->isSimulating()) { return false; }

            // actually those data should be the same as ownAircraft
            CComSystem newCom1 = ownAircraft.getCom1System();
            CComSystem newCom2 = ownAircraft.getCom2System();
            CTransponder newTransponder = ownAircraft.getTransponder();

            bool changed = false;
            if (newCom1.getFrequencyActive() != this->m_simCom1.getFrequencyActive())
            {
                CFrequency newFreq = newCom1.getFrequencyActive();
                SimConnect_TransmitClientEvent(m_hSimConnect, 0, EventSetCom1Active,
                                               CBcdConversions::comFrequencyToBcdHz(newFreq), SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                changed = true;

            }
            if (newCom1.getFrequencyStandby() != this->m_simCom1.getFrequencyStandby())
            {
                CFrequency newFreq = newCom1.getFrequencyStandby();
                SimConnect_TransmitClientEvent(m_hSimConnect, 0, EventSetCom1Standby,
                                               CBcdConversions::comFrequencyToBcdHz(newFreq), SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                changed = true;
            }

            if (newCom2.getFrequencyActive() != this->m_simCom2.getFrequencyActive())
            {
                CFrequency newFreq = newCom2.getFrequencyActive();
                SimConnect_TransmitClientEvent(m_hSimConnect, 0, EventSetCom2Active,
                                               CBcdConversions::comFrequencyToBcdHz(newFreq), SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                changed = true;
            }
            if (newCom2.getFrequencyStandby() != this->m_simCom2.getFrequencyStandby())
            {
                CFrequency newFreq = newCom2.getFrequencyStandby();
                SimConnect_TransmitClientEvent(m_hSimConnect, 0, EventSetCom2Standby,
                                               CBcdConversions::comFrequencyToBcdHz(newFreq), SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                changed = true;
            }

            if (newTransponder.getTransponderCode() != this->m_simTransponder.getTransponderCode())
            {
                SimConnect_TransmitClientEvent(m_hSimConnect, 0, EventSetTransponderCode,
                                               CBcdConversions::transponderCodeToBcd(newTransponder), SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                changed = true;
            }

            if (newTransponder.getTransponderMode() != this->m_simTransponder.getTransponderMode())
            {
                if (m_useSbOffsets)
                {
                    byte ident = newTransponder.isIdentifying() ? 1 : 0; // 1 is ident
                    byte standby = newTransponder.isInStandby() ? 1 : 0; // 1 is standby
                    HRESULT hr = S_OK;

                    hr += SimConnect_SetClientData(m_hSimConnect, ClientAreaSquawkBox, CSimConnectDefinitions::DataClientAreaSbIdent, NULL, 0, 1, &ident);
                    hr += SimConnect_SetClientData(m_hSimConnect, ClientAreaSquawkBox, CSimConnectDefinitions::DataClientAreaSbStandby, NULL, 0, 1, &standby);
                    if (hr != S_OK)
                    {
                        CLogMessage(this).warning("Setting transponder mode failed (SB offsets)");
                    }
                }
                changed = true;
            }

            // avoid changes of cockpit back to old values due to an outdated read back value
            if (changed) { m_skipCockpitUpdateCycles = SkipUpdateCyclesForCockpit; }

            // bye
            return changed;
        }

        void CSimulatorFsx::displayStatusMessage(const BlackMisc::CStatusMessage &message) const
        {
            QByteArray m = message.getMessage().toLocal8Bit().constData();
            m.append('\0');

            SIMCONNECT_TEXT_TYPE type = SIMCONNECT_TEXT_TYPE_PRINT_BLACK;
            switch (message.getSeverity())
            {
            case CStatusMessage::SeverityDebug:
                return;
            case CStatusMessage::SeverityInfo:
                type = SIMCONNECT_TEXT_TYPE_PRINT_GREEN;
                break;
            case CStatusMessage::SeverityWarning:
                type = SIMCONNECT_TEXT_TYPE_PRINT_YELLOW;
                break;
            case CStatusMessage::SeverityError:
                type = SIMCONNECT_TEXT_TYPE_PRINT_RED;
                break;
            }
            HRESULT hr = SimConnect_Text(
                             m_hSimConnect, type, 7.5, EventTextMessage, m.size(),
                             m.data()
                         );
            Q_UNUSED(hr);
        }

        void CSimulatorFsx::displayTextMessage(const BlackMisc::Network::CTextMessage &message) const
        {
            this->displayStatusMessage(message.asStatusMessage(true, true));
        }

        bool CSimulatorFsx::isRenderedAircraft(const CCallsign &callsign) const
        {
            return this->m_simConnectObjects.contains(callsign);
        }

        void CSimulatorFsx::onSimRunning()
        {
            if (m_simRunning) { return; }
            m_simRunning = true;
            HRESULT hr = SimConnect_RequestDataOnSimObject(m_hSimConnect, CSimConnectDefinitions::RequestOwnAircraft,
                         CSimConnectDefinitions::DataOwnAircraft,
                         SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_VISUAL_FRAME);

            hr += SimConnect_RequestDataOnSimObject(m_hSimConnect, CSimConnectDefinitions::RequestOwnAircraftTitle,
                                                    CSimConnectDefinitions::DataOwnAircraftTitle,
                                                    SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SECOND,
                                                    SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);

            hr += SimConnect_RequestDataOnSimObject(m_hSimConnect, CSimConnectDefinitions::RequestSimEnvironment,
                                                    CSimConnectDefinitions::DataSimEnvironment,
                                                    SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SECOND,
                                                    SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);

            if (hr != S_OK)
            {
                CLogMessage(this).error("FSX plugin: SimConnect_RequestDataOnSimObject failed");
                return;
            }

            // Request the data from SB only when its changed and only ONCE so we don't have to run a 1sec event to get/set this info ;)
            hr += SimConnect_RequestClientData(m_hSimConnect, ClientAreaSquawkBox, CSimConnectDefinitions::RequestSbData,
                                               CSimConnectDefinitions::DataClientAreaSb, SIMCONNECT_CLIENT_DATA_PERIOD_SECOND, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED);

            if (hr != S_OK)
            {
                CLogMessage(this).error("FSX plugin: SimConnect_RequestClientData failed");
                return;
            }

            emitSimulatorCombinedStatus();
        }

        void CSimulatorFsx::onSimStopped()
        {
            if (m_simRunning) {
                m_simRunning = false;
                mapperInstance()->gracefulShutdown(); // stop background reading if ongoing
            }
            emitSimulatorCombinedStatus();
        }

        void CSimulatorFsx::onSimFrame()
        {
            updateRemoteAircraft();
        }

        void CSimulatorFsx::onSimExit()
        {
            m_simConnected = false;
            this->onSimStopped();
        }

        void CSimulatorFsx::updateOwnAircraftFromSimulator(DataDefinitionOwnAircraft simulatorOwnAircraft)
        {
            BlackMisc::Geo::CCoordinateGeodetic position;
            position.setLatitude(CLatitude(simulatorOwnAircraft.latitude, CAngleUnit::deg()));
            position.setLongitude(CLongitude(simulatorOwnAircraft.longitude, CAngleUnit::deg()));

            BlackMisc::Aviation::CAircraftSituation aircraftSituation;
            aircraftSituation.setPosition(position);
            aircraftSituation.setPitch(CAngle(simulatorOwnAircraft.pitch, CAngleUnit::deg()));
            aircraftSituation.setBank(CAngle(simulatorOwnAircraft.bank, CAngleUnit::deg()));
            aircraftSituation.setHeading(CHeading(simulatorOwnAircraft.trueHeading, CHeading::True, CAngleUnit::deg()));
            aircraftSituation.setGroundspeed(CSpeed(simulatorOwnAircraft.velocity, CSpeedUnit::kts()));
            aircraftSituation.setAltitude(CAltitude(simulatorOwnAircraft.altitude, CAltitude::MeanSeaLevel, CLengthUnit::ft()));
            ownAircraft().setSituation(aircraftSituation);

            CAircraftLights lights(simulatorOwnAircraft.lightStrobe,
                                   simulatorOwnAircraft.lightLanding,
                                   simulatorOwnAircraft.lightTaxi,
                                   simulatorOwnAircraft.lightBeacon,
                                   simulatorOwnAircraft.lightNav,
                                   simulatorOwnAircraft.lightLogo);

            QList<bool> helperList {    simulatorOwnAircraft.engine1Combustion != 0, simulatorOwnAircraft.engine2Combustion != 0,
                                        simulatorOwnAircraft.engine3Combustion != 0, simulatorOwnAircraft.engine4Combustion != 0
                                   };

            CAircraftEngineList engines;
            for (int index = 0; index < simulatorOwnAircraft.numberOfEngines; ++index)
            {
                engines.push_back(CAircraftEngine(index + 1, helperList.at(index)));
            }

            CAircraftParts parts(lights, simulatorOwnAircraft.gearHandlePosition,
                                 simulatorOwnAircraft.flapsHandlePosition * 100,
                                 simulatorOwnAircraft.spoilersHandlePosition,
                                 engines,
                                 simulatorOwnAircraft.simOnGround);

            ownAircraft().setParts(parts);

            CComSystem com1 = ownAircraft().getCom1System(); // set defaults
            CComSystem com2 = ownAircraft().getCom2System();
            CTransponder transponder = ownAircraft().getTransponder();

            // When I change cockpit values in the sim (from GUI to simulator, not originating from simulator)
            // it takes a little while before these values are set in the simulator.
            // To avoid jitters, I wait some update cylces to stabilize the values
            if (m_skipCockpitUpdateCycles < 1)
            {
                com1.setFrequencyActive(CFrequency(simulatorOwnAircraft.com1ActiveMHz, CFrequencyUnit::MHz()));
                com1.setFrequencyStandby(CFrequency(simulatorOwnAircraft.com1StandbyMHz, CFrequencyUnit::MHz()));
                bool changedCom1 = ownAircraft().getCom1System() != com1;
                this->m_simCom1 = com1;

                com2.setFrequencyActive(CFrequency(simulatorOwnAircraft.com2ActiveMHz, CFrequencyUnit::MHz()));
                com2.setFrequencyStandby(CFrequency(simulatorOwnAircraft.com2StandbyMHz, CFrequencyUnit::MHz()));
                bool changedCom2 = ownAircraft().getCom2System() != com2;
                this->m_simCom2 = com2;

                transponder.setTransponderCode(simulatorOwnAircraft.transponderCode);
                bool changedXpr = (ownAircraft().getTransponderCode() != transponder.getTransponderCode());

                if (changedCom1 || changedCom2 || changedXpr)
                {
                    this->providerUpdateCockpit(com1, com2, transponder, simulatorOriginator());
                }
            }
            else
            {
                --m_skipCockpitUpdateCycles;
            }
        }

        void CSimulatorFsx::updateOwnAircraftFromSimulator(DataDefinitionClientAreaSb sbDataArea)
        {
            CTransponder::TransponderMode newMode;
            if (sbDataArea.isIdent())
            {
                newMode = CTransponder::StateIdent;
            }
            else
            {
                newMode = sbDataArea.isStandby() ? CTransponder::StateStandby : CTransponder::ModeC;
            }
            bool changed = (this->ownAircraft().getTransponderMode() != newMode);
            if (!changed) { return; }
            CTransponder xpdr = this->ownAircraft().getTransponder();
            xpdr.setTransponderMode(newMode);
            this->providerUpdateCockpit(ownAircraft().getCom1System(), ownAircraft().getCom2System(), xpdr, this->simulatorOriginator());
        }

        void CSimulatorFsx::setSimConnectObjectID(DWORD requestID, DWORD objectID)
        {
            // First check, if this request id belongs to us
            auto it = std::find_if(m_simConnectObjects.begin(), m_simConnectObjects.end(),
            [requestID](const CSimConnectObject & obj) { return obj.getRequestId() == static_cast<int>(requestID); });
            if (it == m_simConnectObjects.end()) { return; }

            // belongs to us
            it->setObjectId(objectID);
            SimConnect_AIReleaseControl(m_hSimConnect, objectID, requestID);
            SimConnect_TransmitClientEvent(m_hSimConnect, objectID, EventFreezeLat, 1,
                                           SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
            SimConnect_TransmitClientEvent(m_hSimConnect, objectID, EventFreezeAlt, 1,
                                           SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
            SimConnect_TransmitClientEvent(m_hSimConnect, objectID, EventFreezeAtt, 1,
                                           SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        }

        void CSimulatorFsx::timerEvent(QTimerEvent *event)
        {
            Q_UNUSED(event);
            ps_dispatch();
        }

        void CSimulatorFsx::ps_dispatch()
        {
            SimConnect_CallDispatch(m_hSimConnect, SimConnectProc, this);
            if (m_useFsuipc && m_fsuipc)
            {
                CSimulatedAircraft fsuipcAircraft(ownAircraft());
                //! \todo split in high / low frequency reads
                bool ok = m_fsuipc->read(fsuipcAircraft, true, true, true);
                if (ok)
                {
                    // do whatever is required
                    Q_UNUSED(fsuipcAircraft);
                }
            }
        }

        void CSimulatorFsx::ps_connectToFinished()
        {
            if (m_watcherConnect.result())
            {
                initEvents();
                initDataDefinitionsWhenConnected();
                m_simconnectTimerId = startTimer(10);
                m_simConnected = true;

                emitSimulatorCombinedStatus();
            }
            else
            {
                m_simConnected = false;
                emitSimulatorCombinedStatus();
            }
        }

        bool CSimulatorFsx::removeRemoteAircraft(const CCallsign &callsign)
        {
            // only remove from sim
            if (!m_simConnectObjects.contains(callsign)) { return false; }
            return removeRemoteAircraft(m_simConnectObjects.value(callsign));
        }

        void CSimulatorFsx::removeAllRemoteAircraft()
        {
            QList<CCallsign> callsigns(m_simConnectObjects.keys());
            for (const CCallsign &cs : callsigns)
            {
                removeRemoteAircraft(cs);
            }
        }

        bool CSimulatorFsx::removeRemoteAircraft(const CSimConnectObject &simObject)
        {
            m_simConnectObjects.remove(simObject.getCallsign());
            SimConnect_AIRemoveObject(m_hSimConnect, simObject.getObjectId(), simObject.getRequestId());
            remoteAircraft().setRendered(simObject.getCallsign(), false);
            CLogMessage(this).info("FSX: Removed aircraft %1") << simObject.getCallsign().toQString();
            return true;
        }

        HRESULT CSimulatorFsx::initEvents()
        {
            HRESULT hr = S_OK;
            // System events, see http://msdn.microsoft.com/en-us/library/cc526983.aspx#SimConnect_SubscribeToSystemEvent
            hr += SimConnect_SubscribeToSystemEvent(m_hSimConnect, SystemEventSimStatus, "Sim");
            hr += SimConnect_SubscribeToSystemEvent(m_hSimConnect, SystemEventObjectAdded, "ObjectAdded");
            hr += SimConnect_SubscribeToSystemEvent(m_hSimConnect, SystemEventObjectRemoved, "ObjectRemoved");
            hr += SimConnect_SubscribeToSystemEvent(m_hSimConnect, SystemEventFrame, "Frame");
            hr += SimConnect_SubscribeToSystemEvent(m_hSimConnect, SystemEventPause, "Pause");
            if (hr != S_OK)
            {
                CLogMessage(this).error("FSX plugin error: %1") << "SimConnect_SubscribeToSystemEvent failed";
                return hr;
            }

            // Mapped events, see event ids here: http://msdn.microsoft.com/en-us/library/cc526980.aspx
            hr += SimConnect_MapClientEventToSimEvent(m_hSimConnect, EventPauseToggle, "PAUSE_TOGGLE");
            hr += SimConnect_MapClientEventToSimEvent(m_hSimConnect, SystemEventSlewToggle, "SLEW_TOGGLE");
            hr += SimConnect_MapClientEventToSimEvent(m_hSimConnect, EventFreezeLat, "FREEZE_LATITUDE_LONGITUDE_SET");
            hr += SimConnect_MapClientEventToSimEvent(m_hSimConnect, EventFreezeAlt, "FREEZE_ALTITUDE_SET");
            hr += SimConnect_MapClientEventToSimEvent(m_hSimConnect, EventFreezeAtt, "FREEZE_ATTITUDE_SET");
            hr += SimConnect_MapClientEventToSimEvent(m_hSimConnect, EventSetCom1Active, "COM_RADIO_SET");
            hr += SimConnect_MapClientEventToSimEvent(m_hSimConnect, EventSetCom1Standby, "COM_STBY_RADIO_SET");
            hr += SimConnect_MapClientEventToSimEvent(m_hSimConnect, EventSetCom2Active, "COM2_RADIO_SET");
            hr += SimConnect_MapClientEventToSimEvent(m_hSimConnect, EventSetCom2Standby, "COM2_STBY_RADIO_SET");
            hr += SimConnect_MapClientEventToSimEvent(m_hSimConnect, EventSetTransponderCode, "XPNDR_SET");

            hr += SimConnect_MapClientEventToSimEvent(m_hSimConnect, EventSetTimeZuluYear, "ZULU_YEAR_SET");
            hr += SimConnect_MapClientEventToSimEvent(m_hSimConnect, EventSetTimeZuluDay, "ZULU_DAY_SET");
            hr += SimConnect_MapClientEventToSimEvent(m_hSimConnect, EventSetTimeZuluHours, "ZULU_HOURS_SET");
            hr += SimConnect_MapClientEventToSimEvent(m_hSimConnect, EventSetTimeZuluMinutes, "ZULU_MINUTES_SET");

            hr += SimConnect_MapClientEventToSimEvent(m_hSimConnect, EventToggleTaxiLights, "TOGGLE_TAXI_LIGHTS");

            if (hr != S_OK)
            {
                CLogMessage(this).error("FSX plugin error: %1") << "SimConnect_MapClientEventToSimEvent failed";
                return hr;
            }

            // facility
            hr += SimConnect_SubscribeToFacilities(m_hSimConnect, SIMCONNECT_FACILITY_LIST_TYPE_AIRPORT, m_nextObjID++);
            if (hr != S_OK)
            {
                CLogMessage(this).error("FSX plugin error: %1") << "SimConnect_SubscribeToFacilities failed";
                return hr;
            }
            return hr;
        }

        HRESULT CSimulatorFsx::initDataDefinitionsWhenConnected()
        {
            return CSimConnectDefinitions::initDataDefinitionsWhenConnected(m_hSimConnect);
        }

        HRESULT CSimulatorFsx::initWhenConnected()
        {
            // called when connected

            HRESULT hr = initEvents();
            if (hr != S_OK)
            {
                CLogMessage(this).error("FSX plugin: initEvents failed");
                return hr;
            }

            // inti data definitions and SB data area
            hr += initDataDefinitionsWhenConnected();
            if (hr != S_OK)
            {
                CLogMessage(this).error("FSX plugin: initDataDefinitionsWhenConnected failed");
                return hr;
            }

            return hr;
        }

        void CSimulatorFsx::updateRemoteAircraft()
        {
            static_assert(sizeof(DataDefinitionRemoteAircraftParts) == 120, "DataDefinitionRemoteAircraftParts has an incorrect size.");
            Q_ASSERT(this->m_interpolator);
            Q_ASSERT_X(this->m_interpolator->thread() != this->thread(), "updateOtherAircraft", "interpolator should run in its own thread");

            // nothing to do, reset request id and exit
            if (this->isPaused()) { return; } // no interpolation while paused
            int remoteAircraftNo = this->remoteAircraft().size();
            if (remoteAircraftNo < 1) { m_interpolationRequest = 0;  return; }

            // interpolate and send to SIM
            m_interpolationRequest++;

            // values used for position and parts
            bool isOnGround = false;

            qint64 currentTimestamp = QDateTime::currentMSecsSinceEpoch();
            for (const CSimConnectObject &simObj : m_simConnectObjects)
            {
                const CCallsign callsign(simObj.getCallsign());
                IInterpolator::InterpolationStatus interpolatorStatus;
                if (simObj.getObjectId() == 0) { continue; }
                CAircraftSituation interpolatedSituation = this->m_interpolator->getInterpolatedSituation(callsign, currentTimestamp, interpolatorStatus);

                // having the onGround flag in parts forces me to obtain parts here
                // which is not the smartest thing regarding performance
                IInterpolator::PartsStatus partsStatus;
                CAircraftPartsList parts = this->m_interpolator->getAndRemovePartsBeforeTime(callsign, currentTimestamp - IInterpolator::TimeOffsetMs, partsStatus);

                if (interpolatorStatus.allTrue())
                {
                    // update situation
                    SIMCONNECT_DATA_INITPOSITION position = aircraftSituationToFsxInitPosition(interpolatedSituation);

                    //! \todo The onGround in parts is nuts, as already mentioned in the discussion
                    // a) I am forced to read parts even if i just want to update position
                    // b) Unlike the other values it is not a fire and forget value, as I need it again in the next cycle
                    if (partsStatus.supportsParts && !parts.isEmpty())
                    {
                        // we have parts, and use the closest ground
                        isOnGround = parts.front().isOnGround();
                    }
                    else
                    {
                        isOnGround = interpolatedSituation.isOnGroundGuessed();
                    }

                    position.OnGround = isOnGround ? 1 : 0;
                    HRESULT hr = S_OK;
                    hr += SimConnect_SetDataOnSimObject(m_hSimConnect, CSimConnectDefinitions::DataRemoteAircraftPosition,
                                                        simObj.getObjectId(), 0, 0,
                                                        sizeof(SIMCONNECT_DATA_INITPOSITION), &position);
                    if (hr != S_OK) { CLogMessage(this).warning("Failed so set position on SimObject %1 callsign: %2") << simObj.getObjectId() << callsign; }



                } // interpolation data

                if (interpolatorStatus.interpolationSucceeded)
                {
                    // aircraft parts
                    // inside "interpolator if", as no parts can be sent without position
                    updateRemoteAircraftParts(simObj, parts, partsStatus, interpolatedSituation, isOnGround); // update and retrieve parts in the same step
                }

            } // all callsigns
            qint64 dt = QDateTime::currentMSecsSinceEpoch() - currentTimestamp;
            m_statsUpdateAircraftTimeTotal += dt;
            m_statsUpdateAircraftCount++;
            m_statsUpdateAircraftTimeAvg = m_statsUpdateAircraftTimeTotal / m_statsUpdateAircraftCount;
        }

        bool CSimulatorFsx::updateRemoteAircraftParts(const CSimConnectObject &simObj, const CAircraftPartsList &parts, IInterpolator::PartsStatus partsStatus, const CAircraftSituation &interpolatedSituation, bool isOnGround) const
        {
            // set parts
            DataDefinitionRemoteAircraftParts ddRemoteAircraftParts;
            if (partsStatus.supportsParts)
            {
                // parts is supported, but do we need to update?
                if (parts.isEmpty()) { return false; }

                // we have parts
                CAircraftParts newestParts = parts.front();
                ddRemoteAircraftParts.lightStrobe = newestParts.getLights().isStrobeOn() ? 1.0 : 0.0;
                ddRemoteAircraftParts.lightLanding = newestParts.getLights().isLandingOn() ? 1.0 : 0.0;
                // ddRemoteAircraftParts.lightTaxi = newestParts.getLights().isTaxiOn() ? 1.0 : 0.0;
                ddRemoteAircraftParts.lightBeacon = newestParts.getLights().isBeaconOn() ? 1.0 : 0.0;
                ddRemoteAircraftParts.lightNav = newestParts.getLights().isNavOn() ? 1.0 : 0.0;
                ddRemoteAircraftParts.lightLogo = newestParts.getLights().isLogoOn() ? 1.0 : 0.0;
                ddRemoteAircraftParts.flapsLeadingEdgeLeftPercent = newestParts.getFlapsPercent() / 100.0;
                ddRemoteAircraftParts.flapsLeadingEdgeRightPercent = newestParts.getFlapsPercent() / 100.0;
                ddRemoteAircraftParts.flapsTrailingEdgeLeftPercent = newestParts.getFlapsPercent() / 100.0;
                ddRemoteAircraftParts.flapsTrailingEdgeRightPercent = newestParts.getFlapsPercent() / 100.0;
                ddRemoteAircraftParts.spoilersHandlePosition = newestParts.isSpoilersOut() ? 1.0 : 0.0;
                ddRemoteAircraftParts.gearHandlePosition = newestParts.isGearDown() ? 1 : 0;
                ddRemoteAircraftParts.engine1Combustion = newestParts.isEngineOn(1) ? 1 : 0;
                ddRemoteAircraftParts.engine2Combustion = newestParts.isEngineOn(2) ? 1 : 0;;
                ddRemoteAircraftParts.engine3Combustion = newestParts.isEngineOn(3) ? 1 : 0;
                ddRemoteAircraftParts.engine4Combustion = newestParts.isEngineOn(4) ? 1 : 0;
            }
            else
            {
                // mode is guessing parts
                if (this->m_interpolationRequest % 20 != 0) { return false; } // only update every 20th cycle
                ddRemoteAircraftParts.gearHandlePosition = isOnGround ? 1 : 0;

                // when first detected moving, lights on
                if (isOnGround)
                {
                    // ddRemoteAircraftParts.lightTaxi = 1.0;
                    ddRemoteAircraftParts.lightBeacon = 1.0;
                    ddRemoteAircraftParts.lightNav = 1.0;

                    double gskmh = interpolatedSituation.getGroundSpeed().value(CSpeedUnit::km_h());
                    if (gskmh > 7.5)
                    {
                        // mode taxi
                        // ddRemoteAircraftParts.lightTaxi = 1.0;
                        ddRemoteAircraftParts.lightLanding = 0.0;
                    }
                    else if (gskmh > 25)
                    {
                        // mode accelaration for takeoff
                        // ddRemoteAircraftParts.lightTaxi = 0.0;
                        ddRemoteAircraftParts.lightLanding = 1.0;
                    }
                    else
                    {
                        // slow movements or parking
                        // ddRemoteAircraftParts.lightTaxi = 0.0;
                        ddRemoteAircraftParts.lightLanding = 0.0;
                    }
                }
                else
                {
                    // ddRemoteAircraftParts.lightTaxi = 0.0;
                    ddRemoteAircraftParts.lightBeacon = 1.0;
                    ddRemoteAircraftParts.lightNav = 1.0;
                    // landing lights for < 10000ft (normally MSL, here ignored)
                    ddRemoteAircraftParts.lightLanding = (interpolatedSituation.getAltitude().value(CLengthUnit::ft()) < 10000) ? 1.0 : 0;
                }
            }

            Q_ASSERT(m_hSimConnect);
            HRESULT hr = S_OK;
            hr += SimConnect_SetDataOnSimObject(m_hSimConnect, CSimConnectDefinitions::DataRemoteAircraftParts,
                                                simObj.getObjectId(), 0, 0,
                                                sizeof(DataDefinitionRemoteAircraftParts), &ddRemoteAircraftParts);

            if (hr != S_OK) { CLogMessage(this).warning("Failed so set parts on SimObject %1 callsign: %2") << simObj.getObjectId() << simObj.getCallsign(); }
            return hr == S_OK;
        }

        SIMCONNECT_DATA_INITPOSITION CSimulatorFsx::aircraftSituationToFsxInitPosition(const CAircraftSituation &situation)
        {
            SIMCONNECT_DATA_INITPOSITION position;
            position.Latitude = situation.latitude().value();
            position.Longitude = situation.longitude().value();
            position.Altitude = situation.getAltitude().value(CLengthUnit::ft());
            position.Pitch = situation.getPitch().value();
            position.Bank = situation.getBank().value();
            position.Heading = situation.getHeading().value(CAngleUnit::deg());
            position.Airspeed = situation.getGroundSpeed().value(CSpeedUnit::kts());
            return position;
        }

        void CSimulatorFsx::synchronizeTime(const CTime &zuluTimeSim, const CTime &localTimeSim)
        {
            if (!this->m_simTimeSynced) { return; }
            if (!this->isConnected())   { return; }
            if (m_syncDeferredCounter > 0)
            {
                --m_syncDeferredCounter;
            }
            Q_UNUSED(localTimeSim);

            QDateTime myDateTime = QDateTime::currentDateTimeUtc();
            if (!this->m_syncTimeOffset.isZeroEpsilonConsidered())
            {
                int offsetSeconds = this->m_syncTimeOffset.valueRounded(CTimeUnit::s(), 0);
                myDateTime = myDateTime.addSecs(offsetSeconds);
            }
            QTime myTime = myDateTime.time();
            DWORD h = myTime.hour();
            DWORD m = myTime.minute();
            int targetMins = myTime.hour() * 60 + myTime.minute();
            int simMins = zuluTimeSim.valueRounded(CTimeUnit::min());
            int diffMins = qAbs(targetMins - simMins);
            if (diffMins < 2) { return; }
            HRESULT hr = S_OK;
            hr += SimConnect_TransmitClientEvent(m_hSimConnect, 0, EventSetTimeZuluHours, h, SIMCONNECT_GROUP_PRIORITY_STANDARD, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
            hr += SimConnect_TransmitClientEvent(m_hSimConnect, 0, EventSetTimeZuluMinutes, m, SIMCONNECT_GROUP_PRIORITY_STANDARD, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);

            if (hr != S_OK)
            {
                CLogMessage(this).warning("Sending time sync failed!");
            }
            else
            {
                m_syncDeferredCounter = 5; // allow some time to sync
                CLogMessage(this).info("Synchronized time to UTC: %1") << myTime.toString();
            }
        }

        CSimulatorFsxListener::CSimulatorFsxListener(QObject *parent) : ISimulatorListener(parent),
            m_timer(new QTimer(this))
        {
            Q_CONSTEXPR int QueryInterval = 5 * 1000; // 5 seconds
            m_timer->setInterval(QueryInterval);

            connect(m_timer, &QTimer::timeout, [this]() {
                HANDLE hSimConnect;
                HRESULT result = SimConnect_Open(&hSimConnect, BlackMisc::CProject::systemNameAndVersionChar(), nullptr, 0, 0, 0);
                SimConnect_Close(hSimConnect);

                if (result == S_OK)
                    emit simulatorStarted(m_simulatorInfo);
            });
        }

        void CSimulatorFsxListener::start()
        {
            m_timer->start();
        }

        void CSimulatorFsxListener::stop()
        {
            m_timer->stop();
        }
    } // namespace
} // namespace
