/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_PQ_ANGLE_H
#define BLACKMISC_PQ_ANGLE_H

#include "blackmisc/blackmiscexport.h"
#include "blackmisc/pq/physicalquantity.h"
#include "blackmisc/math/mathutils.h"

namespace BlackMisc
{

    //! \private
    template <> struct CValueObjectPolicy<PhysicalQuantities::CAngle> : public CValueObjectPolicy<>
    {
        using MetaType = Policy::MetaType::DefaultAndQList;
    };

    namespace PhysicalQuantities
    {
        //! Physical unit angle (radians, degrees)
        class BLACKMISC_EXPORT CAngle : public CValueObject<CAngle, CPhysicalQuantity<CAngleUnit, CAngle>>
        {
        public:
            //! Default constructor
            CAngle() : CValueObject(0, CAngleUnit::defaultUnit()) {}

            //! Init by double value
            CAngle(double value, const CAngleUnit &unit): CValueObject(value, unit) {}

            //! \copydoc CPhysicalQuantity(const QString &unitString)
            CAngle(const QString &unitString) : CValueObject(unitString) {}

            /*!
             * \brief Init as sexagesimal degrees, minutes, seconds
             * The sign of all parameters must be the same, either all positive or all negative.
             */
            CAngle(int degrees, int minutes, double seconds) :
                CValueObject(
                    degrees + minutes / 100.0 + seconds / 10000.0,
                    CAngleUnit::sexagesimalDeg()) {}

            /*!
             * \brief Init as sexagesimal degrees, minutes
             * The sign of both parameters must be the same, either both positive or both negative.
             */
            CAngle(int degrees, double minutes) :
                CValueObject(
                    degrees + minutes / 100.0,
                    CAngleUnit::sexagesimalDegMin()) {}

            //! \copydoc CValueObject::toIcon
            virtual BlackMisc::CIcon toIcon() const override;

            //! Value as factor of PI (e.g. 0.5PI)
            double piFactor() const;

            //! PI as convenience method
            static const double &PI();
        };
    }
}

Q_DECLARE_METATYPE(BlackMisc::PhysicalQuantities::CAngle)

#endif // guard
