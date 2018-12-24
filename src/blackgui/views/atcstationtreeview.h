/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_ATCSTATIONTREEVIEW_H
#define BLACKGUI_ATCSTATIONTREEVIEW_H

#include "blackgui/blackguiexport.h"
#include "blackmisc/aviation/atcstationlist.h"
#include "blackmisc/aviation/comsystem.h"
#include "blackmisc/pq/frequency.h"

#include <QTreeView>
#include <QList>
#include <QObject>
#include <QPoint>
#include <QMap>
#include <QModelIndex>

namespace BlackGui
{
    namespace Models
    {
        class CAtcStationTreeModel;
        class CColumns;
    }

    namespace Views
    {
        //! ATC stations view
        class BLACKGUI_EXPORT CAtcStationTreeView : public QTreeView
        {
            Q_OBJECT

        public:
            //! Constructor
            explicit CAtcStationTreeView(QWidget *parent = nullptr);

            //! \copydoc Models::CAtcStationListModel::changedAtcStationConnectionStatus
            void changedAtcStationConnectionStatus(const BlackMisc::Aviation::CAtcStation &station, bool added);

            //! Update container
            void updateContainer(const BlackMisc::Aviation::CAtcStationList &stations);

            //! Clear
            void clear();

            //! Set columns
            void setColumns(const Models::CColumns &columns);

            //! Resize all columns
            void fullResizeToContents();

        signals:
            //! Request some dummy ATC stations
            void testRequestDummyAtcOnlineStations(int number);

            //! Request COM frequency
            void requestComFrequency(const BlackMisc::PhysicalQuantities::CFrequency &frequency, BlackMisc::Aviation::CComSystem::ComUnit unit);

            //! Request a text message to
            void requestTextMessageWidget(const BlackMisc::Aviation::CCallsign &callsign);

        private:
            //! Used model
            const Models::CAtcStationTreeModel *stationModel() const;

            //! Used model
            Models::CAtcStationTreeModel *stationModel();

            //! The selected object
            BlackMisc::Aviation::CAtcStation selectedObject() const;

            //! Suffix for index
            QString suffixForIndex(const QModelIndex &index);

            //! Expanded
            void onExpanded(const QModelIndex &index);

            //! Custom menu
            void customMenu(const QPoint &point);

            //! Store state
            void storeState();

            //! Restore state
            void restoreState();

            //! Tune in/invoke @{
            void tuneInAtcCom1();
            void tuneInAtcCom2();
            void requestTextMessage();
            //! @}

            QMap<QString, bool> m_expanded; //!< suffix/expanded
        };
    } // ns
} // ns

#endif // guard