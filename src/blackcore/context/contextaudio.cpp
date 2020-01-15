/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "contextaudio.h"

#include "blackcore/afv/clients/afvclient.h"
#include "blackcore/context/contextaudio.h"
#include "blackcore/context/contextnetwork.h"      // for user login
#include "blackcore/context/contextownaircraft.h"  // for COM integration
#include "blackcore/context/contextsimulator.h"    // for COM intergration
#include "blackcore/context/contextaudioimpl.h"
#include "blackcore/context/contextaudioproxy.h"
#include "blackmisc/simplecommandparser.h"
#include "blackmisc/dbusserver.h"
#include "blackmisc/stringutils.h"
#include "blackmisc/verify.h"
#include "blackmisc/icons.h"
#include "blackconfig/buildconfig.h"

#ifdef Q_OS_WIN
#include "comdef.h"
#endif

using namespace BlackMisc;
using namespace BlackMisc::Aviation;
using namespace BlackMisc::Audio;
using namespace BlackMisc::Network;
using namespace BlackMisc::PhysicalQuantities;
using namespace BlackMisc::Simulation;
using namespace BlackSound;
using namespace BlackCore::Afv::Clients;

//! \cond

namespace BlackCore
{
    namespace Context
    {
        IContextAudio::IContextAudio(CCoreFacadeConfig::ContextMode mode, CCoreFacade *runtime) :
            IContext(mode, runtime)
        {
            // void
        }

        void IContextAudio::onChangedLocalDevices(const CAudioDeviceInfoList &devices)
        {
            this->registerDevices(devices);
        }

        const QString &IContextAudio::InterfaceName()
        {
            static const QString s(BLACKCORE_CONTEXTAUDIO_INTERFACENAME);
            return s;
        }

        const QString &IContextAudio::ObjectPath()
        {
            static const QString s(BLACKCORE_CONTEXTAUDIO_OBJECTPATH);
            return s;
        }

        IContextAudio *IContextAudio::create(CCoreFacade *runtime, CCoreFacadeConfig::ContextMode mode, CDBusServer *server, QDBusConnection &connection)
        {
            // for audio no empty context is available
            // since CContextAudioBaseImpl provides audio on either side (core/GUI) we do not use ContextAudioEmpty
            // ContextAudioEmpty would cause issue, as it is initializing "common parts" during shutdown
            switch (mode)
            {
            case CCoreFacadeConfig::Local:
            case CCoreFacadeConfig::LocalInDBusServer:
            default:
                return (new CContextAudio(mode, runtime))->registerWithDBus(server);
            case CCoreFacadeConfig::Remote:
                return new CContextAudioProxy(CDBusServer::coreServiceName(connection), connection, mode, runtime);
            case CCoreFacadeConfig::NotUsed:
                BLACK_VERIFY_X(false, Q_FUNC_INFO, "Empty context not supported for audio (since AFV)");
                return nullptr;
            }
        }

        bool CContextAudioBase::parseCommandLine(const QString &commandLine, const CIdentifier &originator)
        {
            Q_UNUSED(originator)
            if (commandLine.isEmpty()) { return false; }
            CSimpleCommandParser parser(
            {
                ".vol", ".volume",    // output volume
                ".mute",              // mute
                ".unmute",            // unmute
                ".aliased"
            });
            parser.parse(commandLine);
            if (!parser.isKnownCommand()) { return false; }

            if (parser.matchesCommand(".mute"))
            {
                this->setMute(true);
                return true;
            }
            else if (parser.matchesCommand(".unmute"))
            {
                this->setMute(false);
                return true;
            }
            else if (parser.commandStartsWith("vol") && parser.countParts() > 1)
            {
                const int v = parser.toInt(1);
                this->setVoiceOutputVolume(v);
                return true;
            }
            else if (afvClient() && parser.matchesCommand(".aliased") && parser.countParts() > 1)
            {
                const bool enable = parser.toBool(1, true);
                afvClient()->enableAliasedStations(enable);

                CLogMessage(this).info(u"Aliased stations are: %1") << boolToOnOff(enable);
                return true;
            }
            return false;
        }

