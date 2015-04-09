/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "navigatordockwidget.h"
#include "ui_navigatordockwidget.h"

using namespace BlackGui;

namespace BlackGui
{
    namespace Components
    {
        CNavigatorDockWidget::CNavigatorDockWidget(QWidget *parent) :
            CDockWidgetInfoArea(parent),
            ui(new Ui::CNavigatorDockWidget)
        {
            this->allowStatusBar(false);
            ui->setupUi(this);
        }

        CNavigatorDockWidget::~CNavigatorDockWidget()
        { }

    } // ns
} // ns
