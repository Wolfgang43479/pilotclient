/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "registermetadataaudio.h"
#include "audio.h"

#include <QDBusMetaType>

namespace BlackMisc
{
    namespace Audio
    {
        //! Register metadata for audio / voice
        void registerMetadata()
        {
            CAudioDeviceInfo::registerMetadata();
            CAudioDeviceInfoList::registerMetadata();
            CVoiceRoom::registerMetadata();
            CVoiceRoomList::registerMetadata();
            CSettings::registerMetadata();
            qDBusRegisterMetaType<BlackMisc::Audio::CNotificationSounds::PlayMode>();
            qDBusRegisterMetaType<BlackMisc::Audio::CNotificationSounds::Notification>();
        }
    } // ns
} // ns
