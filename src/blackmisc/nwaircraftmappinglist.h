/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_AIRCRAFTMAPPINGLIST_H
#define BLACKMISC_AIRCRAFTMAPPINGLIST_H

#include "nwaircraftmapping.h"
#include "collection.h"
#include "sequence.h"
#include <QObject>
#include <QString>
#include <QList>

namespace BlackMisc
{
    namespace Network
    {
        /*!
         * Value object encapsulating a list of aircraft mappings
         */
        class CAircraftMappingList : public CSequence<CAircraftMapping>
        {
        public:
            //! Empty constructor.
            CAircraftMappingList();

            //! Construct from a base class object.
            CAircraftMappingList(const CSequence<CAircraftMapping> &other);

            //! QVariant, required for DBus QVariant lists
            virtual QVariant toQVariant() const override { return QVariant::fromValue(*this); }

            //! Find by ICAO code, empty fields treated as wildcards
            CAircraftMappingList findByIcaoCodeWildcard(const BlackMisc::Aviation::CAircraftIcao &searchIcao) const;

            //! Find by ICAO code, empty fields treated literally
            CAircraftMappingList findByIcaoCodeExact(const BlackMisc::Aviation::CAircraftIcao &searchIcao) const;

            //! Find by model string
            CAircraftMappingList findByModelString(const QString modelString, Qt::CaseSensitivity sensitivity) const;

            //! Register metadata
            static void registerMetadata();

        };

    } //namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Network::CAircraftMappingList)
Q_DECLARE_METATYPE(BlackMisc::CCollection<BlackMisc::Network::CAircraftMapping>)
Q_DECLARE_METATYPE(BlackMisc::CSequence<BlackMisc::Network::CAircraftMapping>)

#endif //guard
