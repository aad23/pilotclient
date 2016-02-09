/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file
//! \ingroup sampleblackmiscsim

#ifndef BLACKSAMPLE_SAMPLESVPILOTRULES_H
#define BLACKSAMPLE_SAMPLESVPILOTRULES_H

#include <QTextStream>

namespace BlackSample
{
    //! Samples for vPilot rules
    class CSamplesVPilotRules
    {
    public:
        //! Run the samples
        static void samples(QTextStream &streamOut, QTextStream &streamIn);
    };
} // namespace

#endif