        CContextAudioBase::CContextAudioBase(CCoreFacadeConfig::ContextMode mode, CCoreFacade *runtime) :
            IContextAudio(mode, runtime),
            CIdentifiable(this)
        {
            CContextAudioBase::registerHelp();

            if (CContextAudioBase::isNoAudioSet())
            {
                CLogMessage(this).info(u"Voice client disabled");
            }
            else
            {
                this->initVoiceClient();
            }

            const CSettings as = m_audioSettings.getThreadLocal();
            this->setVoiceOutputVolume(as.getOutVolume());
            m_selcalPlayer = new CSelcalPlayer(CAudioDeviceInfo::getDefaultOutputDevice(), this);

            this->changeDeviceSettings();
            QPointer<CContextAudioBase> myself(this);
            QTimer::singleShot(5000, this, [ = ]
            {
                if (!myself || !sApp || sApp->isShuttingDown()) { return; }
                // myself->changeDeviceSettings();
                myself->onChangedAudioSettings();
                myself->onChangedLocalDevices(m_activeLocalDevices);
            });
        }

        CContextAudioBase::~CContextAudioBase()
        {
            this->gracefulShutdown();
        }

        void CContextAudioBase::initVoiceClient()
        {
            if (m_voiceClient) { return; }
            m_voiceClient = new CAfvClient(CVoiceSetup().getAfvVoiceServerUrl(), this);

            const CVoiceSetup vs = m_voiceSettings.getThreadLocal();
            m_voiceClient->updateVoiceServerUrl(vs.getAfvVoiceServerUrl());

#ifdef Q_OS_WIN
            if (!m_winCoInitialized)
            {
                HRESULT hr = CoInitializeEx(nullptr,  COINIT_MULTITHREADED);

                // RPC_E_CHANGED_MODE: CoInitializeEx was already called by someone else in this thread with a different mode.
                if (hr == RPC_E_CHANGED_MODE)
                {
                    CLogMessage(this).debug(u"CoInitializeEx was already called with a different mode. Trying again.");
                    hr = CoInitializeEx(nullptr,  COINIT_APARTMENTTHREADED);
                }

                // S_OK: The COM library was initialized successfully on this thread.
                // S_FALSE: The COM library is already initialized on this thread. Reference count was incremented. This is not an error.
                if (hr == S_OK || hr == S_FALSE) { m_winCoInitialized = true; }
            }
#endif

            Q_ASSERT_X(CThreadUtils::isApplicationThread(m_voiceClient->thread()), Q_FUNC_INFO, "Should be in main thread");
            m_voiceClient->start(); // thread
            Q_ASSERT_X(m_voiceClient->owner() == this, Q_FUNC_INFO, "Wrong owner");
            Q_ASSERT_X(!CThreadUtils::isApplicationThread(m_voiceClient->thread()), Q_FUNC_INFO, "Must NOT be in main thread");

            // connect(m_voiceClient, &CAfvClient::outputVolumePeakVU,            this, &CContextAudioBase::outputVolumePeakVU, Qt::QueuedConnection);
            // connect(m_voiceClient, &CAfvClient::inputVolumePeakVU,             this, &CContextAudioBase::inputVolumePeakVU,  Qt::QueuedConnection);
            // connect(m_voiceClient, &CAfvClient::receivingCallsignsChanged,     this, &CContextAudioBase::receivingCallsignsChanged,     Qt::QueuedConnection);
            // connect(m_voiceClient, &CAfvClient::updatedFromOwnAircraftCockpit, this, &CContextAudioBase::updatedFromOwnAircraftCockpit, Qt::QueuedConnection);
            connect(m_voiceClient, &CAfvClient::startedAudio, this, &CContextAudioBase::startedAudio, Qt::QueuedConnection);
            connect(m_voiceClient, &CAfvClient::stoppedAudio, this, &CContextAudioBase::stoppedAudio, Qt::QueuedConnection);
            connect(m_voiceClient, &CAfvClient::ptt,          this, &CContextAudioBase::ptt,          Qt::QueuedConnection);
            connect(m_voiceClient, &CAfvClient::connectionStatusChanged, this, &CContextAudioBase::onAfvConnectionStatusChanged, Qt::QueuedConnection);

            const CAudioDeviceInfoList devices = CAudioDeviceInfoList::allDevices();
            if (devices != m_activeLocalDevices)
            {
                m_activeLocalDevices = devices;
                emit this->changedLocalAudioDevices(devices);
            }
        }

