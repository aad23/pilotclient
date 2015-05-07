/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#ifndef BLACKMISC_CPROJECT_H
#define BLACKMISC_CPROJECT_H

#include "blackmiscexport.h"
#include <QString>

namespace BlackMisc
{

    /*!
     * Metadata about the project
     */
    class BLACKMISC_EXPORT CProject
    {
    public:
        //! with BlackCore?
        static bool isCompiledWithBlackCore();

        //! with BlackSound?
        static bool isCompiledWithBlackSound();

        //! with BlackInput?
        static bool isCompiledWithBlackInput();

        //! with FS9 support?
        static bool isCompiledWithFs9Support();

        //! with FSX support?
        static bool isCompiledWithFsxSupport();

        //! with XPlane support?
        static bool isCompiledWithXPlaneSupport();

        //! with any simulator libraries
        static bool isCompiledWithFlightSimulatorSupport();

        //! with GUI?
        static bool isCompiledWithGui();

        //! Info string about compilation
        static const QString &compiledInfo();

        //! Simulator String info
        static const QString &simulators();

        //! Simulator String info
        static const char *simulatorsChar();

        //! Version info
        static const QString &version();

        //! System's name and version
        static const QString &systemNameAndVersion();

        //! System's name and version
        static const char *systemNameAndVersionChar();

        //! Version major
        static int versionMajor();

        //! Version minor
        static int versionMinor();

        //! Debug build?
        static bool isDebugBuild();

        //! Release build?
        static bool isReleaseBuild();

        //! Running on Windows NT platform?
        static bool isRunningOnWindowsNtPlatform();

    private:
        //! Constructor
        CProject() {}

        //! Split version
        static int getMajorMinor(int index);
    };
}

#endif // guard
