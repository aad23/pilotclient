/*  Copyright (C) 2013 VATSIM Community / contributors
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BLACKMISC_AVIATION_AVIONICSBASE_H
#define BLACKMISC_AVIATION_AVIONICSBASE_H

//! \file

#include "blackmisc/blackmiscexport.h"
#include "blackmisc/valueobject.h"
#include "blackmisc/pq/constants.h"
#include <QtGlobal>

namespace BlackMisc
{

    namespace Aviation
    {

        //! Base class for avionics
        class BLACKMISC_EXPORT CAvionicsBase :
            public Mixin::HashByTuple<CAvionicsBase>,
            public Mixin::DBusByTuple<CAvionicsBase>,
            public Mixin::JsonByTuple<CAvionicsBase>,
            public Mixin::EqualsByTuple<CAvionicsBase>,
            public Mixin::LessThanByTuple<CAvionicsBase>,
            public Mixin::CompareByTuple<CAvionicsBase>,
            public Mixin::String<CAvionicsBase>
        {
        protected:
            QString m_name; //!< name of the unit

            //! \brief Constructor
            CAvionicsBase(const QString &name) : m_name(name) {}

            //! Destructor
            virtual ~CAvionicsBase() = default;

            //! \brief Set name
            void setName(const QString &name)
            {
                this->m_name = name;
            }

        public:
            //! \copydoc CValueObject::convertToQString
            QString convertToQString(bool i18n = false) const { Q_UNUSED(i18n); return ""; }

            //! \brief Name
            QString getName() const
            {
                return this->m_name;
            }

            //! \brief Are set values valid?
            virtual bool validValues() const
            {
                return true;
            }

        private:
            BLACK_ENABLE_TUPLE_CONVERSION(CAvionicsBase)
        };

    }
}

BLACK_DECLARE_TUPLE_CONVERSION(BlackMisc::Aviation::CAvionicsBase, (o.m_name))

#endif // guard
