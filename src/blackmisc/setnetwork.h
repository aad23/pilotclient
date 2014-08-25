/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_SETTINGS_NETWORK_H
#define BLACKMISC_SETTINGS_NETWORK_H

#include "nwserverlist.h"
#include "valueobject.h"
#include "statusmessagelist.h"
#include "settingutilities.h"
#include <QJsonObject>

namespace BlackMisc
{
    class CVariant;

    namespace Settings
    {
        //! Value object encapsulating information of network related settings.
        //! \remarks Not only traffic network settings, but also URLs, DBus address, ...
        class CSettingsNetwork : public BlackMisc::CValueObject
        {
        public:
            //! Default constructor.
            CSettingsNetwork();

            //! Destructor.
            virtual ~CSettingsNetwork() {}

            //! Update
            static const QString &CmdSetCurrentServer()
            {
                static const QString cmd("currenttrafficserver");
                return cmd;
            }

            //! Path
            static const QString &ValueTrafficServers()
            {
                static const QString value("trafficservers");
                return value;
            }

            //! Path
            static const QString &ValueBookingServiceUrl()
            {
                static const QString value("bookingserviceurl");
                return value;
            }

            //! Path
            static const QString &ValueDBusServerAddress()
            {
                static const QString value("dbuserveraddress");
                return value;
            }

            //! \copydoc CValueObject::toQVariant()
            virtual QVariant toQVariant() const override { return QVariant::fromValue(*this); }

            //! \copydoc CValueObject::convertFromQVariant
            virtual void convertFromQVariant(const QVariant &variant) override { BlackMisc::setFromQVariant(this, variant); }

            //! Value object, traffic network server objects
            BlackMisc::Network::CServerList getTrafficNetworkServers() const { return m_trafficNetworkServers; }

            //! Selected traffic network server
            BlackMisc::Network::CServer getCurrentTrafficNetworkServer() const { return m_trafficNetworkServerCurrent; }

            //! URL of booking service
            QString getBookingServiceUrl() const { return m_bookingServiceUrl; }

            //! Address for DBus Server
            QString getDBusServerAddress() const { return m_dbusServerAddress; }

            //! Selected traffic network server
            bool setCurrentNetworkServer(const BlackMisc::Network::CServer &currentServer);

            //! Traffic network server objects
            void addTrafficNetworkServer(const BlackMisc::Network::CServer &server) { m_trafficNetworkServers.push_back(server); }

            //! Equal operator ==
            bool operator ==(const CSettingsNetwork &other) const;

            //! Unequal operator !=
            bool operator !=(const CSettingsNetwork &other) const;

            //! \copydoc BlackCore::IContextSettings
            virtual BlackMisc::CStatusMessageList value(const QString &path, const QString &command, const CVariant &value, bool &changedFlag);

            //! \copydoc CValueObject::getValueHash
            virtual uint getValueHash() const override;

            //! \copydoc CValueObject::toJson
            virtual QJsonObject toJson() const override;

            //! \copydoc CValueObject::convertFromJson
            virtual void convertFromJson(const QJsonObject &json) override;

            //! Init with meaningful default values
            void initDefaultValues();

            //! \copydoc CValueObject::registerMetadata
            static void registerMetadata();

            //! JSON member names
            static const QStringList &jsonMembers();

        protected:
            //! \copydoc CValueObject::convertToQString
            virtual QString convertToQString(bool i18n = false) const override;

            //! \copydoc CValueObject::getMetaTypeId
            virtual int getMetaTypeId() const override;

            //! \copydoc CValueObject::isA
            virtual bool isA(int metaTypeId) const override;

            //! \copydoc CValueObject::compareImpl
            virtual int compareImpl(const CValueObject &other) const override;

            //! \copydoc CValueObject::marshallToDbus
            virtual void marshallToDbus(QDBusArgument &argument) const override;

            //! \copydoc CValueObject::unmarshallFromDbus
            virtual void unmarshallFromDbus(const QDBusArgument &argument) override;

        private:
            BLACK_ENABLE_TUPLE_CONVERSION(CSettingsNetwork)
            BlackMisc::Network::CServerList m_trafficNetworkServers;
            BlackMisc::Network::CServer m_trafficNetworkServerCurrent;
            QString m_bookingServiceUrl;
            QString m_dbusServerAddress;
        };

    } // namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Settings::CSettingsNetwork)
BLACK_DECLARE_TUPLE_CONVERSION(BlackMisc::Settings::CSettingsNetwork, (o.m_trafficNetworkServers, o.m_trafficNetworkServerCurrent, o.m_bookingServiceUrl, o.m_dbusServerAddress))


#endif // guard
