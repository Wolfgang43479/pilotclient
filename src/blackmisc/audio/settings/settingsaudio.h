/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_AUDIO_SETTINGS_AUDIO_H
#define BLACKMISC_AUDIO_SETTINGS_AUDIO_H

#include "blackmisc/blackmiscexport.h"
#include "blackmisc/valueobject.h"
#include "blackmisc/statusmessagelist.h"
#include "blackmisc/audio/notificationsounds.h"
#include <QJsonObject>

namespace BlackMisc
{
    namespace Audio
    {
        namespace Settings
        {
            //! Value object encapsulating information of audio related settings.
            class BLACKMISC_EXPORT CSettingsAudio : public CValueObject<CSettingsAudio>
            {
            public:
                //! Default constructor.
                CSettingsAudio();

                //! Notification flag (play notification?)
                bool getNotificationFlag(BlackMisc::Audio::CNotificationSounds::Notification notification) const;

                //! Notification flag (play notification?)
                void setNotificationFlag(BlackMisc::Audio::CNotificationSounds::Notification notification, bool value);

                //! Settings value
                BlackMisc::CStatusMessage value(const QString &path, const QString &command, const BlackMisc::CVariant &value, bool &changedFlag);

                //! Init with meaningful default values
                void initDefaultValues();

                //! \copydoc BlackMisc::Mixin::String::toQString
                QString convertToQString(bool i18n = false) const;

            private:
                QString m_notificationFlags;  //!< play notification for notification x, a little trick to use a string here (streamable, hashable, ..)
                void initNotificationFlags(); //!< init flags

                BLACK_METACLASS(CSettingsAudio,
                    BLACK_METAMEMBER(notificationFlags)
                );
            };

        } // namespace
    } // namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Audio::Settings::CSettingsAudio)

#endif // guard
