/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKCORE_VATSIM_VATSIMMETARREADER_H
#define BLACKCORE_VATSIM_VATSIMMETARREADER_H

#include "blackcore/blackcoreexport.h"
#include "blackmisc/aviation/airporticaocode.h"
#include "blackmisc/network/entityflags.h"
#include "blackcore/threadedreader.h"
#include "blackmisc/weather/metar.h"
#include "blackmisc/weather/metardecoder.h"
#include "blackmisc/weather/metarlist.h"

#include <QObject>

class QNetworkReply;

namespace BlackCore
{
    namespace Vatsim
    {
        //! Read bookings from VATSIM
        class BLACKCORE_EXPORT CVatsimMetarReader : public BlackCore::CThreadedReader
        {
            Q_OBJECT

        public:
            //! Constructor
            explicit CVatsimMetarReader(QObject *owner);

            //! Read / re-read bookings
            void readInBackgroundThread();

            //! Get METARs
            //! \threadsafe
            virtual BlackMisc::Weather::CMetarList getMetars() const;

            //! Get METAR for airport
            //! \threadsafe
            virtual BlackMisc::Weather::CMetar getMetarForAirport(const BlackMisc::Aviation::CAirportIcaoCode &icao) const;

            //! Get METARs count
            //! \threadsafe
            virtual int getMetarsCount() const;

        signals:
            //! METARs have been read and converted to BlackMisc::Weather::CMetarList
            void metarsRead(const BlackMisc::Weather::CMetarList &metars);

            //! Data have been read
            void dataRead(BlackMisc::Network::CEntityFlags::Entity entity, BlackMisc::Network::CEntityFlags::ReadState state, int number);

        protected:
            //! \name BlackCore::CThreadedReader overrides
            //! @{
            virtual void cleanup() override;
            virtual void doWorkImpl() override;
            //! @}

        private:
            //! Decode METARs
            //! \threadsafe
            void decodeMetars(QNetworkReply *nwReply);

            //! Do reading
            void readMetars();

            void reloadSettings();

        private:
            BlackMisc::Weather::CMetarDecoder m_metarDecoder;
            BlackMisc::Weather::CMetarList    m_metars;
            BlackMisc::CSettingReadOnly<BlackCore::Vatsim::TVatsimMetars> m_settings { this, &CVatsimMetarReader::reloadSettings };
        };
    } // ns
} // ns
#endif // guard
