/* Copyright (C) 2018
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "notificationsounds.h"
#include <QStringList>
#include <QString>

namespace BlackMisc
{
    namespace Audio
    {
        const QString &CNotificationSounds::flagToString(CNotificationSounds::NotificationFlag notification)
        {
            static const QString unknown("unknown");
            static const QString error("error");
            static const QString login("login");
            static const QString logoff("logoff");
            static const QString privateMsg("private msg.");
            static const QString mentioned("cs mentioned");
            static const QString joined("room joined");
            static const QString left("room left");
            static const QString ptt("PTT click");
            static const QString load("load sounds");

            switch (notification)
            {
            case NotificationError: return error;
            case NotificationLogin: return login;
            case NotificationLogoff: return logoff;
            case NotificationTextMessagePrivate: return privateMsg;
            case NotificationTextCallsignMentioned: return mentioned;
            case NotificationVoiceRoomJoined: return joined;
            case NotificationVoiceRoomLeft: return left;
            case PTTClick: return ptt;
            case LoadSounds: return load;
            default: break;
            }
            return unknown;
        }

        const QString CNotificationSounds::toString(Notification notification)
        {
            QStringList n;
            if (notification.testFlag(NotificationError))  n << flagToString(NotificationError);
            if (notification.testFlag(NotificationLogin))  n << flagToString(NotificationLogin);
            if (notification.testFlag(NotificationLogoff)) n << flagToString(NotificationLogoff);
            if (notification.testFlag(NotificationTextMessagePrivate))    n << flagToString(NotificationTextMessagePrivate);
            if (notification.testFlag(NotificationTextCallsignMentioned)) n << flagToString(NotificationTextCallsignMentioned);
            if (notification.testFlag(NotificationVoiceRoomJoined)) n << flagToString(NotificationVoiceRoomJoined);
            if (notification.testFlag(NotificationVoiceRoomLeft))   n << flagToString(NotificationVoiceRoomLeft);
            if (notification.testFlag(LoadSounds)) n << flagToString(LoadSounds);
            return n.join(", ");
        }
    } // ns
} // ns