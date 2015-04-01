/*  Copyright (C) 2013 VATSIM Community / contributors
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BLACKMISCTEST_SAMPLESGEO_H
#define BLACKMISCTEST_SAMPLESGEO_H

namespace BlackMiscTest
{

/*!
 * \brief Samples for vector / matrix
 */
class CSamplesGeo
{
public:
    /*!
     * \brief Run the samples
     */
    static int samples();

private:
    /*!
     * \brief Avoid init
     */
    CSamplesGeo();
};
} // namespace


#endif // guard
