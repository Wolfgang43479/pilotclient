/* Copyright (C) 2019
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "blackgui/components/audiodevicevolumesetupcomponent.h"
#include "blackgui/guiapplication.h"
#include "blackgui/guiutility.h"
#include "blackcore/afv/clients/afvclient.h"
#include "blackcore/context/contextaudioimpl.h"

#include "blackmisc/audio/audiodeviceinfo.h"
#include "blackmisc/audio/notificationsounds.h"
#include "blackmisc/audio/audiosettings.h"
#include "blackmisc/sequence.h"
#include "ui_audiodevicevolumesetupcomponent.h"

#include <QCheckBox>
#include <QComboBox>
#include <QToolButton>
#include <QtGlobal>
#include <QPointer>
#include <QFileDialog>
#include <QStringLiteral>

using namespace BlackCore;
using namespace BlackCore::Afv::Audio;
using namespace BlackCore::Afv::Clients;
using namespace BlackCore::Context;
using namespace BlackMisc;
using namespace BlackMisc::Aviation;
using namespace BlackMisc::Audio;
using namespace BlackMisc::PhysicalQuantities;

namespace BlackGui
{
    namespace Components
    {
        CAudioDeviceVolumeSetupComponent::CAudioDeviceVolumeSetupComponent(QWidget *parent) :
            QFrame(parent),
            ui(new Ui::CAudioDeviceVolumeSetupComponent)
        {
            ui->setupUi(this);
            connect(ui->hs_VolumeIn,         &QSlider::valueChanged, this, &CAudioDeviceVolumeSetupComponent::onVolumeSliderChanged);
            connect(ui->hs_VolumeOut,        &QSlider::valueChanged, this, &CAudioDeviceVolumeSetupComponent::onVolumeSliderChanged);
            connect(ui->tb_RefreshInDevice,  &QToolButton::released, this, &CAudioDeviceVolumeSetupComponent::onReloadDevices,  Qt::QueuedConnection);
            connect(ui->tb_RefreshOutDevice, &QToolButton::released, this, &CAudioDeviceVolumeSetupComponent::onReloadDevices,  Qt::QueuedConnection);
            connect(ui->tb_ResetInVolume,    &QToolButton::released, this, &CAudioDeviceVolumeSetupComponent::onResetVolumeIn,  Qt::QueuedConnection);
            connect(ui->tb_ResetOutVolume,   &QToolButton::released, this, &CAudioDeviceVolumeSetupComponent::onResetVolumeOut, Qt::QueuedConnection);

            ui->hs_VolumeIn->setMaximum(CSettings::InMax);
            ui->hs_VolumeIn->setMinimum(CSettings::InMin);
            ui->hs_VolumeOut->setMaximum(CSettings::OutMax);
            ui->hs_VolumeOut->setMinimum(CSettings::OutMin);

            const CSettings as(m_audioSettings.getThreadLocal());
            const int i = this->getInValue();
            const int o = this->getOutValue();
            ui->hs_VolumeIn->setValue(i);
            ui->hs_VolumeOut->setValue(o);
            ui->cb_SetupAudioLoopback->setChecked(false);
            ui->cb_DisableAudioEffects->setChecked(!as.isAudioEffectsEnabled());

            ui->led_AudioConnected->setToolTips("Voice on and authenticated", "Voice off");
            ui->led_AudioConnected->setShape(CLedWidget::Rounded);
            ui->led_Rx1->setToolTips("COM1 receiving", "COM1 idle");
            ui->led_Rx1->setShape(CLedWidget::Rounded);
            ui->led_Rx2->setToolTips("COM2 receiving", "COM2 idle");
            ui->led_Rx2->setShape(CLedWidget::Rounded);

            // deferred init, because in a distributed swift system
            // it takes a moment until the settings are sychronized
            // this is leading to undesired "save settings" messages and played sounds
            QPointer<CAudioDeviceVolumeSetupComponent> myself(this);
            QTimer::singleShot(2000, this, [ = ]
            {
                if (!myself || !sGui || sGui->isShuttingDown()) { return; }
                this->init();
            });

            // all tx/rec checkboxes
            CGuiUtility::checkBoxesReadOnly(ui->fr_TxRx, true);
        }

        void CAudioDeviceVolumeSetupComponent::init()
        {
            if (!sGui || sGui->isShuttingDown() || !sGui->getCContextAudioBase()) { return; }

            // audio is optional
            const bool audio = this->hasAudio();
            this->setEnabled(audio);
            this->reloadSettings();

            bool c = connect(ui->cb_SetupAudioLoopback, &QCheckBox::toggled, this, &CAudioDeviceVolumeSetupComponent::onLoopbackToggled);
            Q_ASSERT(c);
            c = connect(ui->cb_DisableAudioEffects, &QCheckBox::toggled, this, &CAudioDeviceVolumeSetupComponent::onDisableAudioEffectsToggled);
            Q_ASSERT(c);

            if (audio)
            {
                this->setAudioRunsWhere();
                this->initAudioDeviceLists();

                // default
                ui->cb_SetupAudioLoopback->setChecked(sGui->getCContextAudioBase()->isAudioLoopbackEnabled());

                // the connects depend on initAudioDeviceLists
                c = connect(ui->cb_SetupAudioInputDevice,  qOverload<int>(&QComboBox::currentIndexChanged), this, &CAudioDeviceVolumeSetupComponent::onAudioDeviceSelected, Qt::QueuedConnection);
                Q_ASSERT(c);
                c = connect(ui->cb_SetupAudioOutputDevice, qOverload<int>(&QComboBox::currentIndexChanged), this, &CAudioDeviceVolumeSetupComponent::onAudioDeviceSelected, Qt::QueuedConnection);
                Q_ASSERT(c);

                // context
                c = connect(sGui->getCContextAudioBase(), &CContextAudioBase::changedAudioDevices, this, &CAudioDeviceVolumeSetupComponent::onAudioDevicesChanged, Qt::QueuedConnection);
                Q_ASSERT(c);
                c = connect(sGui->getCContextAudioBase(), &CContextAudioBase::startedAudio, this, &CAudioDeviceVolumeSetupComponent::onAudioStarted, Qt::QueuedConnection);
                Q_ASSERT(c);
                c = connect(sGui->getCContextAudioBase(), &CContextAudioBase::stoppedAudio, this, &CAudioDeviceVolumeSetupComponent::onAudioStopped, Qt::QueuedConnection);
                Q_ASSERT(c);

                this->initWithAfvClient();
                m_init = true;
            }
            Q_UNUSED(c)
        }

        void CAudioDeviceVolumeSetupComponent::initWithAfvClient()
        {
            if (!afvClient()) { return; }
            m_afvConnections.disconnectAll();

            //! \todo Workaround to avoid context signals
            CAfvClient *afv = afvClient();
            const Qt::ConnectionType ct = Qt::QueuedConnection;
            QMetaObject::Connection c;
            c = connect(afv, &CAfvClient::outputVolumePeakVU, this, &CAudioDeviceVolumeSetupComponent::onOutputVU, ct);
            Q_ASSERT(c);
            m_afvConnections.append(c);
            c = connect(afv, &CAfvClient::inputVolumePeakVU, this, &CAudioDeviceVolumeSetupComponent::onInputVU, ct);
            Q_ASSERT(c);
            m_afvConnections.append(c);
            c = connect(afv, &CAfvClient::receivedCallsignsChanged, this, &CAudioDeviceVolumeSetupComponent::onReceivingCallsignsChanged, ct);
            Q_ASSERT(c);
            m_afvConnections.append(c);
            c = connect(afv, &CAfvClient::updatedFromOwnAircraftCockpit, this, &CAudioDeviceVolumeSetupComponent::onUpdatedClientWithCockpitData, ct);
            Q_ASSERT(c);
            m_afvConnections.append(c);

            QPointer<CAudioDeviceVolumeSetupComponent> myself(this);
            c = connect(afv, &CAfvClient::connectionStatusChanged, this, [ = ]
            {
                if (!myself || !sGui || sGui->isShuttingDown()) { return; }
                myself->setTransmitReceiveInUiFromVoiceClient();
            }, ct);
            Q_ASSERT(c);
            m_afvConnections.append(c);

            this->setTransmitReceiveInUiFromVoiceClient();
        }

        CAudioDeviceVolumeSetupComponent::~CAudioDeviceVolumeSetupComponent()
        { }

        int CAudioDeviceVolumeSetupComponent::getInValue(int from, int to) const
        {
            const double r  = ui->hs_VolumeIn->maximum() - ui->hs_VolumeIn->minimum();
            const double tr = to - from;
            return qRound(ui->hs_VolumeIn->value() / r * tr);
        }

        int CAudioDeviceVolumeSetupComponent::getOutValue(int from, int to) const
        {
            const double r = ui->hs_VolumeOut->maximum() - ui->hs_VolumeOut->minimum();
            const double tr = to - from;
            return qRound(ui->hs_VolumeOut->value() / r * tr);
        }

        void CAudioDeviceVolumeSetupComponent::setInValue(int value, int from, int to)
        {
            if (value > to)   { value = to; }
            else if (value < from) { value = from; }
            const double r = ui->hs_VolumeIn->maximum() - ui->hs_VolumeIn->minimum();
            const double tr = to - from;
            ui->hs_VolumeIn->setValue(qRound(value / tr * r));
        }

        void CAudioDeviceVolumeSetupComponent::setOutValue(int value, int from, int to)
        {
            if (value > to)   { value = to; }
            else if (value < from) { value = from; }
            const double r = ui->hs_VolumeOut->maximum() - ui->hs_VolumeOut->minimum();
            const double tr = to - from;
            ui->hs_VolumeOut->setValue(qRound(value / tr * r));
        }

        void CAudioDeviceVolumeSetupComponent::setInLevel(double value)
        {
            if (value > 1.0) { value = 1.0; }
            else if (value < 0.0) { value = 0.0; }
            ui->wip_InLevelMeter->levelChanged(value);
        }

        void CAudioDeviceVolumeSetupComponent::setOutLevel(double value)
        {
            if (value > 1.0) { value = 1.0; }
            else if (value < 0.0) { value = 0.0; }
            ui->wip_OutLevelMeter->levelChanged(value);
        }

        void CAudioDeviceVolumeSetupComponent::setInfo(const QString &info)
        {
            ui->le_Info->setText(info);
        }

        void CAudioDeviceVolumeSetupComponent::setTransmitReceiveInUi(bool tx1, bool rec1, bool tx2, bool rec2)
        {
            ui->cb_1Tx->setChecked(tx1);
            ui->cb_2Tx->setChecked(tx2);
            ui->cb_1Rec->setChecked(rec1);
            ui->cb_2Rec->setChecked(rec2);
        }

        void CAudioDeviceVolumeSetupComponent::setTransmitReceiveInUiFromVoiceClient()
        {
            if (!this->hasAudio())
            {
                ui->led_AudioConnected->setOn(false);
                return;
            }

            const bool on = sGui->getCContextAudioBase()->isAudioConnected();
            ui->led_AudioConnected->setOn(on);

            const bool com1Enabled = sGui->getCContextAudioBase()->isEnabledComUnit(CComSystem::Com1);
            const bool com2Enabled = sGui->getCContextAudioBase()->isEnabledComUnit(CComSystem::Com2);

            const bool com1Tx = com1Enabled && sGui->getCContextAudioBase()->isTransmittingComUnit(CComSystem::Com1);
            const bool com2Tx = com2Enabled && sGui->getCContextAudioBase()->isTransmittingComUnit(CComSystem::Com2);

            // we do not have receiving, so we use enable
            const bool com1Rx = com1Enabled;
            const bool com2Rx = com2Enabled;

            this->setTransmitReceiveInUi(com1Tx, com1Rx, com2Tx, com2Rx);
        }

        CAfvClient *CAudioDeviceVolumeSetupComponent::afvClient()
        {
            if (!sGui || sGui->isShuttingDown() || !sGui->getCContextAudioBase()) { return nullptr; }
            return sGui->getCContextAudioBase()->afvClient();
        }

        void CAudioDeviceVolumeSetupComponent::reloadSettings()
        {
            const CSettings as(m_audioSettings.getThreadLocal());
            ui->cb_DisableAudioEffects->setChecked(!as.isAudioEffectsEnabled());
            this->setInValue(as.getInVolume());
            this->setOutValue(as.getOutVolume());
        }

        void CAudioDeviceVolumeSetupComponent::initAudioDeviceLists()
        {
            if (!this->hasAudio()) { return; }
            const bool changed = this->onAudioDevicesChanged(sGui->getCContextAudioBase()->getAudioDevicesPlusDefault());
            if (!changed) { return; }
            const CAudioDeviceInfoList currentDevices = sGui->getCContextAudioBase()->getCurrentAudioDevices();
            this->onAudioStarted(currentDevices.getInputDevices().frontOrDefault(), currentDevices.getOutputDevices().frontOrDefault());
        }

        bool CAudioDeviceVolumeSetupComponent::hasAudio() const
        {
            return sGui && sGui->getCContextAudioBase();
        }

        void CAudioDeviceVolumeSetupComponent::onVolumeSliderChanged(int v)
        {
            Q_UNUSED(v)
            m_volumeSliderChanged.inputSignal();
        }

        void CAudioDeviceVolumeSetupComponent::saveVolumes()
        {
            CSettings as(m_audioSettings.getThreadLocal());
            const int i = this->getInValue();
            const int o = this->getOutValue();
            if (as.getInVolume() == i && as.getOutVolume() == o) { return; }
            as.setInVolume(i);
            as.setOutVolume(o);
            m_audioSettings.setAndSave(as);
        }

        void CAudioDeviceVolumeSetupComponent::onOutputVU(double vu)
        {
            this->setOutLevel(vu);
        }

        void CAudioDeviceVolumeSetupComponent::onInputVU(double vu)
        {
            this->setInLevel(vu);
        }

        void CAudioDeviceVolumeSetupComponent::onReloadDevices()
        {
            if (!hasAudio()) { return; }
            this->initAudioDeviceLists();
            const CAudioDeviceInfo i = this->getSelectedInputDevice();
            const CAudioDeviceInfo o = this->getSelectedOutputDevice();
            sGui->getCContextAudioBase()->setCurrentAudioDevices(i, o);
        }

        void CAudioDeviceVolumeSetupComponent::onResetVolumeIn()
        {
            ui->hs_VolumeIn->setValue((ui->hs_VolumeIn->maximum() - ui->hs_VolumeIn->minimum()) / 2);
        }

        void CAudioDeviceVolumeSetupComponent::onResetVolumeOut()
        {
            ui->hs_VolumeOut->setValue((ui->hs_VolumeOut->maximum() - ui->hs_VolumeOut->minimum()) / 2);
        }

        void CAudioDeviceVolumeSetupComponent::setAudioRunsWhere()
        {
            const QString ai = sGui->getCContextAudioBase()->audioRunsWhereInfo();
            ui->le_Info->setPlaceholderText(ai);
        }

        void CAudioDeviceVolumeSetupComponent::onReceivingCallsignsChanged(const CCallsignSet &com1Callsigns, const CCallsignSet &com2Callsigns)
        {
            const QString info = (com1Callsigns.isEmpty() ? QString() : QStringLiteral("COM1: ") % com1Callsigns.getCallsignsAsString()) %
                                 (!com1Callsigns.isEmpty() && !com2Callsigns.isEmpty() ? QStringLiteral(" | ") : QString()) %
                                 (com2Callsigns.isEmpty() ? QString() : QStringLiteral("COM2: ") % com2Callsigns.getCallsignsAsString());

            ui->led_Rx1->setOn(!com1Callsigns.isEmpty());
            ui->led_Rx2->setOn(!com2Callsigns.isEmpty());
            this->setInfo(info);
        }

        void CAudioDeviceVolumeSetupComponent::onUpdatedClientWithCockpitData()
        {
            this->setTransmitReceiveInUiFromVoiceClient();
        }

        CAudioDeviceInfo CAudioDeviceVolumeSetupComponent::getSelectedInputDevice() const
        {
            if (!hasAudio()) { return CAudioDeviceInfo(); }
            const CAudioDeviceInfoList devices = sGui->getCContextAudioBase()->getAudioInputDevicesPlusDefault();
            return devices.findByName(ui->cb_SetupAudioInputDevice->currentText());
        }

        CAudioDeviceInfo CAudioDeviceVolumeSetupComponent::getSelectedOutputDevice() const
        {
            if (!hasAudio()) { return CAudioDeviceInfo(); }
            const CAudioDeviceInfoList devices = sGui->getCContextAudioBase()->getAudioOutputDevicesPlusDefault();
            return devices.findByName(ui->cb_SetupAudioOutputDevice->currentText());
        }

        void CAudioDeviceVolumeSetupComponent::onAudioDeviceSelected(int index)
        {
            if (!sGui || sGui->isShuttingDown() || !sGui->getIContextAudio()) { return; }
            if (index < 0) { return; }

            const CAudioDeviceInfo in  = this->getSelectedInputDevice();
            const CAudioDeviceInfo out = this->getSelectedOutputDevice();
            sGui->getCContextAudioBase()->setCurrentAudioDevices(in, out);
        }

        void CAudioDeviceVolumeSetupComponent::onAudioStarted(const CAudioDeviceInfo &input, const CAudioDeviceInfo &output)
        {
            if (!afvClient()) { return; }
            if (m_afvConnections.isEmpty() && m_init)
            {
                this->initWithAfvClient();
            }

            ui->cb_SetupAudioInputDevice->setCurrentText(input.toQString(true));
            ui->cb_SetupAudioOutputDevice->setCurrentText(output.toQString(true));
            this->setAudioRunsWhere();
        }

        void CAudioDeviceVolumeSetupComponent::onAudioStopped()
        {
            this->setAudioRunsWhere();
            if (!afvClient())
            {
                m_afvConnections.disconnectAll();
            }
        }

        bool CAudioDeviceVolumeSetupComponent::onAudioDevicesChanged(const CAudioDeviceInfoList &devices)
        {
            if (m_cbDevices.hasSameDevices(devices)) { return false; } // avoid numerous follow up actions
            m_cbDevices = devices;

            this->setAudioRunsWhere();
            ui->cb_SetupAudioOutputDevice->clear();
            ui->cb_SetupAudioInputDevice->clear();

            const QString i = ui->cb_SetupAudioInputDevice->currentText();
            const QString o = ui->cb_SetupAudioOutputDevice->currentText();

            for (const CAudioDeviceInfo &device : devices)
            {
                if (device.getType() == CAudioDeviceInfo::InputDevice)
                {
                    ui->cb_SetupAudioInputDevice->addItem(device.toQString(true));
                }
                else if (device.getType() == CAudioDeviceInfo::OutputDevice)
                {
                    ui->cb_SetupAudioOutputDevice->addItem(device.toQString(true));
                }
            }

            if (!i.isEmpty()) { ui->cb_SetupAudioInputDevice->setCurrentText(i); }
            if (!o.isEmpty()) { ui->cb_SetupAudioOutputDevice->setCurrentText(o); }

            return true;
        }

        void CAudioDeviceVolumeSetupComponent::onLoopbackToggled(bool loopback)
        {
            if (!sGui || sGui->isShuttingDown() || !sGui->getIContextAudio()) { return; }
            if (sGui->getCContextAudioBase()->isAudioLoopbackEnabled() == loopback) { return; }
            sGui->getCContextAudioBase()->enableAudioLoopback(loopback);
        }

        void CAudioDeviceVolumeSetupComponent::onDisableAudioEffectsToggled(bool disabled)
        {
            if (!sGui || sGui->isShuttingDown() || !sGui->getIContextAudio()) { return; }
            CSettings as(m_audioSettings.getThreadLocal());
            const bool enabled = !disabled;
            if (as.isAudioEffectsEnabled() == enabled) { return; }
            as.setAudioEffectsEnabled(enabled);
            m_audioSettings.setAndSave(as);
        }
    } // namespace
} // namespace