        void CContextAudioBase::terminateVoiceClient()
        {
            if (m_voiceClient)
            {
                m_voiceClient->gracefulShutdown();
                Q_ASSERT_X(CThreadUtils::isCurrentThreadObjectThread(m_voiceClient), Q_FUNC_INFO, "Needs to be back in current thread");
                m_voiceClient->deleteLater();
                m_voiceClient = nullptr;
#ifdef Q_OS_WIN
                if (m_winCoInitialized)
                {
                    CoUninitialize();
                    m_winCoInitialized = false;
                }
#endif
            }
        }

        void CContextAudioBase::gracefulShutdown()
        {
            this->terminateVoiceClient();
            QObject::disconnect(this);
        }

        void CContextAudioBase::enableVoiceClientAndStart()
        {
            this->initVoiceClient();
            if (m_voiceClient)
            {
                m_voiceClient->startAudio();
                const bool connected = this->connectAudioWithNetworkCredentials();
                Q_UNUSED(connected)

                // one reason for not connecting is NOT using the VATSIM ecosystem
            }
        }

        const CIdentifier &CContextAudioBase::audioRunsWhere() const
        {
            static const CIdentifier i("CContextAudioBaseImpl");
            return i;
        }

        bool CContextAudioBase::isEnabledComUnit(CComSystem::ComUnit comUnit) const
        {
            if (!m_voiceClient) { return false; }
            return m_voiceClient->isEnabledComUnit(comUnit);
        }

        bool CContextAudioBase::isTransmittingComUnit(CComSystem::ComUnit comUnit) const
        {
            if (!m_voiceClient) { return false; }
            return m_voiceClient->isTransmittingdComUnit(comUnit);
        }

        bool CContextAudioBase::connectAudioWithNetworkCredentials()
        {
            if (!m_voiceClient) { return false; }
            if (!sApp || sApp->isShuttingDown() || !sApp->getIContextNetwork()) { return false; }

            const CEcosystem ecoSystem = this->getIContextNetwork()->getConnectedServer().getEcosystem();
            if (ecoSystem != CEcosystem::vatsim())
            {
                CLogMessage(this).info(u"Will not use AFV as ecosystem is '%1'") << ecoSystem.toQString(true);
                return false;
            }

            const CVoiceSetup vs = m_voiceSettings.getThreadLocal();
            m_voiceClient->updateVoiceServerUrl(vs.getAfvVoiceServerUrl());

            const CUser connectedUser = this->getIContextNetwork()->getConnectedServer().getUser();
            const QString client = "swift " % BlackConfig::CBuildConfig::getShortVersionString();
            CCallsign cs = connectedUser.getCallsign();
            this->unRegisterAudioCallsign(cs, this->identifier()); // un-register "myself"
            if (this->hasRegisteredAudioCallsign(cs)) // anybody else using that callsign
            {
                //! \todo KB 2019-11 would need a better algorithm to really find a cs
                cs = CCallsign(cs.asString() +  "2");
            }
            CLogMessage(this).info(u"About to connect to voice as '%1' '%2'") << connectedUser.getId() << cs;
            m_voiceClient->connectTo(connectedUser.getId(), connectedUser.getPassword(), cs.asString(), client);
            this->registerAudioCallsign(cs, this->identifier()); // login can still fail, but we "block" this callsign
            return true;
        }

