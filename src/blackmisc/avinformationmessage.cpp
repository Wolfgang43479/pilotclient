/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "avinformationmessage.h"
#include "blackmiscfreefunctions.h"

namespace BlackMisc
{
    namespace Aviation
    {
        /*
         * Convert to string
         */
        QString CInformationMessage::convertToQString(bool /** i18n **/) const
        {
            return this->m_message;
        }

        /*
         * metaTypeId
         */
        int CInformationMessage::getMetaTypeId() const
        {
            return qMetaTypeId<CInformationMessage>();
        }

        /*
         * is a
         */
        bool CInformationMessage::isA(int metaTypeId) const
        {
            if (metaTypeId == qMetaTypeId<CInformationMessage>()) { return true; }

            return this->CValueObject::isA(metaTypeId);
        }

        /*
         * Compare
         */
        int CInformationMessage::compareImpl(const CValueObject &otherBase) const
        {
            const auto &other = static_cast<const CInformationMessage &>(otherBase);

            return compare(TupleConverter<CInformationMessage>::toTuple(*this), TupleConverter<CInformationMessage>::toTuple(other));
        }

        /*
         * Marshall to DBus
         */
        void CInformationMessage::marshallToDbus(QDBusArgument &argument) const
        {
            argument << TupleConverter<CInformationMessage>::toTuple(*this);
        }

        /*
         * Unmarshall from DBus
         */
        void CInformationMessage::unmarshallFromDbus(const QDBusArgument &argument)
        {
            argument >> TupleConverter<CInformationMessage>::toTuple(*this);
        }

        /*
         * Equal?
         */
        bool CInformationMessage::operator ==(const CInformationMessage &other) const
        {
            if (this == &other) return true;
            return TupleConverter<CInformationMessage>::toTuple(*this) == TupleConverter<CInformationMessage>::toTuple(other);
        }

        /*
         * Unequal?
         */
        bool CInformationMessage::operator !=(const CInformationMessage &other) const
        {
            return !((*this) == other);
        }

        /*
         * Hash
         */
        uint CInformationMessage::getValueHash() const
        {
            return qHash(TupleConverter<CInformationMessage>::toTuple(*this));
        }

        /*
         * Register metadata
         */
        void CInformationMessage::registerMetadata()
        {
            qRegisterMetaType<CInformationMessage>();
            qDBusRegisterMetaType<CInformationMessage>();
        }

        /*
         * JSON members
         */
        const QStringList &CInformationMessage::jsonMembers()
        {
            return TupleConverter<CInformationMessage>::jsonMembers();
        }

        /*
         * To JSON
         */
        QJsonObject CInformationMessage::toJson() const
        {
            return BlackMisc::serializeJson(CInformationMessage::jsonMembers(), TupleConverter<CInformationMessage>::toTuple(*this));
        }

        /*
         * To JSON
         */
        void CInformationMessage::fromJson(const QJsonObject &json)
        {
            this->m_message = json.value("m_message").toString();
            this->m_type = static_cast<CInformationMessage::InformationType>(qRound(json.value("m_type").toDouble()));
            this->m_receivedTimestamp = QDateTime::fromString(json.value("m_receivedTimestamp").toString());
        }

        /*
         * Type as string
         */
        const QString &CInformationMessage::getTypeAsString() const
        {
            switch (this->m_type)
            {
            case ATIS:
                {
                    static const QString atis("ATIS");
                    return atis;
                }
            case METAR:
                {
                    static const QString metar("METAR");
                    return metar;
                }
            case TAF:
                {
                    static const QString taf("TAF");
                    return taf;
                }
            default:
                {
                    static const QString ds("unknown");
                    return ds;
                }
            }
        }
    } // namespace
} // namespace
