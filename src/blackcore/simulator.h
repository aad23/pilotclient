/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKCORE_SIMULATOR_H
#define BLACKCORE_SIMULATOR_H

#include "blacksim/simulatorinfo.h"
#include "blackmisc/simulation/aircraftmodellist.h"
#include "blackmisc/statusmessagelist.h"
#include "blackmisc/simulation/simulatedaircraftlist.h"
#include "blackmisc/avairportlist.h"
#include "blackmisc/nwtextmessage.h"
#include "blackmisc/nwclient.h"
#include "blackmisc/pixmap.h"
#include <QObject>

namespace BlackCore
{
    //! Interface to a simulator.
    class ISimulator : public QObject
    {
        Q_OBJECT
        Q_ENUMS(ConnectionStatus)

    public:
        //! ISimulator connection
        enum ConnectionStatus
        {
            Disconnected,
            Connected,
            ConnectionFailed
        };

        //! Constructor
        ISimulator(QObject *parent = nullptr);

        //! Destructor
        virtual ~ISimulator() {}

        //! Are we connected to the simulator?
        virtual bool isConnected() const = 0;

        //! Can we connect?
        virtual bool canConnect() const = 0;

        //! Is time synchronization on?
        virtual bool isTimeSynchronized() const = 0;

        //! Simulator paused?
        virtual bool isPaused() const = 0;

        //! Simulator running?
        virtual bool isSimulating() const = 0;

    public slots:

        //! Connect to simulator
        virtual bool connectTo() = 0;

        //! Connect asynchronously to simulator
        virtual void asyncConnectTo() = 0;

        //! Disconnect from simulator
        virtual bool disconnectFrom() = 0;

        //! Return user aircraft object
        virtual BlackMisc::Simulation::CSimulatedAircraft getOwnAircraft() const = 0;

        //! Add new remote aircraft to the simulator
        virtual void addRemoteAircraft(const BlackMisc::Simulation::CSimulatedAircraft &remoteAircraft) = 0;

        //! Simulated other aircraft in range
        virtual BlackMisc::Simulation::CSimulatedAircraftList getRemoteAircraft() const = 0;

        //! Add new aircraft situation
        virtual void addAircraftSituation(const BlackMisc::Aviation::CCallsign &callsign, const BlackMisc::Aviation::CAircraftSituation &situation) = 0;

        //! Remove remote aircraft from simulator
        virtual int removeRemoteAircraft(const BlackMisc::Aviation::CCallsign &callsign) = 0;

        //! Change remote aircraft per property
        virtual int changeRemoteAircraft(const BlackMisc::Simulation::CSimulatedAircraft &toChangeAircraft, const BlackMisc::CPropertyIndexVariantMap &changeValues) = 0;

        //! Update own aircraft cockpit (usually from context)
        virtual bool updateOwnSimulatorCockpit(const BlackMisc::Aviation::CAircraft &aircraft) = 0;

        //! Simulator info
        virtual BlackSim::CSimulatorInfo getSimulatorInfo() const = 0;

        //! Display a status message in the simulator
        virtual void displayStatusMessage(const BlackMisc::CStatusMessage &message) const = 0;

        //! Display a text message
        virtual void displayTextMessage(const BlackMisc::Network::CTextMessage &message) const = 0;

        //! Own aircraft Model
        virtual BlackMisc::Simulation::CAircraftModel getOwnAircraftModel() const = 0;

        //! Aircraft models for available remote aircrafts
        virtual BlackMisc::Simulation::CAircraftModelList getInstalledModels() const = 0;

        //! Airports in range
        virtual BlackMisc::Aviation::CAirportList getAirportsInRange() const = 0;

        //! Set time synchronization between simulator and user's computer time
        //! \remarks not all drivers implement this, e.g. if it is an intrinsic simulator feature
        virtual void setTimeSynchronization(bool enable, BlackMisc::PhysicalQuantities::CTime offset) = 0;

        //! Time synchronization offset
        virtual BlackMisc::PhysicalQuantities::CTime getTimeSynchronizationOffset() const = 0;

        //! Representing icon for model string
        virtual BlackMisc::CPixmap iconForModel(const QString &modelString) const = 0;

    signals:
        //! Emitted when the connection status has changed
        void connectionStatusChanged(ISimulator::ConnectionStatus status);

        //! Emitted when own aircraft model has changed
        void ownAircraftModelChanged(BlackMisc::Simulation::CSimulatedAircraft aircraft);

        //! Simulator combined status
        void simulatorStatusChanged(bool connected, bool running, bool paused);

        //! Simulator started
        void simulatorStarted();

        //! Simulator stopped;
        void simulatorStopped();

        //! A single model has been matched
        void modelMatchingCompleted(BlackMisc::Simulation::CSimulatedAircraft aircraft);

        //! Installed aircraft models ready or changed
        void installedAircraftModelsChanged();

    protected:
        //! Emit the combined status
        //! \sa simulatorStatusChanged;
        void emitSimulatorCombinedStatus();

    };

    //! Factory pattern class to create instances of ISimulator
    class ISimulatorFactory
    {
    public:

        //! ISimulatorVirtual destructor
        virtual ~ISimulatorFactory() {}

        //! Create a new instance
        virtual ISimulator *create(QObject *parent = nullptr) = 0;

        //! Simulator info
        virtual BlackSim::CSimulatorInfo getSimulatorInfo() const = 0;
    };

} // namespace BlackCore

// TODO: Use CProject to store this string
Q_DECLARE_INTERFACE(BlackCore::ISimulatorFactory, "net.vatsim.PilotClient.BlackCore.SimulatorInterface")
Q_DECLARE_METATYPE(BlackCore::ISimulator::ConnectionStatus)

#endif // guard
