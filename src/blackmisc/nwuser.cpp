/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "nwuser.h"
#include "blackmisc/icon.h"
#include "blackmisc/blackmiscfreefunctions.h"
#include "blackmisc/propertyindex.h"
#include <tuple>

namespace BlackMisc
{
    namespace Network
    {
        /*
         * Convert to string
         */
        QString CUser::convertToQString(bool /** i18n **/) const
        {
            if (this->m_realname.isEmpty()) return "<no realname>";
            QString s = this->m_realname;
            if (this->hasValidId())
            {
                s.append(" (").append(this->m_id).append(')');
            }
            if (this->hasValidCallsign())
            {
                s.append(' ').append(this->getCallsign().getStringAsSet());
            }
            return s;
        }

        /*
         * metaTypeId
         */
        int CUser::getMetaTypeId() const
        {
            return qMetaTypeId<CUser>();
        }

        /*
         * is a
         */
        bool CUser::isA(int metaTypeId) const
        {
            if (metaTypeId == qMetaTypeId<CUser>()) { return true; }
            return this->CValueObject::isA(metaTypeId);
        }

        /*
         * Compare
         */
        int CUser::compareImpl(const CValueObject &otherBase) const
        {
            const auto &other = static_cast<const CUser &>(otherBase);
            return compare(TupleConverter<CUser>::toTuple(*this), TupleConverter<CUser>::toTuple(other));
        }

        /*
         * Marshall to DBus
         */
        void CUser::marshallToDbus(QDBusArgument &argument) const
        {
            argument << TupleConverter<CUser>::toTuple(*this);
        }

        /*
         * Unmarshall from DBus
         */
        void CUser::unmarshallFromDbus(const QDBusArgument &argument)
        {
            argument >> TupleConverter<CUser>::toTuple(*this);
        }

        /*
         * Equal?
         */
        bool CUser::operator ==(const CUser &other) const
        {
            if (this == &other) return true;
            return TupleConverter<CUser>::toTuple(*this) == TupleConverter<CUser>::toTuple(other);
        }

        /*
         * Unequal?
         */
        bool CUser::operator !=(const CUser &other) const
        {
            return !((*this) == other);
        }

        /*
         * Exchange data
         */
        void CUser::syncronizeData(CUser &otherUser)
        {
            if (otherUser == (*this)) return;

            if (this->hasValidRealName())
                otherUser.setRealName(this->getRealName());
            else if (otherUser.hasValidRealName())
                this->setRealName(otherUser.getRealName());

            if (this->hasValidId())
                otherUser.setId(this->getId());
            else if (otherUser.hasValidId())
                this->setId(otherUser.getId());

            if (this->hasValidEmail())
                otherUser.setEmail(this->getEmail());
            else if (otherUser.hasValidEmail())
                this->setEmail(otherUser.getEmail());

            if (this->hasValidCallsign())
                otherUser.setCallsign(this->getCallsign());
            else if (otherUser.hasValidCallsign())
                this->setCallsign(otherUser.getCallsign());
        }

        /*
         * Hash
         */
        uint CUser::getValueHash() const
        {
            return qHash(TupleConverter<CUser>::toTuple(*this));
        }

        /*
         * To JSON
         */
        QJsonObject CUser::toJson() const
        {
            return BlackMisc::serializeJson(CUser::jsonMembers(), TupleConverter<CUser>::toTuple(*this));
        }

        /*
         * From JSON
         */
        void CUser::fromJson(const QJsonObject &json)
        {
            BlackMisc::deserializeJson(json, CUser::jsonMembers(), TupleConverter<CUser>::toTuple(*this));
        }

        /*
         * Members
         */
        const QStringList &CUser::jsonMembers()
        {
            return TupleConverter<CUser>::jsonMembers();
        }

        /*
         * Register metadata
         */
        void CUser::registerMetadata()
        {
            qRegisterMetaType<CUser>();
            qDBusRegisterMetaType<CUser>();
        }

        /*
         * Property by index
         */
        QVariant CUser::propertyByIndex(const BlackMisc::CPropertyIndex &index) const
        {
            if (index.isMyself()) { return this->toQVariant(); }
            ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexEmail:
                return QVariant(this->m_email);
            case IndexId:
                return QVariant(this->m_id);
            case IndexPassword:
                return QVariant(this->m_password);
            case IndexRealName:
                return QVariant(this->m_realname);
            case IndexCallsign:
                return this->m_callsign.propertyByIndex(index.copyFrontRemoved());
            default:
                return CValueObject::propertyByIndex(index);
            }
        }

        /*
         * Set property as index
         */
        void CUser::setPropertyByIndex(const QVariant &variant, const BlackMisc::CPropertyIndex &index)
        {
            if (index.isMyself())
            {
                this->fromQVariant(variant);
                return;
            }
            ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexEmail:
                this->setEmail(variant.value<QString>());
                break;
            case IndexId:
                this->setId(variant.value<QString>());
                break;
            case IndexPassword:
                this->setPassword(variant.value<QString>());
                break;
            case IndexRealName:
                this->setRealName(variant.value<QString>());
                break;
            case IndexCallsign:
                this->m_callsign.setPropertyByIndex(variant, index.copyFrontRemoved());
                break;
            default:
                CValueObject::setPropertyByIndex(variant, index);
                break;
            }
        }
    } // namespace
} // namespace
