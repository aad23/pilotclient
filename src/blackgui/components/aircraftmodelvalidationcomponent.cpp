/* Copyright (C) 2019
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "aircraftmodelvalidationcomponent.h"
#include "ui_aircraftmodelvalidationcomponent.h"
#include "blackgui/guiapplication.h"
#include "blackcore/context/contextsimulator.h"

using namespace BlackMisc;
using namespace BlackMisc::Simulation;
using namespace BlackCore::Context;

namespace BlackGui
{
    namespace Components
    {
        CAircraftModelValidationComponent::CAircraftModelValidationComponent(QWidget *parent) :
            COverlayMessagesFrame(parent),
            ui(new Ui::CAircraftModelValidationComponent)
        {
            ui->setupUi(this);
            ui->comp_Simulator->setMode(CSimulatorSelector::ComboBox);
            ui->comp_Simulator->setRememberSelection(false);

            const CAircraftMatcherSetup setup = m_matchingSettings.get();
            ui->cb_EnableStartupCheck->setChecked(setup.doVerificationAtStartup());
            connect(ui->cb_EnableStartupCheck,  &QCheckBox::toggled,    this, &CAircraftModelValidationComponent::onCheckAtStartupChanged);
            connect(ui->pb_TempDisableInvalid,  &QPushButton::released, this, &CAircraftModelValidationComponent::onButtonClicked);
            connect(ui->pb_TempDisableSelected, &QPushButton::released, this, &CAircraftModelValidationComponent::onButtonClicked);
            connect(ui->pb_TriggerValidation,   &QPushButton::released, this, &CAircraftModelValidationComponent::triggerValidation);
        }

        CAircraftModelValidationComponent::~CAircraftModelValidationComponent()
        { }

        void CAircraftModelValidationComponent::validatedModelSet(const CSimulatorInfo &simulator, const CAircraftModelList &valid, const CAircraftModelList &invalid, bool stopped, const CStatusMessageList &msgs)
        {
            Q_UNUSED(simulator);
            Q_UNUSED(valid);
            ui->tvp_InvalidModels->updateContainerMaybeAsync(invalid);
            ui->comp_Simulator->setValue(simulator);
            ui->comp_Messages->clear();
            ui->comp_Messages->appendStatusMessagesToList(msgs);

            const QString msg = stopped ?
                                QStringLiteral("Validation for '%1' stopped, maybe your models are not accessible").arg(simulator.toQString(true)) :
                                QStringLiteral("Validated for '%1'. Valid: %2 Invalid: %3").arg(simulator.toQString(true)).arg(valid.size()).arg(invalid.size());
            ui->lbl_Summay->setText(msg);
            if (stopped) { this->showOverlayHTMLMessage(msg, 5000); }

            const CAircraftMatcherSetup setup = m_matchingSettings.get();
            ui->cb_EnableStartupCheck->setChecked(setup.doVerificationAtStartup());
            ui->pb_TempDisableInvalid->setEnabled(!invalid.isEmpty());
            ui->pb_TempDisableSelected->setEnabled(!invalid.isEmpty());
        }

        void CAircraftModelValidationComponent::tempDisableModels(const CAircraftModelList &models)
        {
            if (models.isEmpty()) { return; }
            if (!sGui || sGui->isShuttingDown() || !sGui->supportsContexts()) { return; }
            if (!sGui->getIContextSimulator()) { return; }
            sGui->getIContextSimulator()->disableModelsForMatching(models, true);
        }

        void CAircraftModelValidationComponent::onCheckAtStartupChanged(bool checked)
        {
            CAircraftMatcherSetup setup = m_matchingSettings.get();
            if (setup.doVerificationAtStartup() == checked) { return; }
            setup.setVerificationAtStartup(checked);
            CStatusMessage msg = m_matchingSettings.setAndSave(setup);
            Q_UNUSED(msg);
        }

        void CAircraftModelValidationComponent::triggerValidation()
        {
            if (!sGui || sGui->isShuttingDown() || !sGui->supportsContexts()) { return; }
            if (!sGui->getIContextSimulator()) { return; }
            if (sGui->getIContextSimulator()->isValidationInProgress())
            {
                this->showOverlayHTMLMessage("Validation in progress", 5000);
                return;
            }

            const CSimulatorInfo sim = ui->comp_Simulator->getValue();
            if (sGui->getIContextSimulator()->triggerModelSetValidation(sim))
            {
                this->showOverlayHTMLMessage(QStringLiteral("Triggered validation for '%1'").arg(sim.toQString(true)), 5000);
            }
            else
            {
                this->showOverlayHTMLMessage(QStringLiteral("Cannot trigger validation for '%1'").arg(sim.toQString(true)), 5000);
            }
        }

        void CAircraftModelValidationComponent::onButtonClicked()
        {
            const QObject *sender = QObject::sender();
            if (sender == ui->pb_TempDisableInvalid) { this->tempDisableModels(ui->tvp_InvalidModels->container()); }
            else if (sender == ui->pb_TempDisableSelected) { this->tempDisableModels(ui->tvp_InvalidModels->selectedObjects()); }
        }
    } // ns
} // ns
