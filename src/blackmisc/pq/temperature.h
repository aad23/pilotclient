/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_PQ_TEMPERATURE_H
#define BLACKMISC_PQ_TEMPERATURE_H

#include "blackmisc/blackmiscexport.h"
#include "blackmisc/pq/physicalquantity.h"

namespace BlackMisc
{

    //! \private
    template <> struct CValueObjectPolicy<PhysicalQuantities::CTemperature> : public CValueObjectPolicy<>
    {
        using MetaType = Policy::MetaType::DefaultAndQList;
    };

    namespace PhysicalQuantities
    {

        /*!
         * Physical unit temperature
         */
        class BLACKMISC_EXPORT CTemperature : public CValueObject<CTemperature, CPhysicalQuantity<CTemperatureUnit, CTemperature>>
        {
        public:
            //! Default constructor
            CTemperature() : CValueObject(0, CTemperatureUnit::defaultUnit()) {}

            //! Init by double value
            CTemperature(double value, const CTemperatureUnit &unit): CValueObject(value, unit) {}

            //! \copydoc CPhysicalQuantity(const QString &unitString)
            CTemperature(const QString &unitString) : CValueObject(unitString) {}
        };

    }
}

Q_DECLARE_METATYPE(BlackMisc::PhysicalQuantities::CTemperature)

#endif // guard
