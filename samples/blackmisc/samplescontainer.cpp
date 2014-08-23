/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of Swift Project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "samplescontainer.h"
#include "blackmisc/blackmiscfreefunctions.h"
#include "blackmisc/avatcstationlist.h"
#include <QDebug>
#include <QMetaType>

using namespace BlackMisc::Aviation;
using namespace BlackMisc::PhysicalQuantities;
using namespace BlackMisc::Geo;
using namespace BlackMisc::Network;

namespace BlackMiscTest
{

    /*
     * Samples
     */
    int CSamplesContainer::samples()
    {
        // ATC stations
        QDateTime dtFrom = QDateTime::currentDateTimeUtc();
        QDateTime dtUntil = dtFrom.addSecs(60 * 60.0); // 1 hour
        QDateTime dtFrom2 = dtUntil;
        QDateTime dtUntil2 = dtUntil.addSecs(60 * 60.0);
        CCoordinateGeodetic geoPos =
            CCoordinateGeodetic::fromWgs84("48° 21′ 13″ N", "11° 47′ 09″ E", CLength(1487, CLengthUnit::ft()));
        CAtcStation station1(CCallsign("eddm_twr"), CUser("123456", "Joe Doe"),
                             CFrequency(118.7, CFrequencyUnit::MHz()),
                             geoPos, CLength(50, CLengthUnit::km()), false, dtFrom, dtUntil);
        CAtcStation station2(station1);
        CAtcStation station3(CCallsign("eddm_app"), CUser("654321", "Jen Doe"),
                             CFrequency(120.7, CFrequencyUnit::MHz()),
                             geoPos, CLength(100, CLengthUnit::km()), false, dtFrom2, dtUntil2);

        // ATC List
        CAtcStationList atcList;
        atcList.push_back(station1);
        atcList.push_back(station2);
        atcList.push_back(station3);
        atcList.push_back(station1);
        atcList.push_back(station2);
        atcList.push_back(station3);
        qDebug() << "-- list:";
        qDebug() << atcList.toQString();

        CAtcStationList atcListFind = atcList.findBy(&CAtcStation::getCallsign, "eddm_twr", &CAtcStation::getFrequency, CFrequency(118.7, CFrequencyUnit::MHz()));
        qDebug() << "-- find by:";
        qDebug() << atcListFind.toQString();

        CAtcStationList atcListSort = atcList.sortedBy(&CAtcStation::getBookedFromUtc, &CAtcStation::getCallsign, &CAtcStation::getControllerRealName);
        qDebug() << "-- sort by:";
        qDebug() << atcListSort.toQString();

        qDebug() << "-----------------------------------------------";
        return 0;
    }

} // namespace
