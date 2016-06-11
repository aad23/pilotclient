/* Copyright (C) 2014
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file
//! \ingroup sampleblackmiscsim

#include "samplesfscommon.h"
#include "blackmisc/sampleutils.h"
#include "blackmisc/simulation/fscommon/aircraftcfgentrieslist.h"
#include "blackmisc/simulation/fscommon/aircraftcfgparser.h"
#include "blackmisc/simulation/settings/settingssimulator.h"
#include "blackmisc/simulation/simulatorinfo.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QString>
#include <QTemporaryFile>
#include <QTextStream>
#include <QTime>
#include <QtGlobal>

using namespace BlackMisc;
using namespace BlackMisc::Simulation;
using namespace BlackMisc::Simulation::FsCommon;
using namespace BlackMisc::Simulation::Settings;

namespace BlackSample
{
    void CSamplesFsCommon::samples(QTextStream &streamOut, QTextStream &streamIn)
    {
        const QString fsDir = CSampleUtils::selectDirectory(
        {
            "C:/Program Files (x86)/Microsoft Games/Microsoft Flight Simulator X/SimObjects",
            "C:/Flight Simulator 9/Aircraft"
        }, streamOut, streamIn);

        const CSimulatorInfo sim = fsDir.toLower().contains("simobjects") ? CSimulatorInfo::FSX : CSimulatorInfo::FS9;
        CMultiSimulatorSimulatorSettings multiSettings;
        const CSettingsSimulator originalSettings = multiSettings.getSettings(sim);
        CSettingsSimulator newSettings(originalSettings);
        newSettings.setModelDirectory(fsDir);
        multiSettings.setSettings(newSettings, sim); // set, but do NOT(!) save

        CAircraftCfgParser parser(sim);
        streamOut << "start reading, press RETURN" << endl;
        QString input = streamIn.readLine();
        Q_UNUSED(input);

        streamOut << "reading directly" << endl;
        QTime time;
        time.start();
        streamOut << "reading " << parser.getModelDirectory() << endl;
        parser.startLoading();
        streamOut << "read entries: " << parser.getAircraftCfgEntriesList().size() << " in " << time.restart() << "ms" << endl;

        CAircraftCfgEntriesList entriesList = parser.getAircraftCfgEntriesList();
        QJsonDocument doc(entriesList.toJson());
        QByteArray jsonArray(doc.toJson());
        streamOut << "write JSON array with size " << jsonArray.size() << endl;
        QTemporaryFile tempFile;
        tempFile.open();
        tempFile.write(jsonArray);
        tempFile.close();
        streamOut << "written to " << tempFile.fileName() << " in " << time.restart() << "ms" <<  endl;

        // re-read
        tempFile.open();
        jsonArray = tempFile.readAll();
        doc = QJsonDocument::fromJson(jsonArray);
        entriesList.clear();
        entriesList.convertFromJson(doc.object());
        streamOut << "read JSON array with size " << jsonArray.size() << endl;
        streamOut << "read entries from disk: " << entriesList.size() << " in " << time.restart() << "ms" << endl;
        tempFile.close();

        // restore settings: DO NOT SAVE !!!
        multiSettings.setSettings(originalSettings, sim);
    }
} // namespace
