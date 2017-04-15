/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_INPUT_KEYBOARDKEY_H
#define BLACKMISC_INPUT_KEYBOARDKEY_H

#include "blackmisc/blackmiscexport.h"
#include "blackmisc/input/keycodes.h"
#include "blackmisc/metaclass.h"
#include "blackmisc/propertyindex.h"
#include "blackmisc/valueobject.h"
#include "blackmisc/variant.h"

#include <QList>
#include <QMetaType>
#include <QString>

namespace BlackMisc
{
    namespace Input
    {
        //! Value object representing a keyboard key.
        class BLACKMISC_EXPORT CKeyboardKey : public CValueObject<CKeyboardKey>
        {
        public:
            //! Properties by index
            enum ColumnIndex
            {
                IndexKey = 0,
                IndexKeyAsString,
                IndexKeyObject, // just for updates
            };

            //! Default constructor
            CKeyboardKey();

            //! Constructor
            CKeyboardKey(BlackMisc::Input::KeyCode keyCode);

            //! Get key code
            KeyCode getKey() const { return this->m_keyCode; }

            //! Get key code
            QString getKeyAsString() const;

            //! Set key code
            void setKey(KeyCode key) { this->m_keyCode = key; }

            //! Set key code
            void setKey(int key) { this->m_keyCode = static_cast<KeyCode>(key); }

            //! Set key code
            void setKey(char key) { this->m_keyCode = static_cast<KeyCode>(key); }

            //! Is unknown?
            bool isUnknown() const { return !this->hasKey(); }

            //! Modifier?
            bool isModifier() const
            {
                return allModifiers().contains(m_keyCode);
            }

            //! with key?
            bool hasKey() const
            {
                return !(this->m_keyCode == KeyCode::Key_Unknown);
            }

            //! Set key object
            void setKeyObject(CKeyboardKey key);

            //! \copydoc BlackMisc::Mixin::Index::setPropertyByIndex
            void setPropertyByIndex(const BlackMisc::CPropertyIndex &index, const CVariant &variant);

            //! \copydoc BlackMisc::Mixin::Index::propertyByIndex
            CVariant propertyByIndex(const BlackMisc::CPropertyIndex &index) const;

            //! \copydoc BlackMisc::Mixin::String::toQString
            QString convertToQString(bool i18n = false) const;

        private:
            static const QList<KeyCode> &allModifiers();

            KeyCode m_keyCode; //!< Key code

            BLACK_METACLASS(
                CKeyboardKey,
                BLACK_METAMEMBER(keyCode)
            );
        };
    }
}

Q_DECLARE_METATYPE(BlackMisc::Input::CKeyboardKey)

#endif // guard
