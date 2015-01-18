/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of Swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "avcallsign.h"
#include "iconlist.h"

namespace BlackMisc
{
    namespace Aviation
    {
        /*
         * Convert to string
         */
        QString CCallsign::convertToQString(bool i18n) const
        {
            Q_UNUSED(i18n);
            return this->m_callsign;
        }

        /*
         * Unify the callsign
         */
        QString CCallsign::unifyCallsign(const QString &callsign)
        {
            QString unified = callsign.toUpper();
            // allow A-Z, 0-9, _, but no spaces
            unified = unified.remove(QRegExp("[^A-Z\\d_]"));
            return unified;
        }

        /*
         * Iconify
         */
        const CIcon &CCallsign::convertToIcon(const CCallsign &callsign)
        {
            if (callsign.hasSuffix())
            {
                QString t = callsign.getSuffix();
                if (t.length() < 3) { return CIconList::iconByIndex(CIcons::NetworkRoleUnknown); }
                if ("APP" == t) { return CIconList::iconByIndex(CIcons::NetworkRoleApproach); }
                if ("GND" == t) { return CIconList::iconByIndex(CIcons::NetworkRoleGround); }
                if ("TWR" == t) { return CIconList::iconByIndex(CIcons::NetworkRoleTower); }
                if ("DEL" == t) { return CIconList::iconByIndex(CIcons::NetworkRoleDelivery); }
                if ("CTR" == t) { return CIconList::iconByIndex(CIcons::NetworkRoleCenter); }
                if ("SUP" == t) { return CIconList::iconByIndex(CIcons::NetworkRoleSup); }
                if ("OBS" == t) { return CIconList::iconByIndex(CIcons::NetworkRoleObs); }
                if ("ATIS" == t) { return CIconList::iconByIndex(CIcons::AviationAtis); }
                return CIconList::iconByIndex(CIcons::NetworkRoleUnknown);
            }
            else
            {
                return CIconList::iconByIndex(CIcons::NetworkRolePilot);
            }
        }

        /*
         * ATC callsign?
         */
        bool CCallsign::isAtcCallsign() const
        {
            if (!this->hasSuffix()) { return false; }
            return atcCallsignSuffixes().contains(this->getSuffix(), Qt::CaseInsensitive);
        }

        /*
         * ATC callsign?
         */
        bool CCallsign::isAtcAlikeCallsign() const
        {
            if (!this->hasSuffix()) { return false; }
            return atcAlikeCallsignSuffixes().contains(this->getSuffix(), Qt::CaseInsensitive);
        }

        /*
         * Callsign as Observer
         */
        QString CCallsign::getAsObserverCallsignString() const
        {
            if (this->isEmpty()) { return ""; }
            QString obs = this->getStringAsSet();
            if (obs.endsWith("_OBS", Qt::CaseInsensitive)) { return obs; }
            if (obs.contains('_')) { obs = obs.left(obs.lastIndexOf('_')); }
            return obs.append("_OBS").toUpper();
        }

        /*
         * Suffix
         */
        QString CCallsign::getSuffix() const
        {
            QString s;
            if (this->hasSuffix())
            {
                s = this->getStringAsSet().section('_', -1).toUpper();
            }
            return s;
        }

        /*
         * Suffix?
         */
        bool CCallsign::hasSuffix() const
        {
            return this->getStringAsSet().contains('_');
        }

        /*
         * Equals callsign?
         */
        bool CCallsign::equalsString(const QString &callsignString) const
        {
            CCallsign other(callsignString);
            return other == (*this);
        }

        /*
         * Index
         */
        CVariant CCallsign::propertyByIndex(const CPropertyIndex &index) const
        {
            if (index.isMyself()) { return this->toCVariant(); }
            ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexCallsignString:
                return CVariant(this->asString());
            case IndexCallsignStringAsSet:
                return CVariant(this->getStringAsSet());
            case IndexTelephonyDesignator:
                return CVariant(this->getTelephonyDesignator());
            case IndexSuffix:
                return CVariant(this->getSuffix());
            default:
                return CValueObject::propertyByIndex(index);
            }
        }

        /*
         * Index
         */
        void CCallsign::setPropertyByIndex(const CVariant &variant, const CPropertyIndex &index)
        {
            if (index.isMyself())
            {
                this->convertFromCVariant(variant);
                return;
            }
            ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexCallsignString:
                this->m_callsign = variant.toQString();
                break;
            case IndexCallsignStringAsSet:
                this->m_callsignAsSet = variant.toQString();
                break;
            case IndexTelephonyDesignator:
                this->m_telephonyDesignator = variant.toQString();
                break;
            default:
                return CValueObject::setPropertyByIndex(variant, index);
            }
        }

        /*
         * Valid callsign?
         */
        bool CCallsign::isValidCallsign(const QString &callsign)
        {
            //! \todo sometimes callsigns such as 12345, really correct?
            // static QRegularExpression regexp("^[A-Z]+[A-Z0-9]*$");
            static QRegularExpression regexp("^[A-Z0-9]*$");
            if (callsign.length() < 2 || callsign.length() > 10) { return false; }
            return (regexp.match(callsign).hasMatch());
        }

        /*
         * Suffixes
         */
        const QStringList &CCallsign::atcCallsignSuffixes()
        {
            static const QStringList a( { "APP", "GND", "TWR", "DEL", "CTR" });
            return a;
        }

        const QStringList &CCallsign::atcAlikeCallsignSuffixes()
        {
            static const QStringList a( { "ATIS", "APP", "GND", "OBS", "TWR", "DEL", "CTR", "SUP", "FSS" });
            return a;
        }

    } // namespace
} // namespace
