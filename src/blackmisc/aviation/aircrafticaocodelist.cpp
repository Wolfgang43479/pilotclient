/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "aircrafticaocodelist.h"

namespace BlackMisc
{
    namespace Aviation
    {

        CAircraftIcaoCodeList::CAircraftIcaoCodeList(const CSequence<CAircraftIcaoCode> &other) :
            CSequence<CAircraftIcaoCode>(other)
        { }

        CAircraftIcaoCodeList CAircraftIcaoCodeList::fromDatabaseJson(const QJsonArray &array, bool ignoreIncomplete)
        {
            CAircraftIcaoCodeList codes;
            for (const QJsonValue &value : array)
            {
                CAircraftIcaoCode icao(CAircraftIcaoCode::fromDatabaseJson(value.toObject()));
                if (ignoreIncomplete && !icao.hasCompleteData()) { continue; }
                codes.push_back(icao);
            }
            return codes;
        }

    } // namespace
} // namespace