        bool CContextAudioBase::isAudioConnected() const
        {
            return m_voiceClient && m_voiceClient->isConnected();
        }

        bool CContextAudioBase::isAudioStarted() const
        {
            return m_voiceClient && m_voiceClient->isStarted();
        }

        const QList<QCommandLineOption> &CContextAudioBase::getCmdLineOptions()
        {
            static const QList<QCommandLineOption> opts
            {
                QCommandLineOption({{"n", "noaudio"}, QCoreApplication::translate("CContextAudioBase", "No audio for GUI or core.", "noaudio")})
            };
            return opts;
        }

        bool CContextAudioBase::isNoAudioSet()
        {
            if (!sApp) { return false; }
            return sApp->isParserOptionSet("noaudio");
        }

        QString CContextAudioBase::audioRunsWhereInfo() const
        {
            const QString s = QStringLiteral("[%1] Audio on '%2', '%3'.").arg(boolToEnabledDisabled(this->isAudioStarted()), audioRunsWhere().getMachineName(), audioRunsWhere().getProcessName());
            return s;
        }

        CAudioDeviceInfoList CContextAudioBase::getAudioDevices() const
        {
            return CAudioDeviceInfoList::allDevices();
        }

        CAudioDeviceInfoList CContextAudioBase::getAudioInputDevices() const
        {
            return this->getAudioDevices().getInputDevices();
        }

        CAudioDeviceInfoList CContextAudioBase::getAudioOutputDevices() const
        {
            return this->getAudioDevices().getOutputDevices();
        }

        CAudioDeviceInfoList CContextAudioBase::getAudioDevicesPlusDefault() const
        {
            return CAudioDeviceInfoList::allDevicesPlusDefault();
        }

        CAudioDeviceInfoList CContextAudioBase::getAudioInputDevicesPlusDefault() const
        {
            return this->getAudioDevicesPlusDefault().getInputDevices();
        }

        CAudioDeviceInfoList CContextAudioBase::getAudioOutputDevicesPlusDefault() const
        {
            return this->getAudioDevicesPlusDefault().getOutputDevices();
        }

        CAudioDeviceInfoList CContextAudioBase::getCurrentAudioDevices() const
        {
            const QString inputDeviceName = m_inputDeviceSetting.get();
            const CAudioDeviceInfo inputDevice = this->getAudioInputDevicesPlusDefault().findByNameOrDefault(inputDeviceName, CAudioDeviceInfo::getDefaultInputDevice());

            const QString outputDeviceName = m_outputDeviceSetting.get();
            const CAudioDeviceInfo outputDevice = this->getAudioOutputDevicesPlusDefault().findByNameOrDefault(outputDeviceName, CAudioDeviceInfo::getDefaultOutputDevice());

            CAudioDeviceInfoList devices;
            devices.push_back(inputDevice);
            devices.push_back(outputDevice);
            return devices;
        }

        void CContextAudioBase::setCurrentAudioDevices(const CAudioDeviceInfo &inputDevice, const CAudioDeviceInfo &outputDevice)
        {
            if (!m_voiceClient) { return; }

            if (!inputDevice.getName().isEmpty() && inputDevice.getName() != m_inputDeviceSetting.get())
            {
                Q_ASSERT_X(inputDevice.isInputDevice(), Q_FUNC_INFO, "Need input device");
                const CStatusMessage m = m_inputDeviceSetting.setAndSave(inputDevice.getName());
                CLogMessage::preformatted(m);
            }
            if (!outputDevice.getName().isEmpty() && outputDevice.getName() != m_outputDeviceSetting.get())
            {
                Q_ASSERT_X(outputDevice.isOutputDevice(), Q_FUNC_INFO, "Need output device");
                const CStatusMessage m = m_outputDeviceSetting.setAndSave(outputDevice.getName());
                CLogMessage::preformatted(m);
            }

            m_voiceClient->startAudio(inputDevice, outputDevice);
        }

