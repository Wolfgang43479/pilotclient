/* Copyright (C) 2014
 * swift project community / contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#ifndef BLACKSIMPLUGIN_FS9_MULTIPLAYER_PACKETS_H
#define BLACKSIMPLUGIN_FS9_MULTIPLAYER_PACKETS_H

#include "fs9.h"
#include <QString>
#include <QtGlobal>
#include <tuple>

//! \file

namespace BlackSimPlugin
{
    namespace Fs9
    {
        //! Multiplayer packet - change player plane
        struct MPChangePlayerPlane : public MULTIPLAYER_PACKET_CHANGE_PLAYER_PLANE
        {
            //! Return tuple of member variables
            std::tuple<CFs9Sdk::EngineType &, QString &> getTuple()
            {
                return std::tie(engine, aircraft_name);
            }

            //! Return const tuple of member variables
            std::tuple<const CFs9Sdk::EngineType &, const QString &> getTuple() const
            {
                return std::tie(engine, aircraft_name);
            }
        };

        //! Multiplayer packet - chat text
        struct MPChatText : public MULTIPLAYER_PACKET_CHAT_TEXT
        {
            //! Return tuple of member variables
            std::tuple<QString &> getTuple()
            {
                return std::tie(chat_data);
            }

            //! Return const tuple of member variables
            std::tuple<const QString &> getTuple() const
            {
                return std::tie(chat_data);
            }
        };

        //! Multiplayer packet - position and velocity
        struct MPPositionVelocity : public MULTIPLAYER_PACKET_POSITION_VELOCITY
        {
            //! Return tuple of member variables
            std::tuple<quint32 &, quint32 &, qint32 &, qint32 &, qint32 &,
                quint32 &, quint8 &, quint8 &, quint8 &,
                quint8 &, qint32 &, qint32 &, qint32 &,
                qint32 &, quint16 &, quint16 &,
                quint16 &> getTuple()
            {
                return std::tie(packet_index, application_time, lat_velocity,
                                lon_velocity, alt_velocity, ground_velocity,
                                reserved[0], reserved[1], reserved[2], reserved[3],
                                pbh, lat_i, lon_hi, alt_i, lat_f, lon_lo, alt_f);
            }

            //! Return const tuple of member variables
            std::tuple<const quint32 &, const quint32 &, const qint32 &, const qint32 &, const qint32 &,
            const quint32 &, const quint8 &, const quint8 &, const quint8 &,
            const quint8 &, const qint32 &, const qint32 &, const qint32 &,
            const qint32 &, const quint16 &, const quint16 &,
            const quint16 &> getTuple() const
            {
                return std::tie(packet_index, application_time, lat_velocity,
                                lon_velocity, alt_velocity, ground_velocity,
                                reserved[0], reserved[1], reserved[2], reserved[3],
                                pbh, lat_i, lon_hi, alt_i, lat_f, lon_lo, alt_f);
            }
        };
    }
}

#endif // BLACKSIMPLUGIN_FS9_MULTIPLAYER_PACKETS_H
