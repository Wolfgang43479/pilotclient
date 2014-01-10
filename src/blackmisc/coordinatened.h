/*  Copyright (C) 2013 VATSIM Community / contributors
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BLACKMISC_COORDINATENED_H
#define BLACKMISC_COORDINATENED_H
#include "blackmisc/mathvector3d.h"
#include "blackmisc/mathmatrix3x3.h"
#include "blackmisc/coordinategeodetic.h"

namespace BlackMisc
{
    namespace Geo
    {
        /*!
         * \brief North, East, Down
         */
        class CCoordinateNed : public BlackMisc::Math::CVector3DBase<CCoordinateNed>
        {
        private:
            CCoordinateGeodetic m_referencePosition; //!< geodetic reference position
            bool m_hasReferencePosition; //!< valid reference position?

        protected:
            /*!
             * \brief String for converter
             * \return
             */
            virtual QString convertToQString() const
            {
                QString s = "NED: {N %1, E %2, D %3}";
                s = s.arg(QString::number(this->north(), 'f', 6)).
                    arg(QString::number(this->east(), 'f', 6)).
                    arg(QString::number(this->down(), 'f', 6));
                return s;
            }

        public:
            /*!
             * \brief Default constructor
             */
            CCoordinateNed() : CVector3DBase(), m_referencePosition(), m_hasReferencePosition(false) {}

            /*!
             * \brief Constructor with reference position
             * \param referencePosition
             */
            CCoordinateNed(const CCoordinateGeodetic &referencePosition) : CVector3DBase(), m_referencePosition(referencePosition), m_hasReferencePosition(true) {}

            /*!
             * \brief Constructor by values
             * \param referencePosition
             * \param north
             * \param east
             * \param down
             */
            CCoordinateNed(const CCoordinateGeodetic &referencePosition, double north, double east, double down) : CVector3DBase(north, east, down), m_referencePosition(referencePosition), m_hasReferencePosition(true) {}

            /*!
             * \brief Constructor by values
             * \param north
             * \param east
             * \param down
             */
            CCoordinateNed(double north, double east, double down) : CVector3DBase(north, east, down), m_referencePosition(), m_hasReferencePosition(false) {}

            /*!
             * \brief Copy constructor
             * \param otherNed
             */
            CCoordinateNed(const CCoordinateNed &other) :
                CVector3DBase(other), m_referencePosition(other.m_referencePosition), m_hasReferencePosition(other.m_hasReferencePosition) {}

            /*!
             * \brief Constructor by math vector
             * \param vector
             */
            explicit CCoordinateNed(const BlackMisc::Math::CVector3D &vector) : CVector3DBase(vector.i(), vector.j(), vector.k()), m_referencePosition(), m_hasReferencePosition(false) {}

            /*!
             * \brief Constructor by math vector and reference position
             * \param referencePosition
             * \param vector
             */
            CCoordinateNed(const CCoordinateGeodetic &referencePosition, const BlackMisc::Math::CVector3D &vector) : CVector3DBase(vector.i(), vector.j(), vector.k()), m_referencePosition(referencePosition), m_hasReferencePosition(true) {}

            /*!
             * \brief QVariant, required for DBus QVariant lists
             * \return
             */
            virtual QVariant toQVariant() const
            {
                return QVariant::fromValue(*this);
            }

            /*!
             * \brief Equal operator ==
             * \param other
             * \return
             */
            bool operator ==(const CCoordinateNed &other) const
            {
                if (this == &other) return true;
                return this->m_hasReferencePosition == other.m_hasReferencePosition &&
                       this->m_referencePosition == other.m_referencePosition &&
                       this->CVector3DBase::operator== (other);
            }

            /*!
             * \brief Unequal operator !=
             * \param other
             * \return
             */
            bool operator !=(const CCoordinateNed &other) const
            {
                return !((*this) == other);
            }

            /*!
             * \brief Corresponding reference position
             * \return
             */
            CCoordinateGeodetic referencePosition() const
            {
                return this->m_referencePosition;
            }

            /*!
             * \brief Corresponding reference position
             * \return
             */
            bool hasReferencePosition() const
            {
                return this->m_hasReferencePosition;
            }

            /*!
             * \brief North
             * \return
             */
            double north() const
            {
                return this->m_i;
            }

            /*!
             * \brief East
             * \return
             */
            double east() const
            {
                return this->m_j;
            }

            /*!
             * \brief Down
             * \return
             */
            double down() const
            {
                return this->m_k;
            }

            /*!
             * \brief Set north
             * \param north
             */
            void setNorth(double north)
            {
                this->m_i = north;
            }

            /*!
             * \brief Set east
             * \param east
             */
            void setEast(double east)
            {
                this->m_j = east;
            }

            /*!
             * \brief Set down
             * \param down
             */
            void setDown(double down)
            {
                this->m_k = down;
            }

            /*!
             * \brief Corresponding reference position
             * \param referencePosition
             */
            void setReferencePosition(const CCoordinateGeodetic &referencePosition)
            {
                this->m_referencePosition = referencePosition;
                this->m_hasReferencePosition = true;
            }

            /*!
             * \brief Concrete implementation of a 3D vector
             * \return
             */
            BlackMisc::Math::CVector3D toMathVector() const
            {
                return BlackMisc::Math::CVector3D(this->north(), this->east(), this->down());
            }
        };

    } // namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Geo::CCoordinateNed)

#endif // guard