        void CContextAudioBase::setVoiceOutputVolume(int volume)
        {
            if (!m_voiceClient) { return; }

            const bool wasMuted = this->isMuted();
            volume = CSettings::fixOutVolume(volume);

            const int  currentVolume = m_voiceClient->getNormalizedOutputVolume();
            const bool changedVoiceOutput = (currentVolume != volume);
            if (changedVoiceOutput)
            {
                m_voiceClient->setNormalizedOutputVolume(volume);
                m_outVolumeBeforeMute = volume;

                emit this->changedAudioVolume(volume);
                if ((volume > 0 && wasMuted) || (volume < 1 && !wasMuted))
                {
                    // inform about muted
                    emit this->changedMute(volume < 1);
                }
            }

            CSettings as(m_audioSettings.getThreadLocal());
            if (as.getOutVolume() != volume)
            {
                as.setOutVolume(volume);
                m_audioSettings.set(as);
            }
        }

        int CContextAudioBase::getVoiceOutputVolume() const
        {
            if (!m_voiceClient) { return 0; }
            return m_voiceClient->getNormalizedOutputVolume();
        }

        void CContextAudioBase::setMute(bool muted)
        {
            if (!m_voiceClient) { return; }
            if (this->isMuted() == muted) { return; } // avoid roundtrips / unnecessary signals

            if (muted)
            {
                const int nv = m_voiceClient->getNormalizedOutputVolume();
                m_outVolumeBeforeMute = nv;
            }

            m_voiceClient->setMuted(muted);
            if (!muted) { m_voiceClient->setNormalizedOutputVolume(m_outVolumeBeforeMute); }

            // signal
            emit this->changedMute(muted);
        }

        bool CContextAudioBase::isMuted() const
        {
            if (!m_voiceClient) { return false; }
            return m_voiceClient->isMuted();
        }

        void CContextAudioBase::playSelcalTone(const CSelcal &selcal)
        {
            const CTime t = m_selcalPlayer->play(90, selcal);
            const int ms = t.toMs();
            if (ms > 10)
            {
                // As of https://dev.swift-project.org/T558 play additional notification
                const QPointer<const CContextAudioBase> myself(this);
                QTimer::singleShot(ms, this, [ = ]
                {
                    if (!sApp || sApp->isShuttingDown() || !myself) { return; }
                    this->playNotification(CNotificationSounds::NotificationTextMessageSupervisor, true);
                });
            }
        }

        void CContextAudioBase::playNotification(CNotificationSounds::NotificationFlag notification, bool considerSettings, int volume)
        {
            if (m_debugEnabled) { CLogMessage(this, CLogCategory::contextSlot()).debug() << Q_FUNC_INFO << notification; }

            const CSettings settings = m_audioSettings.getThreadLocal();
            const bool play = !considerSettings || settings.isNotificationFlagSet(notification);
            if (!play) { return; }
            if (notification == CNotificationSounds::PTTClickKeyDown && (considerSettings && settings.noAudioTransmission()))
            {
                /**
                if (!this->canTalk())
                {
                    // warning sound
                    notification = CNotificationSounds::NotificationNoAudioTransmission;
                }
                **/
            }

            if (volume < 0 || volume > 100)
            {
                volume = 90;
                if (considerSettings) { volume = qMax(25, settings.getNotificationVolume()); }
            }
            m_notificationPlayer.play(notification, volume);
        }

        void CContextAudioBase::enableAudioLoopback(bool enable)
        {
            if (!m_voiceClient) { return; }
            m_voiceClient->setLoopBack(enable);
        }

