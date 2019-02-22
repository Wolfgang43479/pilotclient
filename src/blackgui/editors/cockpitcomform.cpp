/* Copyright (C) 2017
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "cockpitcomform.h"
#include "ui_cockpitcomform.h"
#include "blackmisc/math/mathutils.h"
#include "blackmisc/logmessage.h"
#include "blackmisc/icons.h"

#include <QPushButton>

using namespace BlackMisc;
using namespace BlackMisc::Aviation;
using namespace BlackMisc::Audio;
using namespace BlackMisc::PhysicalQuantities;
using namespace BlackMisc::Math;
using namespace BlackMisc::Simulation;
using namespace BlackGui::Components;

namespace BlackGui
{
    namespace Editors
    {
        CCockpitComForm::CCockpitComForm(QWidget *parent) :
            CForm(parent),
            ui(new Ui::CCockpitComForm)
        {
            ui->setupUi(this);

            // SELCAL pairs in cockpit
            ui->frp_ComPanelSelcalSelector->clear();
            connect(ui->tb_ComPanelSelcalTest, &QPushButton::clicked, this, &CCockpitComForm::testSelcal);
            connect(ui->frp_ComPanelSelcalSelector, &CSelcalCodeSelector::valueChanged, this, &CCockpitComForm::onSelcalChanged);

            // XPdr
            connect(ui->cbp_ComPanelTransponderMode, &CTransponderModeSelector::transponderModeChanged, this, &CCockpitComForm::transponderModeChanged);
            connect(ui->cbp_ComPanelTransponderMode, &CTransponderModeSelector::transponderStateIdentEnded, this, &CCockpitComForm::transponderStateIdentEnded);

            // COM GUI events
            connect(ui->tb_ComPanelCom1Toggle, &QPushButton::clicked, this, &CCockpitComForm::onGuiChangedCockpitValues);
            connect(ui->tb_ComPanelCom2Toggle, &QPushButton::clicked, this, &CCockpitComForm::onGuiChangedCockpitValues);
            connect(ui->ds_ComPanelCom1Active, &QDoubleSpinBox::editingFinished, this, &CCockpitComForm::onGuiChangedCockpitValues);
            connect(ui->ds_ComPanelCom2Active, &QDoubleSpinBox::editingFinished, this, &CCockpitComForm::onGuiChangedCockpitValues);
            connect(ui->ds_ComPanelCom1Standby, &QDoubleSpinBox::editingFinished, this, &CCockpitComForm::onGuiChangedCockpitValues);
            connect(ui->ds_ComPanelCom2Standby, &QDoubleSpinBox::editingFinished, this, &CCockpitComForm::onGuiChangedCockpitValues);
            connect(ui->sbp_ComPanelTransponder, &QDoubleSpinBox::editingFinished, this, &CCockpitComForm::onGuiChangedCockpitValues);
            connect(ui->cbp_ComPanelTransponderMode, &CTransponderModeSelector::transponderModeChanged, this, &CCockpitComForm::onGuiChangedCockpitValues);
            connect(ui->frp_ComPanelSelcalSelector, &CSelcalCodeSelector::valueChanged, this, &CCockpitComForm::onGuiChangedCockpitValues);
            connect(ui->tb_RequestTextMessageCom1, &QToolButton::released, this, &CCockpitComForm::requestCom1TextMessage);
            connect(ui->tb_RequestTextMessageCom2, &QToolButton::released, this, &CCockpitComForm::requestCom2TextMessage);

            ui->led_ComPanelCom1->setShape(CLedWidget::Rounded);
            ui->led_ComPanelCom2->setShape(CLedWidget::Rounded);
            ui->tb_RequestTextMessageCom1->setIcon(CIcons::appTextMessages16());
            ui->tb_RequestTextMessageCom2->setIcon(CIcons::appTextMessages16());
        }

        CCockpitComForm::~CCockpitComForm()
        { }

        void CCockpitComForm::setReadOnly(bool readonly)
        {
            ui->ds_ComPanelCom1Active->setReadOnly(readonly);
            ui->ds_ComPanelCom2Active->setReadOnly(readonly);
            ui->ds_ComPanelCom1Standby->setReadOnly(readonly);
            ui->ds_ComPanelCom2Standby->setReadOnly(readonly);
            ui->cbp_ComPanelTransponderMode->setDisabled(!readonly);
            ui->tb_ComPanelCom1Toggle->setEnabled(!readonly);
            ui->tb_ComPanelCom2Toggle->setEnabled(!readonly);
            this->forceStyleSheetUpdate();
        }

        void CCockpitComForm::setSelectOnly()
        {
            this->setReadOnly(true);
        }

        CStatusMessageList CCockpitComForm::validate(bool nested) const
        {
            Q_UNUSED(nested);
            CStatusMessageList msgs;
            return msgs;
        }

        void CCockpitComForm::setVoiceRoomStatus(const Audio::CVoiceRoomList &selectedVoiceRooms)
        {
            Q_ASSERT_X(selectedVoiceRooms.size() == 2, Q_FUNC_INFO, "Expect 2 voice rooms");
            const CVoiceRoom room1 = selectedVoiceRooms[0];
            const CVoiceRoom room2 = selectedVoiceRooms[1];
            ui->led_ComPanelCom1->setOn(room1.isConnected());
            ui->led_ComPanelCom2->setOn(room2.isConnected());
        }

        void CCockpitComForm::setSelectedAtcStations(const CAtcStationList &selectedStations)
        {
            const CAtcStation com1Station = selectedStations.size() > 0 ? selectedStations[0] : CAtcStation();
            const CAtcStation com2Station = selectedStations.size() > 1 ? selectedStations[1] : CAtcStation();
            if (com1Station.getCallsign().isEmpty())
            {
                ui->lbl_ComPanelCom1Active->setToolTip("");
                ui->led_ComPanelCom1->setOn(false);
            }
            else
            {
                ui->lbl_ComPanelCom1Active->setToolTip(com1Station.getCallsign().getStringAsSet());
                ui->led_ComPanelCom1->setOn(true);

            }
            if (com2Station.getCallsign().isEmpty())
            {
                ui->lbl_ComPanelCom2Active->setToolTip("");
                ui->led_ComPanelCom2->setOn(false);
            }
            else
            {
                ui->lbl_ComPanelCom2Active->setToolTip(com2Station.getCallsign().getStringAsSet());
                ui->led_ComPanelCom2->setOn(true);
            }
        }

        void CCockpitComForm::setTransponderModeStateIdent()
        {
            ui->cbp_ComPanelTransponderMode->setSelectedTransponderModeStateIdent();
        }

        void CCockpitComForm::setValue(const CSimulatedAircraft &aircraft)
        {
            this->setFrequencies(aircraft.getCom1System(), aircraft.getCom2System());
            this->setSelcal(aircraft.getSelcal());
            this->setTransponder(aircraft.getTransponder());
        }

        CSelcal CCockpitComForm::getSelcal() const
        {
            return ui->frp_ComPanelSelcalSelector->getSelcal();
        }

        void CCockpitComForm::setSelcal(const CSelcal &selcal)
        {
            ui->frp_ComPanelSelcalSelector->setSelcal(selcal);
        }

        void CCockpitComForm::initLeds()
        {
            const CLedWidget::LedShape shape = CLedWidget::Rounded;
            ui->led_ComPanelCom1->setValues(CLedWidget::Yellow, CLedWidget::Black, shape, "COM1 connected", "COM1 disconnected", 14);
            ui->led_ComPanelCom2->setValues(CLedWidget::Yellow, CLedWidget::Black, shape, "COM2 connected", "COM2 disconnected", 14);
        }

        CSimulatedAircraft CCockpitComForm::cockpitValuesToAircraftObject()
        {
            CSimulatedAircraft comAircraft;
            CTransponder transponder = comAircraft.getTransponder();
            CComSystem com1 = comAircraft.getCom1System();
            CComSystem com2 = comAircraft.getCom2System();

            //
            // Transponder
            //
            const QString transponderCode = QString::number(ui->sbp_ComPanelTransponder->value());
            if (CTransponder::isValidTransponderCode(transponderCode))
            {
                transponder.setTransponderCode(transponderCode);
            }
            else
            {
                CLogMessage(this).validationWarning(u"Wrong transponder code, reset");
                ui->sbp_ComPanelTransponder->setValue(transponder.getTransponderCode());
            }
            transponder.setTransponderMode(ui->cbp_ComPanelTransponderMode->getSelectedTransponderMode());

            //
            // COM units
            //
            com1.setFrequencyActiveMHz(ui->ds_ComPanelCom1Active->value());
            com1.setFrequencyStandbyMHz(ui->ds_ComPanelCom1Standby->value());
            com2.setFrequencyActiveMHz(ui->ds_ComPanelCom2Active->value());
            com2.setFrequencyStandbyMHz(ui->ds_ComPanelCom2Standby->value());
            this->setFrequencies(com1, com2); // back annotation after rounding

            comAircraft.setCom1System(com1);
            comAircraft.setCom2System(com2);
            comAircraft.setTransponder(transponder);
            return comAircraft;
        }

        void CCockpitComForm::setFrequencies(const CComSystem &com1, const CComSystem &com2)
        {
            double freq = com1.getFrequencyActive().valueRounded(CFrequencyUnit::MHz(), 3);
            if (!isFrequenceEqual(freq, ui->ds_ComPanelCom1Active->value()))
            {
                ui->ds_ComPanelCom1Active->setValue(freq);
            }

            freq = com2.getFrequencyActive().valueRounded(CFrequencyUnit::MHz(), 3);
            if (!isFrequenceEqual(freq, ui->ds_ComPanelCom2Active->value()))
            {
                ui->ds_ComPanelCom2Active->setValue(freq);
            }

            freq = com1.getFrequencyStandby().valueRounded(CFrequencyUnit::MHz(), 3);
            if (!isFrequenceEqual(freq, ui->ds_ComPanelCom1Standby->value()))
            {
                ui->ds_ComPanelCom1Standby->setValue(freq);
            }

            freq = com2.getFrequencyStandby().valueRounded(CFrequencyUnit::MHz(), 3);
            if (!isFrequenceEqual(freq, ui->ds_ComPanelCom2Standby->value()))
            {
                ui->ds_ComPanelCom2Standby->setValue(freq);
            }
        }

        void CCockpitComForm::setTransponder(const CTransponder &transponder)
        {
            const int tc = transponder.getTransponderCode();
            if (tc != ui->sbp_ComPanelTransponder->value())
            {
                ui->sbp_ComPanelTransponder->setValue(tc);
            }

            if (transponder.getTransponderMode() != ui->cbp_ComPanelTransponderMode->getSelectedTransponderMode())
            {
                ui->cbp_ComPanelTransponderMode->setSelectedTransponderMode(transponder.getTransponderMode());
                ui->comp_TransponderLeds->setMode(transponder.getTransponderMode());
            }
        }

        void CCockpitComForm::onGuiChangedCockpitValues()
        {
            const QObject *sender = QObject::sender();
            if (sender == ui->tb_ComPanelCom1Toggle)
            {
                if (isFrequenceEqual(ui->ds_ComPanelCom1Standby->value(), ui->ds_ComPanelCom1Active->value())) { return; }
                const double f = ui->ds_ComPanelCom1Active->value();
                ui->ds_ComPanelCom1Active->setValue(ui->ds_ComPanelCom1Standby->value());
                ui->ds_ComPanelCom1Standby->setValue(f);
            }
            else if (sender == ui->tb_ComPanelCom2Toggle)
            {
                if (isFrequenceEqual(ui->ds_ComPanelCom2Standby->value(), ui->ds_ComPanelCom2Active->value())) { return; }
                const double f = ui->ds_ComPanelCom2Active->value();
                ui->ds_ComPanelCom2Active->setValue(ui->ds_ComPanelCom2Standby->value());
                ui->ds_ComPanelCom2Standby->setValue(f);
            }

            const CSimulatedAircraft aircraft = this->cockpitValuesToAircraftObject();
            emit this->changedCockpitValues(aircraft);
        }

        void CCockpitComForm::onSelcalChanged()
        {
            const CSelcal selcal = ui->frp_ComPanelSelcalSelector->getSelcal();
            emit this->changedSelcal(selcal);
        }

        bool CCockpitComForm::isFrequenceEqual(double f1, double f2)
        {
            return CMathUtils::epsilonEqual(f1, f2);
        }
    } // ns
} // ns
