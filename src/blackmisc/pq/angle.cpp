/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackmisc/pq/angle.h"
#include "blackmisc/iconlist.h"
#include "blackmisc/icons.h"
#include "blackmisc/math/mathutils.h"

#include <cmath>

using namespace BlackMisc::Math;

namespace BlackMisc
{
    namespace PhysicalQuantities
    {
        CAngle::CAngle(int degrees, int minutes, double seconds) :
            CPhysicalQuantity(
                degrees + minutes / 100.0 + seconds / 10000.0,
                CAngleUnit::sexagesimalDeg())
        {
            Q_ASSERT_X((degrees >= 0 && minutes >= 0 && seconds >= 0) ||
                       (degrees <= 0 && minutes <= 0 && seconds <= 0), Q_FUNC_INFO, "Same sign required");
        }

        CAngle::CAngle(int degrees, double minutes) :
            CPhysicalQuantity(
                degrees + minutes / 100.0,
                CAngleUnit::sexagesimalDeg())
        {
            Q_ASSERT_X((degrees >= 0 && minutes >= 0) || (degrees <= 0 && minutes <= 0),
                       Q_FUNC_INFO, "Same sign required");
        }

        void CAngle::unifySign(int &degrees, int &minutes, double &seconds)
        {
            if (degrees < 0)
            {
                if (minutes > 0) minutes *= -1;
                if (seconds > 0) seconds *= -1.0;
            }
            else if (degrees > 0)
            {
                if (minutes < 0) minutes *= -1;
                if (seconds < 0) seconds *= -1.0;
            }
            else
            {
                // degrees was 0
                if ((minutes > 0 && seconds < 0) || (minutes < 0 && seconds > 0))
                {
                    seconds *= -1.0;
                }
            }
        }

        void CAngle::unifySign(int &degrees, int &minutes)
        {
            if ((degrees > 0 && minutes < 0) || (degrees < 0 && minutes > 0))
            {
                minutes *= -1.0;
            }
        }

        BlackMisc::CIcon CAngle::toIcon() const
        {
            BlackMisc::CIcon i = CIcon::iconByIndex(CIcons::StandardIconArrowMediumNorth16);
            i.setRotation(*this);
            return i;
        }

        CAngle::DegMinSecFractionalSec CAngle::asSexagesimalDegMinSec(bool range180Degrees) const
        {
            double dms = this->value(CAngleUnit::sexagesimalDeg());

            if (range180Degrees)
            {
                dms = std::fmod(dms + 180.0, 360.0);
                dms += (dms < 0) ? 180.0 : -180.0;
            }

            DegMinSecFractionalSec values;
            if (dms < 0)
            {
                values.sign = -1;
                dms *= -1.0;
            }

            values.deg = CMathUtils::trunc(dms);
            dms = CMathUtils::fract(dms) * 100;
            values.min = CMathUtils::trunc(dms);
            dms = CMathUtils::fract(dms) * 100;
            values.sec = CMathUtils::trunc(dms);
            dms = CMathUtils::fract(dms);
            values.fractionalSec = CMathUtils::round(dms, 6);
            return values;
        }

        double CAngle::piFactor() const
        {
            return BlackMisc::Math::CMathUtils::round(this->value(CAngleUnit::rad()) / BlackMisc::Math::CMathUtils::PI() , 6);
        }

        const double &CAngle::PI()
        {
            return BlackMisc::Math::CMathUtils::PI();
        }

        double CAngle::sin() const
        {
            return std::sin(this->value(CAngleUnit::rad()));
        }

        double CAngle::cos() const
        {
            return std::cos(this->value(CAngleUnit::rad()));
        }

        double CAngle::tan() const
        {
            return std::tan(this->value(CAngleUnit::rad()));
        }
    } // ns
} // ns
