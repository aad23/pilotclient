/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKSIM_SIMULATORPLUGININFO_H
#define BLACKSIM_SIMULATORPLUGININFO_H

#include "blackmisc/propertyindexvariantmap.h"
#include "blackmisc/valueobject.h"

namespace BlackSim
{
    //! Describing a simulator plugin
    class CSimulatorPluginInfo : public BlackMisc::CValueObject<CSimulatorPluginInfo>
    {
        /**
         * The _name_ property identifies the plugin itself and must be uniqe.
         */
        Q_PROPERTY(QString name READ name)
        
        /**
         * The _simulator_ property specifies which simulator the plugin handles.
         * There cannot be two plugins loaded for the same simulator.
         * Swift enables some features for particular simulators. Currently recognized are:
         *      fsx, fs9, xplane
         */
        Q_PROPERTY(QString simulator READ simulator)
        
        /**
         * The _description_ property provides a short, human-readable description of the plugin.
         */
        Q_PROPERTY(QString description READ description)
        
    public:
        //! Default constructor
        CSimulatorPluginInfo() = default;
        
        //! This constructor takes a JSON object that comes with the plugin metadata
        CSimulatorPluginInfo(const QJsonObject& json);

        //! Unspecified simulator
        bool isUnspecified() const;

        //! Single setting value
        BlackMisc::CVariant getSimulatorSetupValue(int index) const;

        //! Single setting value
        QString getSimulatorSetupValueAsString(int index) const;

        //! Set single settings
        void setSimulatorSetup(const BlackMisc::CPropertyIndexVariantMap &setup);

        //! Check if the provided plugin metadata is valid.
        //! Simulator plugin (driver) has to meet the following requirements:
        //!  * implements org.swift.pilotclient.BlackCore.SimulatorInterface;
        //!  * provides plugin name;
        //!  * specifies simulator it handles.
        //! Unspecified sim is considered as invalid.
        inline bool isValid() const { return m_valid; }
        
        inline bool operator==(const CSimulatorPluginInfo& other) { return name() == other.name(); }
        
        inline const QString& name() const { return m_name; }
        inline const QString& simulator() const { return m_simulator; }
        inline const QString& description() const { return m_description; }

    protected:
        //! \copydoc CValueObject::convertToQString
        virtual QString convertToQString(bool i18n = false) const override;

    private:
        BLACK_ENABLE_TUPLE_CONVERSION(CSimulatorPluginInfo)
        QString m_name;
        QString m_simulator;
        QString m_description;
        bool m_valid { false };
        BlackMisc::CPropertyIndexVariantMap m_simsetup; //!< allows to access simulator keys requried on remote side
    };
}

BLACK_DECLARE_TUPLE_CONVERSION(BlackSim::CSimulatorPluginInfo, (
                                   o.m_name,
                                   attr(o.m_simsetup, flags<DisabledForComparison>())
                               ))
Q_DECLARE_METATYPE(BlackSim::CSimulatorPluginInfo)

#endif // guard
