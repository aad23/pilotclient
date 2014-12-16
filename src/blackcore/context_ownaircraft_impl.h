/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKCORE_CONTEXTOWNAIRCRAFT_IMPL_H
#define BLACKCORE_CONTEXTOWNAIRCRAFT_IMPL_H

#include "blackcore/context_ownaircraft.h"
#include "blackcore/context_settings.h"
#include "blackcore/context_runtime.h"
#include "blackcore/dbus_server.h"

namespace BlackCore
{

    //! Network context implementation
    class CContextOwnAircraft : public IContextOwnAircraft
    {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", BLACKCORE_CONTEXTOWNAIRCRAFT_INTERFACENAME)
        friend class CRuntime;
        friend class IContextOwnAircraft;

    public:
        //! Destructor
        virtual ~CContextOwnAircraft();

        //! Own aircraft
        const BlackMisc::Aviation::CAircraft &ownAircraft() const { return this->m_ownAircraft; }

        //! Own aircraft
        BlackMisc::Aviation::CAircraft &ownAircraft() { return this->m_ownAircraft; }

    public slots: // IContextOwnAircraft overrides

        //! \copydoc IContextOwnAircraft::getOwnAircraft()
        virtual BlackMisc::Aviation::CAircraft getOwnAircraft() const override;

        //! \copydoc IContextOwnAircraft::setOwnAircraft
        virtual void updateOwnAircraft(const BlackMisc::Aviation::CAircraft &aircraft, const QString &originator) override;

        //! \copydoc IContextOwnAircraft::updateOwnPosition
        virtual bool updateOwnPosition(const BlackMisc::Geo::CCoordinateGeodetic &position, const BlackMisc::Aviation::CAltitude &altitude, const QString &originator) override;

        //! \copydoc IContextOwnAircraft::updateOwnSituation
        virtual bool updateOwnSituation(const BlackMisc::Aviation::CAircraftSituation &situation, const QString &originator) override;

        //! \copydoc IContextOwnAircraft::updateOwnCockpit
        virtual bool updateOwnCockpit(const BlackMisc::Aviation::CComSystem &com1, const BlackMisc::Aviation::CComSystem &com2, const BlackMisc::Aviation::CTransponder &transponder, const QString &originator) override;

        //! \copydoc IContextOwnAircraft::updatePilot
        virtual bool updatePilot(const BlackMisc::Network::CUser &pilot, const QString &originator) override;

        //! \copydoc IContextOwnAircraft::updateCallsign
        virtual bool updateCallsign(const BlackMisc::Aviation::CCallsign &callsign, const QString &originator) override;

        //! \copydoc IContextOwnAircraft::updateIcaoData
        virtual bool updateIcaoData(const BlackMisc::Aviation::CAircraftIcao &icaoData, const QString &originator) override;

        //! \copydoc IContextOwnAircraft::updateSelcal
        virtual bool updateSelcal(const BlackMisc::Aviation::CSelcal &selcal, const QString &originator) override;

        //! \copydoc IContextOwnAircraft::setAudioOutputVolumes
        virtual void setAudioOutputVolumes(int outputVolumeCom1, int outputVolumeCom2) override;

        //! \copydoc IContextOwnAircraft::setAudioVoiceRoomOverrideUrls
        virtual void setAudioVoiceRoomOverrideUrls(const QString &voiceRoom1Url, const QString &voiceRoom2Url) override;

        //! \copydoc IContextOwnAircraft::enableAutomaticVoiceRoomResolution
        virtual void enableAutomaticVoiceRoomResolution(bool enable) override;

        //! \addtogroup commandline
        //! @{
        //! <pre>
        //! .x    .xpdr  code       set transponder code    CContextOwnAircraft
        //! .com1 .com2  frequency  set COM1/2 frequency    CContextOwnAircraft
        //! .selcal      code       set SELCAL code         CContextOwnAircraft
        //! </pre>
        //! @}
        //! \copydoc IContextOwnAircraft::parseCommandLine
        virtual bool parseCommandLine(const QString &commandLine) override;

    protected:
        //! Constructor, with link to runtime
        CContextOwnAircraft(CRuntimeConfig::ContextMode, CRuntime *runtime);

        //! Register myself in DBus
        CContextOwnAircraft *registerWithDBus(CDBusServer *server)
        {
            if (!server || this->m_mode != CRuntimeConfig::LocalInDbusServer) return this;
            server->addObject(IContextOwnAircraft::ObjectPath(), this);
            return this;
        }

    private slots:
        //! Station has been changed, needed to tune in/out voice room
        //! Set in runtime
        void ps_changedAtcStationOnlineConnectionStatus(const BlackMisc::Aviation::CAtcStation &atcStation, bool connected);

    private:
        BlackMisc::Aviation::CAircraft m_ownAircraft; //!< my aircraft
        bool m_automaticVoiceRoomResolution = true;   //!< automatic voice room resolution, or disable for override
        QString m_voiceRoom1UrlOverride;              //!< overridden voice room url
        QString m_voiceRoom2UrlOverride;              //!< overridden voice room url

        //! Init my very own aircraft with some defaults, before overridden by simulator
        void initOwnAircraft();

        //! Resolve voice rooms
        void resolveVoiceRooms();

    };
}

#endif // guard