        bool CContextAudioBase::isAudioLoopbackEnabled() const
        {
            if (!m_voiceClient) { return false; }
            return m_voiceClient->isLoopback();
        }

        void CContextAudioBase::setVoiceSetup(const CVoiceSetup &setup)
        {
            // could be recycled for some AFV setup
            Q_UNUSED(setup)
        }

        CVoiceSetup CContextAudioBase::getVoiceSetup() const
        {
            return CVoiceSetup();
        }

        void CContextAudioBase::setVoiceTransmission(bool enable, PTTCOM com)
        {
            if (!m_voiceClient) { return; }
            m_voiceClient->setPttForCom(enable, com);
        }

        void CContextAudioBase::setVoiceTransmissionCom1(bool enabled)
        {
            this->setVoiceTransmission(enabled, COM1);
        }

        void CContextAudioBase::setVoiceTransmissionCom2(bool enabled)
        {
            this->setVoiceTransmission(enabled, COM2);
        }

        void CContextAudioBase::setVoiceTransmissionComActive(bool enabled)
        {
            this->setVoiceTransmission(enabled, COMActive);
        }

        void CContextAudioBase::changeDeviceSettings()
        {
            const CAudioDeviceInfoList devices = this->getCurrentAudioDevices();
            Q_ASSERT_X(devices.size() == 2, Q_FUNC_INFO, "Expect INPUT and OUTPUT device");

            const CAudioDeviceInfo input  = devices.front();
            const CAudioDeviceInfo output = devices.back();
            this->setCurrentAudioDevices(input, output);
        }

        void CContextAudioBase::onChangedAudioSettings()
        {
            const CSettings s = m_audioSettings.get();
            const QString dir = s.getNotificationSoundDirectory();
            m_notificationPlayer.updateDirectory(dir);
            this->setVoiceOutputVolume(s.getOutVolume());
        }

        void CContextAudioBase::onChangedVoiceSettings()
        {
            const CVoiceSetup vs = m_voiceSettings.getThreadLocal();
            m_voiceClient->updateVoiceServerUrl(vs.getAfvVoiceServerUrl());
        }

        void CContextAudioBase::audioIncreaseVolume(bool enabled)
        {
            if (!enabled) { return; }
            const int v = qRound(this->getVoiceOutputVolume() * 1.2);
            this->setVoiceOutputVolume(v);
        }

        void CContextAudioBase::audioDecreaseVolume(bool enabled)
        {
            if (!enabled) { return; }
            const int v = qRound(this->getVoiceOutputVolume() / 1.2);
            this->setVoiceOutputVolume(v);
        }

        void CContextAudioBase::xCtxNetworkConnectionStatusChanged(const CConnectionStatus &from, const CConnectionStatus &to)
        {
            if (!m_voiceClient) { return; }

            Q_UNUSED(from)
            BLACK_VERIFY_X(this->getIContextNetwork(), Q_FUNC_INFO, "Missing network context");

            // we only change network connection of AFV client here
            if (to.isConnected() && this->getIContextNetwork())
            {
                const bool connected = this->connectAudioWithNetworkCredentials();
                Q_UNUSED(connected)

                // one reason for not connecting is NOT using the VATSIM ecosystem
            }
            else if (to.isDisconnected())
            {
                m_voiceClient->disconnectFrom();
            }
        }

        void CContextAudioBase::onAfvConnectionStatusChanged(int status)
        {
            if (!m_voiceClient) { return; }

            const CCallsign cs = m_voiceClient->getCallsign();
            const CAfvClient::ConnectionStatus s = static_cast<CAfvClient::ConnectionStatus>(status);

            switch (s)
            {
            case CAfvClient::Connected:
                this->registerAudioCallsign(cs, this->identifier());
                break;
            case CAfvClient::Disconnected:
                this->unRegisterAudioCallsign(cs, this->identifier());
                break;
            }
        }
    } // ns
} // ns

//! \endcond
