/*  Copyright (C) 2013 VATSIM Community / contributors
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BLACKMISC_PQMASS_H
#define BLACKMISC_PQMASS_H
#include "blackmisc/pqphysicalquantity.h"

namespace BlackMisc
{
    namespace PhysicalQuantities
    {

        /*!
         * \brief Mass
         */
        class CMass : public CPhysicalQuantity<CMassUnit, CMass>
        {
        public:
            //! Default constructor
            CMass() : CPhysicalQuantity(0, CMassUnit::defaultUnit()) {}

            //! Init by double value
            CMass(double value, const CMassUnit &unit) : CPhysicalQuantity(value, unit) {}

            //! \copydoc CPhysicalQuantity(const QString &unitString)
            CMass(const QString &unitString) : CPhysicalQuantity(unitString) {}

            //! \copydoc CValueObject::toQVariant
            virtual QVariant toQVariant() const override
            {
                return QVariant::fromValue(*this);
            }

            //! Virtual destructor
            virtual ~CMass() {}
        };

    } // namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::PhysicalQuantities::CMass)

#endif // guard
