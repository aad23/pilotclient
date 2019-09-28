/* Copyright (C) 2019
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "cryptodtochannel.h"

namespace BlackCore
{
    namespace Afv
    {
        namespace Crypto
        {
            CCryptoDtoChannel::CCryptoDtoChannel(QString channelTag, const QByteArray &aeadReceiveKey, const QByteArray &aeadTransmitKey, int receiveSequenceHistorySize)
            {
                m_channelTag      = channelTag;
                m_aeadReceiveKey  = aeadReceiveKey;
                m_aeadTransmitKey = aeadTransmitKey;

                m_receiveSequenceSizeMaxSize = receiveSequenceHistorySize;
                if (m_receiveSequenceSizeMaxSize < 1) { m_receiveSequenceSizeMaxSize = 1; }
                m_receiveSequenceHistory.fill(0, m_receiveSequenceSizeMaxSize);
                m_receiveSequenceHistoryDepth = 0;
            }

            CCryptoDtoChannel::CCryptoDtoChannel(CryptoDtoChannelConfigDto channelConfig, int receiveSequenceHistorySize)
            {
                m_channelTag      = channelConfig.channelTag;
                m_aeadReceiveKey  = channelConfig.aeadReceiveKey;
                m_aeadTransmitKey = channelConfig.aeadTransmitKey;
                m_hmacKey         = channelConfig.hmacKey;

                m_receiveSequenceSizeMaxSize = receiveSequenceHistorySize;
                if (m_receiveSequenceSizeMaxSize < 1) { m_receiveSequenceSizeMaxSize = 1; }
                m_receiveSequenceHistory.fill(0, m_receiveSequenceSizeMaxSize);
                m_receiveSequenceHistoryDepth = 0;
            }

            QByteArray CCryptoDtoChannel::getTransmitKey(CryptoDtoMode mode)
            {
                switch (mode)
                {
                case CryptoDtoMode::AEAD_ChaCha20Poly1305: return m_aeadTransmitKey;
                case CryptoDtoMode::Undefined:
                case CryptoDtoMode::None:
                    qFatal("GetTransmitKey called with wrong argument.");
                }

                return {};
            }

            QByteArray CCryptoDtoChannel::getTransmitKey(CryptoDtoMode mode, uint &sequenceToSend)
            {
                sequenceToSend = m_transmitSequence;
                m_transmitSequence++;
                m_LastTransmitUtc = QDateTime::currentDateTimeUtc();

                switch (mode)
                {
                case CryptoDtoMode::AEAD_ChaCha20Poly1305: return m_aeadTransmitKey;
                case CryptoDtoMode::Undefined:
                case CryptoDtoMode::None:
                    qFatal("GetTransmitKey called with wrong argument.");
                }

                return {};
            }

            QString CCryptoDtoChannel::getChannelTag() const
            {
                return m_channelTag;
            }

            QByteArray CCryptoDtoChannel::getReceiveKey(CryptoDtoMode mode)
            {
                switch (mode)
                {
                case CryptoDtoMode::AEAD_ChaCha20Poly1305: return m_aeadReceiveKey;
                case CryptoDtoMode::Undefined:
                case CryptoDtoMode::None:
                    qFatal("getReceiveKey called with wrong argument.");
                }

                return {};
            }

            bool CCryptoDtoChannel::checkReceivedSequence(uint sequenceReceived)
            {
                if (contains(sequenceReceived))
                {
                    // Duplication or replay attack
                    return false;
                }

                if (m_receiveSequenceHistoryDepth < m_receiveSequenceSizeMaxSize)                       //If the buffer has been filled...
                {
                    m_receiveSequenceHistory[m_receiveSequenceHistoryDepth++] = sequenceReceived;
                }
                else
                {
                    int minIndex;
                    uint minValue = getMin(minIndex);
                    if (sequenceReceived < minValue) { return false; }          // Possible replay attack
                    m_receiveSequenceHistory[minIndex] = sequenceReceived;
                }

                m_lastReceiveUtc = QDateTime::currentDateTimeUtc();
                return true;
            }

            bool CCryptoDtoChannel::contains(uint sequence) const
            {
                for (int i = 0; i < m_receiveSequenceHistoryDepth; i++)
                {
                    if (m_receiveSequenceHistory[i] == sequence) { return true; }
                }
                return false;
            }

            uint CCryptoDtoChannel::getMin(int &minIndex) const
            {
                uint minValue = std::numeric_limits<uint>::max();
                minIndex  = -1;
                int index = -1;

                for (int i = 0; i < m_receiveSequenceHistoryDepth; i++)
                {
                    index++;
                    if (m_receiveSequenceHistory[i] <= minValue)
                    {
                        minValue = m_receiveSequenceHistory[i];
                        minIndex = index;
                    }
                }
                return minValue;
            }
        } // ns
    } // ns
} // ns