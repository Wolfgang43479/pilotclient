/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "runtimebasedcomponent.h"
#include <QWidget>

namespace BlackGui
{
    namespace Components
    {
        void CRuntimeBasedComponent::setRuntime(BlackCore::CRuntime *runtime, bool runtimeOwner)
        {
            Q_ASSERT(runtime);
            this->m_runtime = runtime;
            this->m_runtimeOwner = runtimeOwner;
            this->runtimeHasBeenSet();
        }

        void CRuntimeBasedComponent::setRuntimeForComponents(BlackCore::CRuntime *runtime, QWidget *parent)
        {
            if (!parent) return;

            // tested for runtime, not too slow, only some ms
            QList<QWidget *> children = parent->findChildren<QWidget *>();
            foreach(QWidget * widget, children)
            {
                if (widget->objectName().isEmpty()) continue; // rule out unamed widgets
                CRuntimeBasedComponent *rbc = dynamic_cast<CRuntimeBasedComponent *>(widget);
                if (rbc) rbc->setRuntime(runtime, false);
            }
        }

        void CRuntimeBasedComponent::createRuntime(const BlackCore::CRuntimeConfig &config, QObject *parent)
        {
            this->m_runtime = new BlackCore::CRuntime(config, parent);
            this->m_runtimeOwner = true;
        }

        void CRuntimeBasedComponent::sendStatusMessage(const BlackMisc::CStatusMessage &statusMessage)
        {
            if (!this->getIContextApplication()) return;
            this->getIContextApplication()->sendStatusMessage(statusMessage);
        }

        void CRuntimeBasedComponent::sendStatusMessages(const BlackMisc::CStatusMessageList &statusMessages)
        {
            if (!this->getIContextApplication()) return;
            this->getIContextApplication()->sendStatusMessages(statusMessages);
        }

        const BlackCore::IContextApplication *CRuntimeBasedComponent::getIContextApplication() const
        {
            if (!this->m_runtime) return nullptr;
            return this->m_runtime->getIContextApplication();
        }

        BlackCore::IContextApplication *CRuntimeBasedComponent::getIContextApplication()
        {
            if (!this->m_runtime) return nullptr;
            return this->m_runtime->getIContextApplication();
        }

        BlackCore::IContextAudio *CRuntimeBasedComponent::getIContextAudio()
        {
            if (!this->m_runtime) return nullptr;
            return this->m_runtime->getIContextAudio();
        }

        const BlackCore::IContextAudio *CRuntimeBasedComponent::getIContextAudio() const
        {
            if (!this->m_runtime) return nullptr;
            return this->m_runtime->getIContextAudio();
        }

        BlackCore::IContextNetwork *CRuntimeBasedComponent::getIContextNetwork()
        {
            if (!this->m_runtime) return nullptr;
            return this->m_runtime->getIContextNetwork();
        }

        const BlackCore::IContextNetwork *CRuntimeBasedComponent::getIContextNetwork() const
        {
            if (!this->m_runtime) return nullptr;
            return this->m_runtime->getIContextNetwork();
        }

        BlackCore::IContextOwnAircraft *CRuntimeBasedComponent::getIContextOwnAircraft()
        {
            if (!this->m_runtime) return nullptr;
            return this->m_runtime->getIContextOwnAircraft();
        }

        const BlackCore::IContextOwnAircraft *CRuntimeBasedComponent::getIContextOwnAircraft() const
        {
            if (!this->m_runtime) return nullptr;
            return this->m_runtime->getIContextOwnAircraft();
        }

        BlackCore::IContextSettings *CRuntimeBasedComponent::getIContextSettings()
        {
            if (!this->m_runtime) return nullptr;
            return this->m_runtime->getIContextSettings();
        }

        const BlackCore::IContextSettings *CRuntimeBasedComponent::getIContextSettings() const
        {
            if (!this->m_runtime) return nullptr;
            return this->m_runtime->getIContextSettings();
        }

        const BlackCore::IContextSimulator *CRuntimeBasedComponent::getIContextSimulator() const
        {
            if (!this->m_runtime) return nullptr;
            return this->m_runtime->getIContextSimulator();
        }

        BlackCore::IContextSimulator *CRuntimeBasedComponent::getIContextSimulator()
        {
            if (!this->m_runtime) return nullptr;
            return this->m_runtime->getIContextSimulator();
        }

        void CRuntimeBasedComponent::playNotifcationSound(BlackSound::CNotificationSounds::Notification notification) const
        {
            if (!this->getIContextAudio()) return;
            this->getIContextAudio()->playNotification(static_cast<uint>(notification), true);
        }
    }
}
