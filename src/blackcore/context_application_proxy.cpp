/* Copyright (C) 2013 VATSIM Community / authors
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "blackcore/context_application_proxy.h"
#include "blackcore/input_manager.h"
#include "blackmisc/blackmiscfreefunctions.h"
#include "blackmisc/loghandler.h"
#include <QObject>
#include <QMetaEnum>
#include <QDBusConnection>

namespace BlackCore
{

    /*
     * Constructor for DBus
     */
    CContextApplicationProxy::CContextApplicationProxy(const QString &serviceName, QDBusConnection &connection, CRuntimeConfig::ContextMode mode, CRuntime *runtime) : IContextApplication(mode, runtime), m_dBusInterface(nullptr)
    {
        this->m_dBusInterface = new BlackMisc::CGenericDBusInterface(serviceName , IContextApplication::ObjectPath(), IContextApplication::InterfaceName(), connection, this);
        this->relaySignals(serviceName, connection);

        // Enable event forwarding from GUI process to core
        CInputManager *inputManager = CInputManager::getInstance();
        connect(inputManager, &CInputManager::hotkeyFuncEvent, this, &CContextApplicationProxy::processHotkeyFuncEvent);
        inputManager->setEventForwarding(true);

        connect(this, &IContextApplication::messageLogged, this, [](const CStatusMessage &message, const Event::COriginator &origin)
        {
            if (!origin.isFromSameProcess())
            {
                CLogHandler::instance()->logRemoteMessage(message);
            }
        });
    }

    /*
     * Workaround for signals
     */
    void CContextApplicationProxy::relaySignals(const QString &serviceName, QDBusConnection &connection)
    {
        // signals originating from impl side
        bool s = connection.connect(serviceName, IContextApplication::ObjectPath(), IContextApplication::InterfaceName(),
                                    "messageLogged", this, SIGNAL(messageLogged(BlackMisc::CStatusMessage,BlackMisc::Event::COriginator)));
        Q_ASSERT(s);
        s = connection.connect(serviceName, IContextApplication::ObjectPath(), IContextApplication::InterfaceName(),
                               "componentChanged", this, SIGNAL(componentChanged(uint, uint)));
        Q_ASSERT(s);

    }

    /*
     * Log a message
     */
    void CContextApplicationProxy::logMessage(const CStatusMessage &message, const Event::COriginator &origin)
    {
        this->m_dBusInterface->callDBus(QLatin1Literal("logMessage"), message, origin);
    }

    /*
     * Ping, is DBus alive?
     */
    qint64 CContextApplicationProxy::ping(qint64 token) const
    {
        qint64 t = this->m_dBusInterface->callDBusRet<qint64>(QLatin1Literal("ping"), token);
        return t;
    }

    /*
     * Component has changed
     */
    void CContextApplicationProxy::notifyAboutComponentChange(uint component, uint action)
    {
        this->m_dBusInterface->callDBus(QLatin1Literal("notifyAboutComponentChange"), component, action);
    }

    /*
     * To file
     */
    bool CContextApplicationProxy::writeToFile(const QString &fileName, const QString &content)
    {
        if (fileName.isEmpty()) return false;
        return this->m_dBusInterface->callDBusRet<bool>(QLatin1Literal("writeToFile"), fileName, content);
    }

    /*
     * From file
     */
    QString CContextApplicationProxy::readFromFile(const QString &fileName)
    {
        if (fileName.isEmpty()) return "";
        return this->m_dBusInterface->callDBusRet<QString>(QLatin1Literal("readFromFile"), fileName);
    }

    /*
     *  Delete file
     */
    bool CContextApplicationProxy::removeFile(const QString &fileName)
    {
        if (fileName.isEmpty()) return false;
        return this->m_dBusInterface->callDBusRet<bool>(QLatin1Literal("removeFile"), fileName);
    }

    /*
     * Check file
     */
    bool CContextApplicationProxy::existsFile(const QString &fileName)
    {
        if (fileName.isEmpty()) return false;
        return this->m_dBusInterface->callDBusRet<bool>(QLatin1Literal("existsFile"), fileName);
    }

    void CContextApplicationProxy::processHotkeyFuncEvent(const BlackMisc::Event::CEventHotkeyFunction &event)
    {
        this->m_dBusInterface->callDBus(QLatin1Literal("processHotkeyFuncEvent"), event);
    }

} // namespace
