/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_USER_H
#define BLACKMISC_USER_H

#include "avcallsign.h"
#include "avairporticao.h"
#include "propertyindex.h"

namespace BlackMisc
{

    namespace Network
    {

        /*!
         * Value object encapsulating information of a user.
         */
        class CUser : public BlackMisc::CValueObject
        {
        public:

            /*!
             * Properties by index
             */
            enum ColumnIndex
            {
                IndexEmail = BlackMisc::CPropertyIndex::GlobalIndexCUser,
                IndexId,
                IndexPassword,
                IndexRealName,
                IndexCallsign
            };

            //! Default constructor.
            CUser() {}

            //! Constructor by callsign
            CUser(const BlackMisc::Aviation::CCallsign &callsign) : m_callsign(callsign) {}

            //! Constructor.
            CUser(const QString &id, const QString &realname, const BlackMisc::Aviation::CCallsign &callsign)
                : m_id(id), m_realname(realname), m_callsign(callsign)
            {}

            //! Constructor.
            CUser(const QString &id, const QString &realname, const QString &email = "", const QString &password = "", const BlackMisc::Aviation::CCallsign &callsign = BlackMisc::Aviation::CCallsign())
                : m_id(id), m_realname(realname), m_email(email), m_password(password), m_callsign(callsign)
            {}

            //! \copydoc CValueObject::toQVariant()
            virtual QVariant toQVariant() const override
            {
                return QVariant::fromValue(*this);
            }

            //! Get full name.
            QString getRealName() const { return m_realname; }

            //! setRealName
            void setRealName(const QString &realname) { m_realname = realname.trimmed(); }

            //! Get password
            QString getPassword() const { return m_password; }

            //! Set password
            void setPassword(const QString &pw) { m_password = pw.trimmed(); }

            //! Valid user object?
            bool isValid() const { return !this->m_realname.isEmpty() &&  !this->m_id.isEmpty(); }

            //! Valid credentials?
            bool hasValidCredentials() const { return this->isValid() && !this->m_password.isEmpty(); }

            //! Valid real name?
            bool hasValidRealName() const { return !this->m_realname.isEmpty(); }

            //! Valid id?
            bool hasValidId() const { return !this->m_id.isEmpty(); }

            //! Has associated callsign?
            bool hasValidCallsign() const { return !m_callsign.isEmpty(); }

            //! Get email.
            QString getEmail() const { return m_email; }

            //! Set email.
            void setEmail(const QString &email) { m_email = email.trimmed(); }

            //! Valid email?
            bool hasValidEmail() const { return !this->m_email.isEmpty(); }

            //! Get id.
            QString getId() const { return m_id; }

            //! Set id
            void setId(const QString &id) { m_id = id.trimmed(); }

            //! Homebase
            const BlackMisc::Aviation::CAirportIcao &getHomebase() const { return this->m_homebase; }

            //! Set homebase
            void setHomebase(const BlackMisc::Aviation::CAirportIcao &homebase) { this->m_homebase = homebase; }

            //! Get associated callsign.
            const BlackMisc::Aviation::CCallsign &getCallsign() const { return m_callsign; }

            //! Set associated callsign
            void setCallsign(const BlackMisc::Aviation::CCallsign &callsign) { m_callsign = callsign; }

            //! Equal operator ==
            bool operator ==(const CUser &other) const;

            //! Unequal operator !=
            bool operator !=(const CUser &other) const;

            //! \copydoc CValueObject::getValueHash()
            virtual uint getValueHash() const override;

            //! \copydoc CValueObject::toIcon()
            virtual BlackMisc::CIcon toIcon() const override { return this->getCallsign().toIcon(); }

            //! \copydoc CValueObject::toJson
            virtual QJsonObject toJson() const override;

            //! \copydoc CValueObject::fromJson
            virtual void fromJson(const QJsonObject &json) override;

            //! \copydoc CValueObject::propertyByIndex(int)
            virtual QVariant propertyByIndex(const BlackMisc::CPropertyIndex &index) const override;

            //! \copydoc CValueObject::setPropertyByIndex(const QVariant, int)
            virtual void setPropertyByIndex(const QVariant &variant, const BlackMisc::CPropertyIndex &index) override;

            /*!
             * This and another user exchange missing data.
             * This user has priority and overrides first.
             * \param otherUser
             */
            void syncronizeData(CUser &otherUser);

            //! Register metadata
            static void registerMetadata();

            //! Members
            static const QStringList &jsonMembers();

        protected:
            //! \copydoc CValueObject::convertToQString
            virtual QString convertToQString(bool i18n = false) const override;

            //! \copydoc CValueObject::getMetaTypeId
            virtual int getMetaTypeId() const override;

            //! \copydoc CValueObject::isA
            virtual bool isA(int metaTypeId) const override;

            //! \copydoc CValueObject::compareImpl
            virtual int compareImpl(const CValueObject &other) const override;

            //! \copydoc CValueObject::marshallToDbus
            virtual void marshallToDbus(QDBusArgument &argument) const override;

            //! \copydoc CValueObject::unmarshallFromDbus
            virtual void unmarshallFromDbus(const QDBusArgument &argument) override;

        private:
            BLACK_ENABLE_TUPLE_CONVERSION(CUser)
            QString m_id;
            QString m_realname;
            QString m_email;
            QString m_password;
            BlackMisc::Aviation::CCallsign m_callsign;
            BlackMisc::Aviation::CAirportIcao m_homebase;
        };

    } // namespace

} // namespace

BLACK_DECLARE_TUPLE_CONVERSION(BlackMisc::Network::CUser, (o.m_id, o.m_realname, o.m_email, o.m_password, o.m_callsign, o.m_homebase))
Q_DECLARE_METATYPE(BlackMisc::Network::CUser)

#endif // guard
