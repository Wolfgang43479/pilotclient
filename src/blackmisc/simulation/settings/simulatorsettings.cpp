/* Copyright (C) 2016
 * swift project community / contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "simulatorsettings.h"
#include "blackmisc/simulation/simulatedaircraft.h"
#include "blackmisc/simulation/fscommon/fscommonutil.h"
#include "blackmisc/simulation/xplane/xplaneutil.h"
#include "blackmisc/stringutils.h"
#include <QStringBuilder>

using namespace BlackMisc;
using namespace BlackMisc::PhysicalQuantities;
using namespace BlackMisc::Simulation::FsCommon;
using namespace BlackMisc::Simulation::XPlane;

namespace BlackMisc
{
    namespace Simulation
    {
        namespace Settings
        {
            CSimulatorSettings::CSimulatorSettings()
            { }

            void CSimulatorSettings::setSimulatorDirectory(const QString &simulatorDirectory)
            {
                m_simulatorDirectory = simulatorDirectory.trimmed();
            }

            const QString &CSimulatorSettings::getSimulatorDirectory() const
            {
                return m_simulatorDirectory;
            }

            void CSimulatorSettings::setModelDirectories(const QStringList &modelDirectories)
            {
                m_modelDirectories = modelDirectories;
            }

            void CSimulatorSettings::setModelDirectory(const QString &modelDirectory)
            {
                m_modelDirectories = QStringList({ modelDirectory });
            }

            const QStringList &CSimulatorSettings::getModelDirectories() const
            {
                return m_modelDirectories;
            }

            void CSimulatorSettings::setModelExcludeDirectories(const QStringList &excludeDirectories)
            {
                m_excludeDirectoryPatterns = excludeDirectories;
            }

            const QStringList &CSimulatorSettings::getModelExcludeDirectoryPatterns() const
            {
                return m_excludeDirectoryPatterns;
            }

            void CSimulatorSettings::resetPaths()
            {
                m_excludeDirectoryPatterns.clear();
                m_modelDirectories.clear();
                m_simulatorDirectory.clear();
            }

            QString CSimulatorSettings::convertToQString(bool i18n) const
            {
                return convertToQString(", ", i18n);
            }

            QString CSimulatorSettings::convertToQString(const QString &separator, bool i18n) const
            {
                Q_UNUSED(i18n);
                return QStringLiteral("model directories: ") %
                       m_modelDirectories.join(',') %
                       separator %
                       QStringLiteral("exclude directories: ") %
                       m_excludeDirectoryPatterns.join(',');
            }

            CVariant CSimulatorSettings::propertyByIndex(const CPropertyIndex &index) const
            {
                if (index.isMyself()) { return CVariant::from(*this); }
                const ColumnIndex i = index.frontCasted<ColumnIndex>();
                switch (i)
                {
                case IndexSimulatorDirectory: return CVariant::fromValue(m_simulatorDirectory);
                case IndexModelDirectory: return CVariant::fromValue(m_modelDirectories);
                case IndexModelExcludeDirectoryPatterns: return CVariant::fromValue(m_excludeDirectoryPatterns);
                default: return CValueObject::propertyByIndex(index);
                }
            }

            void CSimulatorSettings::setPropertyByIndex(const CPropertyIndex &index, const CVariant &variant)
            {
                if (index.isMyself()) { (*this) = variant.to<CSimulatorSettings>(); return; }
                const ColumnIndex i = index.frontCasted<ColumnIndex>();
                switch (i)
                {
                case IndexSimulatorDirectory: this->setSimulatorDirectory(variant.toQString()); break;
                case IndexModelDirectory: this->setSimulatorDirectory(variant.toQString()); break;
                case IndexModelExcludeDirectoryPatterns: m_excludeDirectoryPatterns = variant.value<QStringList>(); break;
                default: CValueObject::setPropertyByIndex(index, variant); break;
                }
            }

            CMultiSimulatorSettings::CMultiSimulatorSettings(QObject *parent) : QObject(parent)
            {
                // void
            }

            CSimulatorSettings CMultiSimulatorSettings::getSettings(const CSimulatorInfo &simulator) const
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return m_simSettingsFs9.get();
                case CSimulatorInfo::FSX: return m_simSettingsFsx.get();
                case CSimulatorInfo::P3D: return m_simSettingsP3D.get();
                case CSimulatorInfo::XPLANE: return m_simSettingsXP.get();
                default:
                    Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                    break;
                }
                return CSimulatorSettings();
            }

            CSpecializedSimulatorSettings CMultiSimulatorSettings::getSpecializedSettings(const CSimulatorInfo &simulator) const
            {
                return CSpecializedSimulatorSettings(this->getSettings(simulator), simulator);
            }

            CStatusMessage CMultiSimulatorSettings::setSettings(const CSimulatorSettings &settings, const CSimulatorInfo &simulator)
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return m_simSettingsFs9.set(settings);
                case CSimulatorInfo::FSX: return m_simSettingsFsx.set(settings);
                case CSimulatorInfo::P3D: return m_simSettingsP3D.set(settings);
                case CSimulatorInfo::XPLANE: return m_simSettingsXP.set(settings);
                default:
                    Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                    break;
                }
                return CStatusMessage({ CLogCategory::settings() }, CStatusMessage::SeverityError, "wrong simulator");
            }

            CStatusMessage CMultiSimulatorSettings::setAndSaveSettings(const CSimulatorSettings &settings, const CSimulatorInfo &simulator)
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return m_simSettingsFs9.setAndSave(settings);
                case CSimulatorInfo::FSX: return m_simSettingsFsx.setAndSave(settings);
                case CSimulatorInfo::P3D: return m_simSettingsP3D.setAndSave(settings);
                case CSimulatorInfo::XPLANE: return m_simSettingsXP.setAndSave(settings);
                default:
                    Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                    break;
                }
                return CStatusMessage({ CLogCategory::settings() }, CStatusMessage::SeverityError, "wrong simulator");
            }

            CStatusMessage CMultiSimulatorSettings::saveSettings(const CSimulatorInfo &simulator)
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return m_simSettingsFs9.save();
                case CSimulatorInfo::FSX: return m_simSettingsFsx.save();
                case CSimulatorInfo::P3D: return m_simSettingsP3D.save();
                case CSimulatorInfo::XPLANE: return m_simSettingsXP.save();
                default:
                    Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                    break;
                }
                return CStatusMessage({ CLogCategory::settings() }, CStatusMessage::SeverityError, "wrong simulator");
            }

            QString CMultiSimulatorSettings::getSimulatorDirectoryIfNotDefault(const CSimulatorInfo &simulator) const
            {
                const CSpecializedSimulatorSettings s = this->getSpecializedSettings(simulator);
                return s.getSimulatorDirectoryIfNotDefault();
            }

            QString CMultiSimulatorSettings::getSimulatorDirectoryOrDefault(const CSimulatorInfo &simulator) const
            {
                const CSpecializedSimulatorSettings s = this->getSpecializedSettings(simulator);
                return s.getSimulatorDirectoryOrDefault();
            }

            QStringList CMultiSimulatorSettings::getModelDirectoriesIfNotDefault(const CSimulatorInfo &simulator) const
            {
                const CSpecializedSimulatorSettings s = this->getSpecializedSettings(simulator);
                return s.getModelDirectoriesIfNotDefault();
            }

            QStringList CMultiSimulatorSettings::getModelDirectoriesOrDefault(const CSimulatorInfo &simulator) const
            {
                const CSpecializedSimulatorSettings s = this->getSpecializedSettings(simulator);
                return s.getModelDirectoriesOrDefault();
            }

            QString CMultiSimulatorSettings::getFirstModelDirectoryOrDefault(const CSimulatorInfo &simulator) const
            {
                const CSpecializedSimulatorSettings s = this->getSpecializedSettings(simulator);
                return s.getFirstModelDirectoryOrDefault();
            }

            QStringList CMultiSimulatorSettings::getModelExcludeDirectoryPatternsIfNotDefault(const CSimulatorInfo &simulator) const
            {
                const CSpecializedSimulatorSettings s = this->getSpecializedSettings(simulator);
                return s.getModelExcludeDirectoryPatternsIfNotDefault();
            }

            QStringList CMultiSimulatorSettings::getModelExcludeDirectoryPatternsOrDefault(const CSimulatorInfo &simulator) const
            {
                const CSpecializedSimulatorSettings s = this->getSpecializedSettings(simulator);
                return s.getModelExcludeDirectoryPatternsOrDefault();
            }

            void CMultiSimulatorSettings::resetToDefaults(const CSimulatorInfo &simulator)
            {
                CSimulatorSettings s = this->getSettings(simulator);
                s.resetPaths();
                this->setAndSaveSettings(s, simulator);
            }

            const QStringList &CMultiSimulatorSettings::defaultModelDirectories(const CSimulatorInfo &simulator) const
            {
                return CSpecializedSimulatorSettings::defaultModelDirectories(simulator);
            }

            const QStringList &CMultiSimulatorSettings::defaultModelExcludeDirectoryPatterns(const CSimulatorInfo &simulator)
            {
                return CSpecializedSimulatorSettings::defaultModelExcludeDirectoryPatterns(simulator);
            }

            const QString &CMultiSimulatorSettings::defaultSimulatorDirectory(const CSimulatorInfo &simulator)
            {
                return CSpecializedSimulatorSettings::defaultSimulatorDirectory(simulator);
            }

            void CSimulatorMessagesSettings::setTechnicalLogSeverity(CStatusMessage::StatusSeverity severity)
            {
                m_technicalLogLevel = static_cast<int>(severity);
            }

            void CSimulatorMessagesSettings::disableTechnicalMessages()
            {
                m_technicalLogLevel = -1;
            }

            bool CSimulatorMessagesSettings::isRelayErrorsMessages() const
            {
                if (m_technicalLogLevel < 0) { return false; }
                return (m_technicalLogLevel <= CStatusMessage::SeverityError);
            }

            bool CSimulatorMessagesSettings::isRelayWarningMessages() const
            {
                if (m_technicalLogLevel < 0) { return false; }
                return (m_technicalLogLevel <= CStatusMessage::SeverityWarning);
            }

            bool CSimulatorMessagesSettings::isRelayInfoMessages() const
            {
                if (m_technicalLogLevel < 0) { return false; }
                return (m_technicalLogLevel <= CStatusMessage::SeverityInfo);
            }

            bool CSimulatorMessagesSettings::isRelayTechnicalMessages() const
            {
                return (m_technicalLogLevel >= 0);
            }

            void CSimulatorMessagesSettings::setRelayTextMessages(CSimulatorMessagesSettings::TextMessageType messageType)
            {
                m_messageType = static_cast<int>(messageType);
            }

            bool CSimulatorMessagesSettings::isRelaySupervisorTextMessages() const
            {
                return this->getRelayedTextMessageTypes().testFlag(TextMessageSupervisor);
            }

            bool CSimulatorMessagesSettings::isRelayPrivateTextMessages() const
            {
                return this->getRelayedTextMessageTypes().testFlag(TextMessagePrivate);
            }

            bool CSimulatorMessagesSettings::isRelayUnicomTextMessages() const
            {
                return this->getRelayedTextMessageTypes().testFlag(TextMessagesUnicom);
            }

            bool CSimulatorMessagesSettings::isRelayCom1TextMessages() const
            {
                return this->getRelayedTextMessageTypes().testFlag(TextMessagesCom1);
            }

            bool CSimulatorMessagesSettings::isRelayCom2TextMessages() const
            {
                return this->getRelayedTextMessageTypes().testFlag(TextMessagesCom2);
            }

            bool CSimulatorMessagesSettings::relayThisStatusMessage(const CStatusMessage &message) const
            {
                if (message.isEmpty()) { return false; }
                if (!this->isGloballyEnabled()) { return false; }
                if (!this->isRelayTechnicalMessages()) { return false; }
                const int s = static_cast<int>(message.getSeverity());
                return (s >= m_technicalLogLevel);
            }

            bool CSimulatorMessagesSettings::relayThisTextMessage(const Network::CTextMessage &msg, const BlackMisc::Simulation::CSimulatedAircraft &aircraft) const
            {
                if (msg.isEmpty()) { return false; }
                if (!this->isGloballyEnabled()) { return false; }
                if (m_messageType == NoTextMessages) { return false; }

                const TextMessageType mt = static_cast<TextMessageType>(m_messageType);
                if (msg.isPrivateMessage() && mt.testFlag(TextMessagePrivate)) { return true; }
                if (msg.isSupervisorMessage() && (mt.testFlag(TextMessagePrivate) || mt.testFlag(TextMessageSupervisor))) { return true; }
                if (msg.isSendToUnicom() && mt.testFlag(TextMessagesUnicom)) { return true; }

                if (msg.isRadioMessage())
                {
                    const CFrequency f(msg.getFrequency());
                    if (mt.testFlag(TextMessagesCom1))
                    {
                        if (aircraft.getCom1System().isActiveFrequencyWithin8_33kHzChannel(f)) { return true; }
                    }
                    if (mt.testFlag(TextMessagesCom2))
                    {
                        if (aircraft.getCom2System().isActiveFrequencyWithin8_33kHzChannel(f)) { return true; }
                    }
                }
                return false;
            }

            CSimulatorMessagesSettings::TextMessageType CSimulatorMessagesSettings::getRelayedTextMessageTypes() const
            {
                return static_cast<CSimulatorMessagesSettings::TextMessageType>(m_messageType);
            }

            QString CSimulatorMessagesSettings::convertToQString(bool i18n) const
            {
                Q_UNUSED(i18n);
                static const QString s("Enabled %1, text messages: %2, severity: %3");
                QString severity;
                if (this->isRelayTechnicalMessages())
                {
                    severity = QStringLiteral("No tech. msgs");
                }
                else
                {
                    severity = CStatusMessage::severityToString(static_cast<CStatusMessage::StatusSeverity>(m_technicalLogLevel));
                }
                return s.arg(boolToOnOff(m_globallyEnabled)).arg(m_messageType).arg(severity);
            }

            CVariant CSimulatorMessagesSettings::propertyByIndex(const CPropertyIndex &index) const
            {
                if (index.isMyself()) { return CVariant::from(*this); }
                const ColumnIndex i = index.frontCasted<ColumnIndex>();
                switch (i)
                {
                case IndexTechnicalLogSeverity: return CVariant::fromValue(m_technicalLogLevel);
                case IndexTextMessageRelay: return CVariant::from(m_messageType);
                case IndexGloballyEnabled: return CVariant::from(m_globallyEnabled);
                default: return CValueObject::propertyByIndex(index);
                }
            }

            void CSimulatorMessagesSettings::setPropertyByIndex(const CPropertyIndex &index, const CVariant &variant)
            {
                if (index.isMyself()) { (*this) = variant.to<CSimulatorMessagesSettings>(); return; }
                const ColumnIndex i = index.frontCasted<ColumnIndex>();
                switch (i)
                {
                case IndexTechnicalLogSeverity: this->setTechnicalLogSeverity(static_cast<CStatusMessage::StatusSeverity>(variant.toInt())); break;
                case IndexTextMessageRelay: this->setRelayTextMessages(static_cast<CSimulatorMessagesSettings::TextMessageType>(variant.toInt())); break;
                case IndexGloballyEnabled: this->setGloballyEnabled(variant.toBool()); break;
                default: CValueObject::setPropertyByIndex(index, variant); break;
                }
            }

            void CSpecializedSimulatorSettings::setSimulatorDirectory(const QString &simDir)
            {
                m_genericSettings.setSimulatorDirectory(simDir);
            }

            const QString &CSpecializedSimulatorSettings::getDefaultSimulatorDirectory() const
            {
                return CSpecializedSimulatorSettings::defaultSimulatorDirectory(m_simulator);
            }

            const QString &CSpecializedSimulatorSettings::getSimulatorDirectoryIfNotDefault() const
            {
                static const QString empty;
                if (!m_genericSettings.hasSimulatorDirectory()) { return empty; }
                if (m_genericSettings.getSimulatorDirectory() == CSpecializedSimulatorSettings::defaultSimulatorDirectory(m_simulator))
                {
                    return empty;
                }
                return m_genericSettings.getSimulatorDirectory();
            }

            const QString &CSpecializedSimulatorSettings::getSimulatorDirectoryOrDefault() const
            {
                return (m_genericSettings.hasSimulatorDirectory()) ?
                       m_genericSettings.getSimulatorDirectory() :
                       CSpecializedSimulatorSettings::defaultSimulatorDirectory(m_simulator);
            }

            const QStringList CSpecializedSimulatorSettings::getModelDirectoriesOrDefault() const
            {
                return m_genericSettings.hasModelDirectories() ?
                       m_genericSettings.getModelDirectories() :
                       this->getModelDirectoriesFromSimulatorDirectoryOrDefault();
            }

            const QStringList CSpecializedSimulatorSettings::getModelDirectoriesFromSimulatorDirectoy() const
            {
                if (!m_genericSettings.hasSimulatorDirectory()) { return QStringList(); }
                const QString s(m_genericSettings.getSimulatorDirectory());
                switch (m_simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return QStringList({CFsCommonUtil::fs9AircraftDirFromSimDir(s)});
                case CSimulatorInfo::FSX: return QStringList({CFsCommonUtil::fsxSimObjectsDirFromSimDir(s)});
                case CSimulatorInfo::P3D: return QStringList({CFsCommonUtil::p3dSimObjectsDirFromSimDir(s)});
                case CSimulatorInfo::XPLANE: return QStringList({CXPlaneUtil::modelDirectoriesFromSimDir(s)});
                default: break;
                }
                return QStringList();
            }

            const QStringList CSpecializedSimulatorSettings::getModelDirectoriesFromSimulatorDirectoryOrDefault() const
            {
                if (!m_genericSettings.hasSimulatorDirectory()) { return CSpecializedSimulatorSettings::defaultModelDirectories(m_simulator); }
                return this->getModelDirectoriesFromSimulatorDirectoy();
            }

            const QStringList &CSpecializedSimulatorSettings::getModelDirectoriesIfNotDefault() const
            {
                static const QStringList empty;
                if (!m_genericSettings.hasModelDirectories()) { return empty; }
                if (m_genericSettings.getModelDirectories() == CSpecializedSimulatorSettings::defaultModelDirectories(m_simulator))
                {
                    return empty;
                }
                return m_genericSettings.getModelDirectories();
            }

            const QStringList &CSpecializedSimulatorSettings::getDefaultModelExcludeDirectoryPatterns() const
            {
                return CSpecializedSimulatorSettings::defaultModelExcludeDirectoryPatterns(m_simulator);
            }

            const QString CSpecializedSimulatorSettings::getFirstModelDirectoryOrDefault() const
            {
                static const QString empty;
                if (this->getModelDirectoriesOrDefault().isEmpty()) { return empty; }
                return this->getModelDirectoriesOrDefault().first();
            }

            const QStringList &CSpecializedSimulatorSettings::getDefaultModelDirectories() const
            {
                return CSpecializedSimulatorSettings::defaultModelDirectories(m_simulator);
            }

            const QStringList &CSpecializedSimulatorSettings::getModelExcludeDirectoryPatternsIfNotDefault() const
            {
                static const QStringList empty;
                if (!m_genericSettings.hasModelExcludeDirectoryPatterns()) { return empty; }
                if (m_genericSettings.getModelExcludeDirectoryPatterns() == CSpecializedSimulatorSettings::defaultModelExcludeDirectoryPatterns(m_simulator))
                {
                    return empty;
                }
                return m_genericSettings.getModelExcludeDirectoryPatterns();
            }

            const QStringList &CSpecializedSimulatorSettings::getModelExcludeDirectoryPatternsOrDefault() const
            {
                return m_genericSettings.hasModelExcludeDirectoryPatterns() ?
                       m_genericSettings.getModelExcludeDirectoryPatterns() :
                       CSpecializedSimulatorSettings::defaultModelExcludeDirectoryPatterns(m_simulator);
            }

            const QStringList &CSpecializedSimulatorSettings::defaultModelDirectories(const CSimulatorInfo &simulator)
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                static const QStringList e;
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9:
                    {
                        if (CFsCommonUtil::fs9AircraftDir().isEmpty()) { return e; }
                        static const QStringList md({ CFsCommonUtil::fs9AircraftDir() });
                        return  md;
                    }
                case CSimulatorInfo::FSX:
                    {
                        if (CFsCommonUtil::fsxSimObjectsDir().isEmpty()) { return e; }
                        static const QStringList md({ CFsCommonUtil::fsxSimObjectsDir() });
                        return  md;
                    }
                case CSimulatorInfo::P3D:
                    {
                        if (CFsCommonUtil::p3dSimObjectsDir().isEmpty()) { return e; }
                        static const QStringList md({ CFsCommonUtil::p3dSimObjectsDir() });
                        return  md;
                    }
                case CSimulatorInfo::XPLANE:
                    {
                        return CXPlaneUtil::xplaneModelDirectories();
                    }
                default:
                    Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                    break;
                }
                return e;
            }

            const QString &CSpecializedSimulatorSettings::defaultSimulatorDirectory(const CSimulatorInfo &simulator)
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return CFsCommonUtil::fs9Dir();
                case CSimulatorInfo::FSX: return CFsCommonUtil::fsxDir();
                case CSimulatorInfo::P3D: return CFsCommonUtil::p3dDir();
                case CSimulatorInfo::XPLANE: return CXPlaneUtil::xplaneRootDir();
                default:
                    Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                    break;
                }
                static const QString empty;
                return empty;
            }

            const QStringList &CSpecializedSimulatorSettings::defaultModelExcludeDirectoryPatterns(const CSimulatorInfo &simulator)
            {
                Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                switch (simulator.getSimulator())
                {
                case CSimulatorInfo::FS9: return CFsCommonUtil::fs9AircraftObjectsExcludeDirectoryPatterns();
                case CSimulatorInfo::FSX: return CFsCommonUtil::fsxSimObjectsExcludeDirectoryPatterns();
                case CSimulatorInfo::P3D: return CFsCommonUtil::p3dSimObjectsExcludeDirectoryPatterns();
                case CSimulatorInfo::XPLANE: return CXPlaneUtil::xplaneModelExcludeDirectoryPatterns();
                default:
                    Q_ASSERT_X(simulator.isSingleSimulator(), Q_FUNC_INFO, "No single simulator");
                    break;
                }
                static const QStringList empty;
                return empty;
            }

            const QString CXPlaneSimulatorSettings::getPluginDirOrDefault() const
            {
                return CFileUtils::appendFilePathsAndFixUnc(this->getSimulatorDirectoryOrDefault(), CXPlaneUtil::xplanePluginPathName());
            }
        } // ns
    } // ns
} // ns
