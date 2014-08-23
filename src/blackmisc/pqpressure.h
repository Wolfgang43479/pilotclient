/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_PQPRESSURE_H
#define BLACKMISC_PQPRESSURE_H

#include "pqphysicalquantity.h"

namespace BlackMisc
{
    namespace PhysicalQuantities
    {

        /*!
         * Physical unit distance
         */
        class CPressure : public CPhysicalQuantity<CPressureUnit, CPressure>
        {
        public:
            //! Default constructor
            CPressure() : CPhysicalQuantity(0, CPressureUnit::defaultUnit()) {}

            //! Init by double value
            CPressure(double value, const CPressureUnit &unit) : CPhysicalQuantity(value, unit) {}

            //! \copydoc CPhysicalQuantity(const QString &unitString)
            CPressure(const QString &unitString) : CPhysicalQuantity(unitString) {}

            //! \copydoc CValueObject::toQVariant
            virtual QVariant toQVariant() const override
            {
                return QVariant::fromValue(*this);
            }

            //! Virtual destructor
            virtual ~CPressure() {}
        };

    } // namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::PhysicalQuantities::CPressure)

#endif // BLACKMISC_PQPRESSURE_H
