/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackmiscfreefunctions.h"
#include "avheading.h"

using BlackMisc::PhysicalQuantities::CAngle;
using BlackMisc::PhysicalQuantities::CAngleUnit;

namespace BlackMisc
{
    namespace Aviation
    {

        /*
         * Own implementation for streaming
         */
        QString CHeading::convertToQString(bool i18n) const
        {
            QString s = CAngle::convertToQString(i18n).append(" ");
            if (i18n)
            {
                return s.append(this->isMagneticHeading() ?
                                QCoreApplication::translate("Aviation", "magnetic") :
                                QCoreApplication::translate("Aviation", "true"));
            }
            else
            {
                return s.append(this->isMagneticHeading() ? "magnetic" : "true");
            }
        }

        /*
         * Marshall
         */
        void CHeading::marshallToDbus(QDBusArgument &argument) const
        {
            CAngle::marshallToDbus(argument);
            argument << TupleConverter<CHeading>::toTuple(*this);
        }

        /*
         * Unmarshall
         */
        void CHeading::unmarshallFromDbus(const QDBusArgument &argument)
        {
            CAngle::unmarshallFromDbus(argument);
            argument >> TupleConverter<CHeading>::toTuple(*this);
        }

        /*
         * Hash
         */
        uint CHeading::getValueHash() const
        {
            QList<uint> hashs;
            hashs << CAngle::getValueHash();
            hashs << qHash(TupleConverter<CHeading>::toTuple(*this));
            return BlackMisc::calculateHash(hashs, "CHeading");
        }

        /*
         * Compare
         */
        int CHeading::compareImpl(const CValueObject &otherBase) const
        {
            const auto &other = static_cast<const CHeading &>(otherBase);
            int result = compare(TupleConverter<CHeading>::toTuple(*this), TupleConverter<CHeading>::toTuple(other));
            return result == 0 ? CAngle::compareImpl(otherBase) : result;
        }

        bool CHeading::operator ==(const CHeading &other) const
        {
            if (this == &other) return true;
            if (!CAngle::operator ==(other)) return false;
            return TupleConverter<CHeading>::toTuple(*this) == TupleConverter<CHeading>::toTuple(other);
        }

        /*
         * Unequal?
         */
        bool CHeading::operator !=(const CHeading &other) const
        {
            return !((*this) == other);
        }

        /*!
         * \brief Register metadata of unit and quantity
         */
        void CHeading::registerMetadata()
        {
            qRegisterMetaType<CHeading>();
            qDBusRegisterMetaType<CHeading>();
        }

        /*
         * To JSON
         */
        QJsonObject CHeading::toJson() const
        {
            QJsonObject json = BlackMisc::serializeJson(CHeading::jsonMembers(), TupleConverter<CHeading>::toTuple(*this));
            return BlackMisc::Json::appendJsonObject(json, CAngle::toJson());
        }

        /*
         * To JSON
         */
        void CHeading::fromJson(const QJsonObject &json)
        {
            CAngle::fromJson(json);
            BlackMisc::deserializeJson(json, CHeading::jsonMembers(), TupleConverter<CHeading>::toTuple(*this));
        }

        /*
         * Members
         */
        const QStringList &CHeading::jsonMembers()
        {
            return TupleConverter<CHeading>::jsonMembers();
        }


    } // namespace
} // namespace
