/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "airlineicaocodelist.h"

namespace BlackMisc
{
    namespace Aviation
    {
        CAirlineIcaoCodeList::CAirlineIcaoCodeList(const CSequence<CAirlineIcaoCode> &other) :
            CSequence<CAirlineIcaoCode>(other)
        { }

        CAirlineIcaoCodeList CAirlineIcaoCodeList::findByDesignator(const QString &designator)
        {
            if (CAirlineIcaoCode::isValidAirlineDesignator(designator)) { return CAirlineIcaoCodeList(); }
            return this->findBy([&](const CAirlineIcaoCode & code)
            {
                return code.matchesDesignator(designator);
            });
        }

        CAirlineIcaoCode CAirlineIcaoCodeList::findByVDesignator(const QString &designator)
        {
            if (CAirlineIcaoCode::isValidAirlineDesignator(designator)) { return CAirlineIcaoCode(); }
            return this->findFirstBy([&](const CAirlineIcaoCode & code)
            {
                return code.matchesVDesignator(designator);
            });
        }

        CAirlineIcaoCodeList CAirlineIcaoCodeList::fromDatabaseJson(const QJsonArray &array,  bool ignoreIncomplete)
        {
            CAirlineIcaoCodeList codes;
            for (const QJsonValue &value : array)
            {
                CAirlineIcaoCode icao(CAirlineIcaoCode::fromDatabaseJson(value.toObject()));
                if (ignoreIncomplete && !icao.hasCompleteData()) { continue; }
                codes.push_back(icao);
            }
            return codes;
        }

        QStringList CAirlineIcaoCodeList::toCompleterStrings() const
        {
            QStringList c;
            for (const CAirlineIcaoCode &icao : *this)
            {
                QString cs(icao.getCombinedStringWithKey());
                c.append(cs);
            }
            return c;
        }

    } // namespace
} // namespace
