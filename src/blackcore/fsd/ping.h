/* Copyright (C) 2019
 * swift project community / contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

//! \file

#ifndef BLACKCORE_FSD_PING_H
#define BLACKCORE_FSD_PING_H

#include "messagebase.h"

namespace BlackCore
{
    namespace Fsd
    {
        class BLACKCORE_EXPORT Ping : public MessageBase
        {
        public:
            Ping(const QString &sender, const QString &receiver, const QString &timestamp);
            virtual ~Ping() {}

            QStringList toTokens() const;
            static Ping fromTokens(const QStringList &tokens);
            static QString pdu() { return "$PI"; }

            QString m_timestamp;

        private:
            Ping();
        };

        inline bool operator==(const Ping &lhs, const Ping &rhs)
        {
            return  lhs.sender() == rhs.sender() &&
                    lhs.receiver() == rhs.receiver() &&
                    lhs.m_timestamp == rhs.m_timestamp;
        }

        inline bool operator!=(const Ping &lhs, const Ping &rhs)
        {
            return !(lhs == rhs);
        }
    }
}

#endif // guard