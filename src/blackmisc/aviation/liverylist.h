/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_AVIATION_LIVERYLIST_H
#define BLACKMISC_AVIATION_LIVERYLIST_H

#include "blackmisc/blackmiscexport.h"
#include "blackmisc/datastoreobjectlist.h"

#include "blackmisc/aviation/livery.h"

namespace BlackMisc
{
    namespace Aviation
    {
        //! Value object for a list of airports.
        class BLACKMISC_EXPORT CLiveryList :
            public CSequence<CLivery>,
            public BlackMisc::IDatastoreObjectListWithIntegerKey<CLivery, CLiveryList>,
            public BlackMisc::Mixin::MetaType<CLiveryList>
        {
        public:
            BLACKMISC_DECLARE_USING_MIXIN_METATYPE(CLiveryList)

            //! Default constructor.
            CLiveryList();

            //! Construct from a base class object.
            CLiveryList(const CSequence<CLivery> &other);

            //! Find 0..n liveries by combined code
            CLiveryList findByCombinedCode(const QString &combinedCode) const;

        };
    } //namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Aviation::CLiveryList)
Q_DECLARE_METATYPE(BlackMisc::CCollection<BlackMisc::Aviation::CLivery>)
Q_DECLARE_METATYPE(BlackMisc::CSequence<BlackMisc::Aviation::CLivery>)

#endif //guard
