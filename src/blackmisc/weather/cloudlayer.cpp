/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "blackmisc/weather/cloudlayer.h"
#include "blackmisc/propertyindex.h"
#include "blackmisc/variant.h"

#include <QHash>

using namespace BlackMisc::Aviation;

namespace BlackMisc
{
    namespace Weather
    {

        CCloudLayer::CCloudLayer(const BlackMisc::Aviation::CAltitude &base,
                                 const BlackMisc::Aviation::CAltitude &top,
                                 Coverage coverage) :
            m_base(base), m_top(top)
        {
            setCoverage(coverage);
        }

        CCloudLayer::CCloudLayer(const BlackMisc::Aviation::CAltitude &base,
                                 const BlackMisc::Aviation::CAltitude &top,
                                 double precipitationRate,
                                 Precipitation precipitation,
                                 Clouds clouds,
                                 Coverage coverage) :
            m_base(base), m_top(top), m_precipitationRate(precipitationRate),
            m_precipitation(precipitation), m_clouds(clouds)
        {
            setCoverage(coverage);
        }

        void CCloudLayer::setCoverage(Coverage coverage)
        {
            m_coveragePercent = 100 * coverage / 4;
        }

        CCloudLayer::Coverage CCloudLayer::getCoverage() const
        {
            if (m_coveragePercent > 85) { return Overcast; }
            if (m_coveragePercent > 60 && m_coveragePercent <= 85) { return Broken; }
            if (m_coveragePercent > 30 && m_coveragePercent <= 60) { return Scattered; }
            if (m_coveragePercent > 10 && m_coveragePercent <= 30) { return Few; }
            return None;
        }

        CVariant CCloudLayer::propertyByIndex(const BlackMisc::CPropertyIndex &index) const
        {
            if (index.isMyself()) { return CVariant::from(*this); }
            ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexBase:
                return CVariant::fromValue(m_base);
            case IndexTop:
                return CVariant::fromValue(m_top);
            case IndexPrecipitationRate:
                return CVariant::fromValue(m_precipitationRate);
            case IndexPrecipitation:
                return CVariant::fromValue(m_precipitation);
            case IndexClouds:
                return CVariant::fromValue(m_clouds);
            case IndexCoveragePercent:
                return CVariant::fromValue(m_coveragePercent);
            default:
                return CValueObject::propertyByIndex(index);
            }
        }

        void CCloudLayer::setPropertyByIndex(const CPropertyIndex &index, const CVariant &variant)
        {
            if (index.isMyself()) { (*this) = variant.to<CCloudLayer>(); return; }
            ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexBase:
                setBase(variant.value<CAltitude>());
                break;
            case IndexTop:
                setTop(variant.value<CAltitude>());
                break;
            case IndexPrecipitationRate:
                setPrecipitationRate(variant.value<int>());
                break;
            case IndexPrecipitation:
                setPrecipitation(variant.value<Precipitation>());
                break;
            case IndexClouds:
                setClouds(variant.value<Clouds>());
                break;
            case IndexCoveragePercent:
                setCoveragePercent(variant.value<int>());
                break;
            default:
                CValueObject::setPropertyByIndex(index, variant);
                break;
            }
        }

        QString CCloudLayer::convertToQString(bool /** i18n **/) const
        {
            static const QHash<Coverage, QString> hash =
            {
                { None, "" },
                { Few, "few" },
                { Scattered, "scattered" },
                { Broken, "broken" },
                { Overcast, "overcast" }
            };

            return QStringLiteral("%1 from %2 to %3").arg(hash.value(getCoverage()), m_base.toQString(), m_top.toQString());
        }

    } // namespace
} // namespace
