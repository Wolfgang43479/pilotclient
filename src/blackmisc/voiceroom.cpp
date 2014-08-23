/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "voiceroom.h"
#include "blackmisc/blackmiscfreefunctions.h"
#include <QChar>
#include <QStringList>
#include <tuple>

namespace BlackMisc
{
    namespace Audio
    {
        CVoiceRoom::CVoiceRoom(const QString &serverUrl, bool connected) :
            m_connected(connected), m_audioPlaying(false)
        {
            if (serverUrl.contains("/"))
            {
                QString url = serverUrl.trimmed().toLower();
                url.replace(CVoiceRoom::protocolComplete(), "");
                url.replace(CVoiceRoom::protocol(), "");
                QStringList splitParts = serverUrl.split("/");
                m_hostname = splitParts.at(0);
                m_channel = splitParts.at(1);
            }
        }

        /*
         * Compare
         */
        int CVoiceRoom::compareImpl(const CValueObject &otherBase) const
        {
            const auto &other = static_cast<const CVoiceRoom &>(otherBase);
            return compare(TupleConverter<CVoiceRoom>::toTuple(*this), TupleConverter<CVoiceRoom>::toTuple(other));
        }

        /*
         * Marshall to DBus
         */
        void CVoiceRoom::marshallToDbus(QDBusArgument &argument) const
        {
            argument << TupleConverter<CVoiceRoom>::toTuple(*this);
        }

        /*
         * Unmarshall from DBus
         */
        void CVoiceRoom::unmarshallFromDbus(const QDBusArgument &argument)
        {
            argument >> TupleConverter<CVoiceRoom>::toTuple(*this);
        }

        /*
         * Hash
         */
        uint CVoiceRoom::getValueHash() const
        {
            return qHash(TupleConverter<CVoiceRoom>::toTuple(*this));
        }

        /*
         * Equal?
         */
        bool CVoiceRoom::operator ==(const CVoiceRoom &other) const
        {
            if (this == &other) return true;
            return TupleConverter<CVoiceRoom>::toTuple(*this) == TupleConverter<CVoiceRoom>::toTuple(other);
        }

        /*
         * Unequal?
         */
        bool CVoiceRoom::operator !=(const CVoiceRoom &other) const
        {
            return !((*this) == other);
        }

        /*
         * Metadata
         */
        void CVoiceRoom::registerMetadata()
        {
            qRegisterMetaType<CVoiceRoom>();
            qDBusRegisterMetaType<CVoiceRoom>();
        }

        /*
         * To JSON
         */
        QJsonObject CVoiceRoom::toJson() const
        {
            return BlackMisc::serializeJson(CVoiceRoom::jsonMembers(), TupleConverter<CVoiceRoom>::toTuple(*this));
        }

        /*
         * To JSON
         */
        void CVoiceRoom::fromJson(const QJsonObject &json)
        {
            BlackMisc::deserializeJson(json, CVoiceRoom::jsonMembers(), TupleConverter<CVoiceRoom>::toTuple(*this));
        }

        /*
         * Members
         */
        const QStringList &CVoiceRoom::jsonMembers()
        {
            return TupleConverter<CVoiceRoom>::jsonMembers();
        }


        /*
         * To string
         */
        QString CVoiceRoom::convertToQString(bool /* i18n */) const
        {
            if (!this->isValid()) return "Invalid";
            QString s = this->getVoiceRoomUrl(false);
            s.append(this ->isConnected() ? " connected" : " unconnected");
            if (this->m_audioPlaying) s.append(" playing");
            return s;
        }

        /*
         * metaTypeId
         */
        int CVoiceRoom::getMetaTypeId() const
        {
            return qMetaTypeId<CVoiceRoom>();
        }

        /*
         * is a
         */
        bool CVoiceRoom::isA(int metaTypeId) const
        {
            if (metaTypeId == qMetaTypeId<CVoiceRoom>()) { return true; }

            return CValueObject::isA(metaTypeId);
        }

        /*
         * Server URL
         */
        QString CVoiceRoom::getVoiceRoomUrl(bool noProtocol) const
        {
            if (!this->isValid()) return "";
            QString url(noProtocol ? "" : CVoiceRoom::protocolComplete());
            url.append(this->m_hostname);
            url.append("/");
            url.append(this->m_channel);
            return url;
        }

        /*
         * ATIS voice channel
         */
        bool CVoiceRoom::isAtis() const
        {
            return (this->m_channel.contains("ATIS", Qt::CaseInsensitive));
        }
    } // Voice
} // BlackMisc
