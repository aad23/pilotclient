/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_NETWORK_TEXTMESSAGELIST_H
#define BLACKMISC_NETWORK_TEXTMESSAGELIST_H

#include "blackmisc/aviation/callsign.h"
#include "blackmisc/blackmiscexport.h"
#include "blackmisc/collection.h"
#include "blackmisc/network/textmessage.h"
#include "blackmisc/pq/frequency.h"
#include "blackmisc/sequence.h"
#include "blackmisc/timestampobjectlist.h"
#include "blackmisc/variant.h"

#include <QList>
#include <QMetaType>
#include <QString>

namespace BlackMisc
{
    namespace Network
    {
        class CTextMessage;

        //! Value object encapsulating a list of text messages
        class BLACKMISC_EXPORT CTextMessageList :
            public CSequence<CTextMessage>,
            public ITimestampObjectList<CTextMessage, CTextMessageList>,
            public Mixin::MetaType<CTextMessageList>
        {
        public:
            BLACKMISC_DECLARE_USING_MIXIN_METATYPE(CTextMessageList)

            //! Default constructor.
            CTextMessageList();

            //! Constructor, single private message
            CTextMessageList(const QString &message, const Aviation::CCallsign &recipientCallsign);

            //! Constructor, single private message
            CTextMessageList(const QString &message, const Aviation::CCallsign &senderCallsign, const Aviation::CCallsign &recipientCallsign);

            //! Constructor, single radio message
            CTextMessageList(const QString &message, const PhysicalQuantities::CFrequency &frequency, const Aviation::CCallsign &senderCallsign = Aviation::CCallsign());

            //! Constructor, single message
            CTextMessageList(const CTextMessage &message);

            //! Constructor, multi-frequency radio messages
            CTextMessageList(const QString &message, const QList<PhysicalQuantities::CFrequency> &frequencies, const Aviation::CCallsign &sender = Aviation::CCallsign());

            //! Construct from a base class object.
            CTextMessageList(const CSequence<CTextMessage> &other);

            //! Private messages
            CTextMessageList getPrivateMessages() const;

            //! Contains private messages?
            bool containsPrivateMessages() const;

            //! Public messages
            CTextMessageList getRadioMessages() const;

            //! Supervisor messages
            CTextMessageList getSupervisorMessages() const;

            //! Contains radio messages?
            bool containsRadioMessages() const;

            //! Contains supervisor message
            CTextMessageList containsSupervisorMessages() const;

            //! Find by frequency
            CTextMessageList findByFrequency(const PhysicalQuantities::CFrequency &frequency) const;

            //! Toggle all sender receivers
            void toggleSenderRecipients();

            //! Mark all messages as sent
            void markAsSent();

        };
    } //namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Network::CTextMessageList)
Q_DECLARE_METATYPE(BlackMisc::CCollection<BlackMisc::Network::CTextMessage>)
Q_DECLARE_METATYPE(BlackMisc::CSequence<BlackMisc::Network::CTextMessage>)

#endif //guard
