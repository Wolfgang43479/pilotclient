/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "nwuser.h"
#include "blackmisc/avairporticao.h"
#include "blackmisc/icon.h"
#include "blackmisc/blackmiscfreefunctions.h"
#include "blackmisc/propertyindex.h"
#include "blackmisc/variant.h"
#include <tuple>
#include <QRegExp>

using namespace BlackMisc::Aviation;

namespace BlackMisc
{
    namespace Network
    {
        CUser::CUser(const QString &id, const QString &realname, const CCallsign &callsign)
            : m_id(id.trimmed()), m_realname(realname), m_callsign(callsign)
        {
            this->setRealName(realname); // extracts homebase
        }

        CUser::CUser(const QString &id, const QString &realname, const QString &email, const QString &password, const CCallsign &callsign)
            : m_id(id.trimmed()), m_realname(realname), m_email(email), m_password(password), m_callsign(callsign)
        {
            this->setRealName(realname); // extracts homebase
        }

        QString CUser::convertToQString(bool i18n) const
        {
            Q_UNUSED(i18n);
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

        void CUser::setRealName(const QString &realname)
        {
            // try to strip homebase
            // I understand the limitations, but we will have more correct hits as failures I assume
            const QRegularExpression reg("(-\\s*|\\s)([A-Z]{4})$");
            QString rn = realname.trimmed().simplified();
            if (rn.isEmpty())
            {
                this->m_realname = "";
                return;
            }

            if (!this->hasValidHomebase())
            {
                //! only apply stripping if home base is not explicitly given
                QRegularExpressionMatch match = reg.match(rn);
                if (match.hasMatch())
                {
                    int pos = match.capturedStart(0);
                    QString icao = match.captured(0).trimmed().right(4);
                    rn = rn.left(pos).trimmed();
                    this->setHomebase(CAirportIcao(icao));
                }
            }
            this->m_realname = rn;
        }

        CStatusMessageList CUser::validate() const
        {
            CStatusMessageList msgs;
            // callsign optional
            if (!this->hasValidId()) { msgs.push_back(CStatusMessage(CStatusMessage::SeverityWarning, "Invalid id"));}
            if (!this->hasValidRealName()) { msgs.push_back(CStatusMessage(CStatusMessage::SeverityWarning, "Invalid real name"));}
            if (!this->hasValidCredentials()) { msgs.push_back(CStatusMessage(CStatusMessage::SeverityWarning, "Invalid credentials"));}
            return msgs;
        }

        /*
         * Exchange data
         */
        void CUser::syncronizeData(CUser &otherUser)
        {
            if (otherUser == (*this)) { return; }

            if (this->hasValidRealName())
            {
                otherUser.setRealName(this->getRealName());
            }
            else if (otherUser.hasValidRealName())
            {
                this->setRealName(otherUser.getRealName());
            }

            if (this->hasValidId())
            {
                otherUser.setId(this->getId());
            }
            else if (otherUser.hasValidId())
            {
                this->setId(otherUser.getId());
            }

            if (this->hasValidEmail())
            {
                otherUser.setEmail(this->getEmail());
            }
            else if (otherUser.hasValidEmail())
            {
                this->setEmail(otherUser.getEmail());
            }

            if (this->hasValidCallsign())
            {
                otherUser.setCallsign(this->getCallsign());
            }
            else if (otherUser.hasValidCallsign())
            {
                this->setCallsign(otherUser.getCallsign());
            }
        }

        bool CUser::isValidVatsimId(const QString &id)
        {
            if (id.isEmpty()) { return false; }
            bool ok;
            int i = id.toInt(&ok);
            if (!ok) { return false; }
            return i >= 100000 && i <= 9999999;
        }

        /*
         * Property by index
         */
        CVariant CUser::propertyByIndex(const BlackMisc::CPropertyIndex &index) const
        {
            if (index.isMyself()) { return this->toCVariant(); }
            ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexEmail:
                return CVariant(this->m_email);
            case IndexId:
                return CVariant(this->m_id);
            case IndexPassword:
                return CVariant(this->m_password);
            case IndexRealName:
                return CVariant(this->m_realname);
            case IndexHomebase:
                return this->m_homebase.propertyByIndex(index.copyFrontRemoved());
            case IndexCallsign:
                return this->m_callsign.propertyByIndex(index.copyFrontRemoved());
            default:
                return CValueObject::propertyByIndex(index);
            }
        }

        /*
         * Set property as index
         */
        void CUser::setPropertyByIndex(const CVariant &variant, const BlackMisc::CPropertyIndex &index)
        {
            if (index.isMyself())
            {
                this->convertFromCVariant(variant);
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
            case IndexHomebase:
                this->m_homebase.setPropertyByIndex(variant, index.copyFrontRemoved());
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
