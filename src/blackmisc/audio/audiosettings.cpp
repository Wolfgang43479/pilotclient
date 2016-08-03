/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackmisc/audio/audiosettings.h"

#include <QChar>
#include <QtGlobal>

using namespace BlackMisc::Audio;

namespace BlackMisc
{
    namespace Audio
    {
        CSettings::CSettings()
        {
            this->initDefaultValues();
        }

        bool CSettings::getNotificationFlag(CNotificationSounds::Notification notification) const
        {
            const int i = static_cast<int>(notification);
            if (i >= m_notificationFlags.length()) return true; // default
            QChar f = m_notificationFlags.at(i);
            return '1' == f;
        }

        QString CSettings::convertToQString(bool i18n) const
        {
            Q_UNUSED(i18n);
            QString s("Notification flags:");
            s.append(" ").append(m_notificationFlags);
            return s;
        }

        void CSettings::initDefaultValues()
        {
            this->initNotificationFlags();
        }

        void CSettings::initNotificationFlags()
        {
            // if we add flags in the future, we automatically extend ...
            constexpr int l = 1 + static_cast<int>(CNotificationSounds::Notification::NotificationsLoadSounds);
            if (this->m_notificationFlags.length() < l)
            {
                int cl = m_notificationFlags.length();
                this->m_notificationFlags.append(QString(l - cl, '1'));
            }
        }

    } // namespace
} // namespace
