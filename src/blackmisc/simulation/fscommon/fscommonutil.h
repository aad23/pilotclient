/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_SIMULATION_FSCOMMONUTIL_H
#define BLACKMISC_SIMULATION_FSCOMMONUTIL_H

#include "blackmisc/simulation/aircraftmodellist.h"
#include "blackmisc/logcategorylist.h"
#include "blackmisc/blackmiscexport.h"

#include <atomic>
#include <QSet>
#include <QStringList>

namespace BlackMisc
{
    namespace Simulation
    {
        namespace FsCommon
        {
            //! FS9/FSX/P3D utils
            class BLACKMISC_EXPORT CFsCommonUtil
            {
            public:
                //! Log categories
                static const BlackMisc::CLogCategoryList &getLogCategories();

                //! Constructor
                CFsCommonUtil() = delete;

                //! FSX directory obtained from registry
                static const QString &fsxDirFromRegistry();

                //! FSX directory from different sources
                static const QString &fsxDir();

                //! FSX's simObject directory from registry
                static const QString &fsxSimObjectsDirFromRegistry();

                //! FSX's simobject dir, resolved from multiple sources
                static const QString &fsxSimObjectsDir();

                //! FSX aircraft dir, relative to simulator directory
                static QString fsxSimObjectsDirFromSimDir(const QString &simDir);

                //! Exclude directories for simObjects
                static const QStringList &fsxSimObjectsExcludeDirectoryPatterns();

                //! FSX's simObject dir and the add on dirs
                static QStringList fsxSimObjectsDirPlusAddOnXmlSimObjectsPaths(const QString &simObjectsDir = "");

                //! P3D's simObject dir and the add on dirs
                static QStringList p3dSimObjectsDirPlusAddOnXmlSimObjectsPaths(const QString &simObjectsDir, const QString &versionHint);

                //! Guess the P3D version such as v4, v5
                static QString guessP3DVersion(const QString &candidate);

                //! P3D directory obtained from registry
                static const QString &p3dDirFromRegistry();

                //! P3D directory from different sources
                static const QString &p3dDir();

                //! P3D's simObject directory from registry
                static const QString &p3dSimObjectsDirFromRegistry();

                //! P3D's simObject dir, resolved from multiple sources
                static const QString &p3dSimObjectsDir();

                //! P3D aircraft dir, relative to simulator directory
                static QString p3dSimObjectsDirFromSimDir(const QString &simDir);

                //! Exclude directories for simObjects
                static const QStringList &p3dSimObjectsExcludeDirectoryPatterns();

                //! FS9 directory obtained from registry
                static const QString &fs9DirFromRegistry();

                //! FS9 directory obtained from multiple sources
                static const QString &fs9Dir();

                //! FS9's aircraft directory from registry
                static const QString &fs9AircraftDirFromRegistry();

                //! FS9's aircraft directory
                static const QString &fs9AircraftDir();

                //! FS9 aircraft dir, relative to simulator directory
                static QString fs9AircraftDirFromSimDir(const QString &simDir);

                //! Exclude directories for aircraft objects
                static const QStringList &fs9AircraftObjectsExcludeDirectoryPatterns();

                //! Adjust file directory
                static bool adjustFileDirectory(CAircraftModel &model, const QString &simObjectsDirectory);

                //! Adjust file directory
                static bool adjustFileDirectory(CAircraftModel &model, const QStringList &simObjectsDirectories);

                //! Copy the terrain probe
                static int copyFsxTerrainProbeFiles(const QString &simObjectDir, CStatusMessageList &messages);

                //! Find the config files (add-ons.cfg)
                //! \note "C:/Users/Joe Doe/AppData/Roaming/Lockheed Martin/Prepar3D v4"
                //! \param versionHint like "v5"
                static QSet<QString> findP3dAddOnConfigFiles(const QString &versionHint = "v5");

                //! Find the config files (simobjects.cfg)
                //! \note "C:/Users/Joe Doe/AppData/Roaming/Lockheed Martin/Prepar3D v4"
                //! \param versionHint like "v5"
                static QSet<QString> findP3dSimObjectsConfigFiles(const QString &versionHint = "v5");

                //! All PATH values from the config files
                static QSet<QString> allConfigFilesPathValues(const QStringList &configFiles, bool checked, const QString &pathPrefix);

                //! All add-on paths from the XML add-on files "add-on.xml"
                static QSet<QString> allP3dAddOnXmlSimObjectPaths(const QStringList &addOnPaths, bool checked);

                //! All add-on paths from the XML add-on files "add-on.xml" files, use CFsCommonUtil::findP3dAddOnConfigFiles to find config files
                static QSet<QString> allP3dAddOnXmlSimObjectPaths(const QString &versionHint = "v4");

                //! Get all the SimObjects paths from all config files
                static QSet<QString> allFsxSimObjectPaths();

                //! Find the config files (fsx.cfg)
                // C:/Users/Joe Doe/AppData/Roaming/Microsoft/FSX/fsx.cfg
                static QStringList findFsxConfigFiles();

                //! Get all the SimObjects paths from fsx.cfg
                // SimObjectPaths.0=SimObjects\Airplanes
                static QSet<QString> fsxSimObjectsPaths(const QStringList &fsxFiles, bool checked);

                //! Get all the SimObjects files from fsx.cfg
                // SimObjectPaths.0=SimObjects\Airplanes
                static QSet<QString> fsxSimObjectsPaths(const QString &fsxFile, bool checked);

                //! Validate aircraft.cfg entries (sometimes also sim.cfg)
                //! \remark only for FSX/P3D/FS9 models
                static CStatusMessageList validateAircraftConfigFiles(const CAircraftModelList &models, CAircraftModelList &validModels, CAircraftModelList &invalidModels, bool ignoreEmptyFileNames, int stopAtFailedFiles, std::atomic_bool &wasStopped);

                //! Validate if known SimObjects path are used
                //! \remark only for P3D
                static CStatusMessageList validateP3DSimObjectsPath(const CAircraftModelList &models, CAircraftModelList &validModels, CAircraftModelList &invalidModels, bool ignoreEmptyFileNames, int stopAtFailedFiles, std::atomic_bool &wasStopped, const QString &simulatorDir);

                //! Validate if known SimObjects path are used
                //! \remark only for FSX
                static CStatusMessageList validateFSXSimObjectsPath(const CAircraftModelList &models, CAircraftModelList &validModels, CAircraftModelList &invalidModels, bool ignoreEmptyFileNames, int stopAtFailedFiles, std::atomic_bool &wasStopped, const QString &simulatorDir);

                //! .air file filter
                static const QString airFileFilter();

            private:
                //! Utility functions
                //! @{
                static QSet<QString> findP3dConfigFiles(const QString &configFile, const QString &versionHint = "v5");
                //! @}

                //! Validate if known SimObjects path are used
                //! \remark only for P3D/FSX
                static CStatusMessageList validateSimObjectsPath(const QSet<QString> &simObjectDirs, const CAircraftModelList &models, CAircraftModelList &validModels, CAircraftModelList &invalidModels, bool ignoreEmptyFileNames, int stopAtFailedFiles, std::atomic_bool &stopped);

                //! Log the reading of config files
                static bool logConfigPathReading();
            };
        } // namespace
    } // namespace
} // namespace

#endif // guard
