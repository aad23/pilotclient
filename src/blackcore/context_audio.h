/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKCORE_CONTEXTAUDIO_H
#define BLACKCORE_CONTEXTAUDIO_H

#include "blackcore/context.h"
#include "blackmisc/genericdbusinterface.h"
#include "blackmisc/audiodeviceinfolist.h"
#include "blackmisc/voiceroomlist.h"
#include "blackmisc/nwuserlist.h"
#include "blackmisc/avaircraft.h"
#include "blackmisc/avcallsignlist.h"
#include "blackmisc/avselcal.h"
#include <QObject>

//! \addtogroup dbus
//! @{

//! DBus interface for context
#define BLACKCORE_CONTEXTAUDIO_INTERFACENAME "net.vatsim.PilotClient.BlackCore.ContextAudio"

//! DBus object path for context
#define BLACKCORE_CONTEXTAUDIO_OBJECTPATH "/Audio"

//! @}

namespace BlackCore
{

    //! \brief Audio context interface
    class IContextAudio : public CContext
    {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", BLACKCORE_CONTEXTAUDIO_INTERFACENAME)

    protected:
        //! \brief Constructor
        IContextAudio(CRuntimeConfig::ContextMode mode, CRuntime *runtime) : CContext(mode, runtime) {}

    public:

        //! ComUnit number
        enum ComUnit
        {
            Com1,
            Com2
        };

        //! \brief Interface name
        static const QString &InterfaceName()
        {
            static QString s(BLACKCORE_CONTEXTAUDIO_INTERFACENAME);
            return s;
        }

        //! \brief Object path
        static const QString &ObjectPath()
        {
            static QString s(BLACKCORE_CONTEXTAUDIO_OBJECTPATH);
            return s;
        }

        //! \copydoc CContext::getPathAndContextId()
        virtual QString getPathAndContextId() const { return this->buildPathAndContextId(ObjectPath()); }

        //! Factory method
        static IContextAudio *create(CRuntime *parent, CRuntimeConfig::ContextMode mode, CDBusServer *server, QDBusConnection &conn);

        //! \brief Destructor
        virtual ~IContextAudio() {}

    signals:
        //! \brief Audio test has been completed
        void audioTestCompleted();

        //! Voice rooms changed
        //! \details the flag indicates, whether a room got connected or disconnected
        void changedVoiceRooms(const BlackMisc::Audio::CVoiceRoomList &voiceRooms, bool connected);

        //! Volumes changed (COM1, COM2)
        //! \sa setVolumes
        // KB: Is see some potential changes here, which we should do when we have the new 2.0 vatlib
        // 1. volume integrated in voice room?
        // 2. Value object for volumes CVolume / CVolumeList?
        void changedAudioVolumes(int com1Volume, int com2Volume);

        //! Mute changed
        void changedMute(bool muted);

        //! Changed audio devices (e.g. device enabled/disable)
        void changedAudioDevices(const BlackMisc::Audio::CAudioDeviceInfoList &devices);

        //! Changed slection of audio devices
        void changedSelectedAudioDevices(const BlackMisc::Audio::CAudioDeviceInfoList &devices);

    public slots:
        //! Get voice rooms for COM1, COM2:
        virtual BlackMisc::Audio::CVoiceRoomList getComVoiceRoomsWithAudioStatus() const = 0;

        //! Get voice rooms for COM1, COM2, but without latest audio status
        virtual BlackMisc::Audio::CVoiceRoomList getComVoiceRooms() const = 0;

        //! Get voice room per com unit
        virtual BlackMisc::Audio::CVoiceRoom getVoiceRoom(int comUnit, bool withAudioStatus) const = 0;

        //! Set voice rooms
        virtual void setComVoiceRooms(const BlackMisc::Audio::CVoiceRoomList &voiceRooms) = 0;

        //! Leave all voice rooms
        virtual void leaveAllVoiceRooms() = 0;

        //! Room user callsigns
        virtual BlackMisc::Aviation::CCallsignList getRoomCallsigns(int comUnit) const = 0;

        //! Room users
        virtual BlackMisc::Network::CUserList getRoomUsers(int comUnit) const = 0;

        //! Audio devices
        virtual BlackMisc::Audio::CAudioDeviceInfoList getAudioDevices() const = 0;

        /*!
         * \brief Get current audio device
         * \return input and output devices
         */
        virtual BlackMisc::Audio::CAudioDeviceInfoList getCurrentAudioDevices() const = 0;

        /*!
         * \brief Set current audio device
         * \param audioDevice can be input or audio device
         */
        virtual void setCurrentAudioDevice(const BlackMisc::Audio::CAudioDeviceInfo &audioDevice) = 0;

        /*!
         * \brief Set volumes via com units, also allows to mute
         * \see BlackMisc::Aviation::CComSystem::setVolumeInput()
         * \see BlackMisc::Aviation::CComSystem::setVolumeOutput()
         */
        virtual void setVolumes(const BlackMisc::Aviation::CComSystem &com1, const BlackMisc::Aviation::CComSystem &com2) = 0;

        //! Set the volumes (0..100)
        virtual void setVolumes(int volumeCom1, int volumeCom2) = 0;

        //! Set voice output volume (0..300)
        virtual void setVoiceOutputVolume(int volume) = 0;

        //! Set mute state
        virtual void setMute(bool mute) = 0;

        //! Is muted?
        virtual bool isMuted() const = 0;

        //! Play SELCAL tone
        virtual void playSelcalTone(const BlackMisc::Aviation::CSelcal &selcal) const = 0;

        /*!
         * \brief Play notification sound
         * \param notification CSoundGenerator::Notification
         * \param considerSettings consider settings (notification on/off), false means settings ignored
         */
        virtual void playNotification(uint notification, bool considerSettings) const = 0;

        //! Microphone test
        virtual void runMicrophoneTest() = 0;

        //! Microphone test
        virtual void runSquelchTest() = 0;

        //! Get the microphone test result
        virtual QString getMicrophoneTestResult() const = 0;

        //! Get the squelch value
        virtual double getSquelchValue() const = 0;

        //! Enable audio loopback
        virtual void enableAudioLoopback(bool enable = true) = 0;

        //! Command line was entered
        virtual bool parseCommandLine(const QString &commandLine) = 0;
    };
}

#endif // guard
