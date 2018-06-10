/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_AVIATION_AIRCRAFTSITUATION_H
#define BLACKMISC_AVIATION_AIRCRAFTSITUATION_H

#include "blackmisc/aviation/altitude.h"
#include "blackmisc/aviation/callsign.h"
#include "blackmisc/aviation/heading.h"
#include "blackmisc/blackmiscexport.h"
#include "blackmisc/geo/elevationplane.h"
#include "blackmisc/geo/coordinategeodetic.h"
#include "blackmisc/geo/latitude.h"
#include "blackmisc/geo/longitude.h"
#include "blackmisc/metaclass.h"
#include "blackmisc/pq/angle.h"
#include "blackmisc/pq/length.h"
#include "blackmisc/pq/speed.h"
#include "blackmisc/pq/time.h"
#include "blackmisc/propertyindex.h"
#include "blackmisc/timestampbased.h"
#include "blackmisc/valueobject.h"
#include "blackmisc/variant.h"

#include <QMetaType>
#include <QString>
#include <QVector3D>
#include <array>

namespace BlackMisc
{
    namespace Geo { class CElevationPlane; }
    namespace Simulation { class CAircraftModel; }
    namespace Aviation
    {
        class CAircraftParts;
        class CAircraftPartsList;
        class CAircraftSituationChange;

        //! Value object encapsulating information of an aircraft's situation
        class BLACKMISC_EXPORT CAircraftSituation :
            public CValueObject<CAircraftSituation>,
            public Geo::ICoordinateGeodetic, public ITimestampWithOffsetBased
        {
        public:
            //! Properties by index
            enum ColumnIndex
            {
                IndexPosition = CPropertyIndex::GlobalIndexCAircraftSituation,
                IndexLatitude,
                IndexLongitude,
                IndexAltitude,
                IndexHeading,
                IndexBank,
                IndexIsOnGround,
                IndexIsOnGroundString,
                IndexOnGroundReliability,
                IndexOnGroundReliabilityString,
                IndexPitch,
                IndexGroundSpeed,
                IndexGroundElevationPlane,
                IndexGroundElevationInfo,
                IndexGroundElevationInfoString,
                IndexGroundElevationPlusInfo,
                IndexCallsign,
                IndexCG,
                IndexCanLikelySkipNearGroundInterpolation
            };

            //! Is on ground?
            enum IsOnGround
            {
                NotOnGround,
                OnGround,
                OnGroundSituationUnknown
            };

            //! Reliability of on ground information
            enum OnGroundDetails
            {
                NotSetGroundDetails,
                // interpolated situation
                OnGroundByInterpolation,  //!< strongest for remote aircraft
                OnGroundByElevationAndCG,
                OnGroundByElevation,
                OnGroundByGuessing,       //!< weakest
                // received situation
                InFromNetwork,            //!< received from network
                InFromParts,              //!< set from aircraft parts
                InNoGroundInfo,           //!< not know
                // send information
                OutOnGroundOwnAircraft    //!< sending on ground
            };

            //! How was altitude corrected?
            enum AltitudeCorrection
            {
                NoCorrection,
                Underflow,       //!< aircraft too low
                DraggedToGround, //!< other scenery too high, but on ground
                AGL,
                NoElevation,     //!< no correction as there is no elevation
                UnknownCorrection
            };

            //! Where did we get elevation from?
            enum GndElevationInfo
            {
                NoElevationInfo,
                TransferredElevation, //!< transferred from nearby situation
                FromProvider,         //!< from BlackMisc::Simulation::ISimulationEnvironmentProvider
                FromCache,            //!< from cache
                SituationChange,      //!< from BlackMisc::Aviation::CAircraftSituationChange
                Test                  //!< unit test
            };

            //! Default constructor.
            CAircraftSituation();

            //! Constructor with callsign
            CAircraftSituation(const CCallsign &correspondingCallsign);

            //! Comprehensive constructor
            CAircraftSituation(const Geo::CCoordinateGeodetic &position,
                               const CHeading &heading = {},
                               const PhysicalQuantities::CAngle &pitch = {},
                               const PhysicalQuantities::CAngle &bank = {},
                               const PhysicalQuantities::CSpeed &gs = {},
                               const Geo::CElevationPlane &groundElevation = {});

            //! Comprehensive constructor
            CAircraftSituation(const CCallsign &correspondingCallsign,
                               const Geo::CCoordinateGeodetic &position,
                               const CHeading &heading = {},
                               const PhysicalQuantities::CAngle &pitch = {},
                               const PhysicalQuantities::CAngle &bank = {},
                               const PhysicalQuantities::CSpeed &gs = {},
                               const Geo::CElevationPlane &groundElevation = {});

            //! \copydoc Mixin::String::toQString
            QString convertToQString(bool i18n = false) const;

            //! \copydoc Mixin::Index::propertyByIndex
            CVariant propertyByIndex(const CPropertyIndex &index) const;

            //! \copydoc Mixin::Index::setPropertyByIndex
            void setPropertyByIndex(const CPropertyIndex &index, const CVariant &variant);

            //! \copydoc Mixin::Index::comparePropertyByIndex
            int comparePropertyByIndex(const CPropertyIndex &index, const CAircraftSituation &compareValue) const;

            //! Get position
            const Geo::CCoordinateGeodetic &getPosition() const { return m_position; }

            //! Position null?
            bool isPositionNull() const { return m_position.isNull(); }

            //! Position or altitude null?
            bool isPositionOrAltitudeNull() const { return this->isPositionNull() || this->getAltitude().isNull(); }

            //! Null situation
            virtual bool isNull() const override;

            //! Equal pitch, bank heading
            //! \sa Geo::ICoordinateGeodetic::equalNormalVectorDouble
            bool equalPbh(const CAircraftSituation &other) const;

            //! Equal PBH and vector
            //! \sa Geo::ICoordinateGeodetic::equalNormalVectorDouble
            bool equalPbhAndVector(const CAircraftSituation &other) const;

            //! Set to null
            void setNull();

            //! Set position
            void setPosition(const Geo::CCoordinateGeodetic &position) { m_position = position; }

            //! \copydoc Geo::ICoordinateGeodetic::latitude()
            virtual Geo::CLatitude latitude() const override { return m_position.latitude(); }

            //! \copydoc Geo::ICoordinateGeodetic::longitude()
            virtual Geo::CLongitude longitude() const override { return m_position.longitude(); }

            //! On ground?
            IsOnGround getOnGround() const { return static_cast<CAircraftSituation::IsOnGround>(m_onGround); }

            //! Is on ground?
            bool isOnGround() const { return this->getOnGround() == OnGround; }

            //! On ground by parts?
            bool isOnGroundFromParts() const;

            //! On ground by network flag?
            bool isOnGroundFromNetwork() const;

            //! On ground?
            const QString &onGroundAsString() const;

            //! On ground info available?
            bool isOnGroundInfoAvailable() const;

            //! Set on ground
            bool setOnGround(bool onGround);

            //! Set on ground
            bool setOnGround(CAircraftSituation::IsOnGround onGround);

            //! Set on ground
            bool setOnGround(CAircraftSituation::IsOnGround onGround, CAircraftSituation::OnGroundDetails details);

            //! On ground factor 0..1 (on ground), -1 not set
            double getOnGroundFactor() const { return m_onGroundFactor; }

            //! Set on ground factor 0..1 (on ground), -1 not set
            void setOnGroundFactor(double groundFactor);

            //! Should we guess on ground?
            bool shouldGuessOnGround() const;

            //! Guess on ground flag
            bool guessOnGround(const CAircraftSituationChange &change, const Simulation::CAircraftModel &model);

            //! Distance to ground, null if impossible to calculate
            PhysicalQuantities::CLength getGroundDistance(const PhysicalQuantities::CLength &centerOfGravity) const;

            //! On ground reliability
            OnGroundDetails getOnGroundDetails() const { return static_cast<CAircraftSituation::OnGroundDetails>(m_onGroundDetails); }

            //! Do the ground details permit ground interpolation?
            bool hasGroundDetailsForGndInterpolation() const;

            //! On ground reliability as string
            const QString &getOnDetailsAsString() const;

            //! On ground details
            bool setOnGroundDetails(CAircraftSituation::OnGroundDetails details);

            //! Set on ground as interpolated from ground fatcor
            bool setOnGroundFromGroundFactorFromInterpolation(double threshold = 0.5);

            //! Set on ground by underflow detection, detects below ground scenarios
            bool setOnGroundByUnderflowDetection(const PhysicalQuantities::CLength &cg);

            //! On ground info as string
            QString getOnGroundInfo() const;

            //! \copydoc Geo::ICoordinateGeodetic::geodeticHeight
            const CAltitude &geodeticHeight() const override { return m_position.geodeticHeight(); }

            //! \copydoc Geo::ICoordinateGeodetic::normalVector
            virtual QVector3D normalVector() const override { return m_position.normalVector(); }

            //! \copydoc Geo::ICoordinateGeodetic::normalVectorDouble
            virtual std::array<double, 3> normalVectorDouble() const override { return m_position.normalVectorDouble(); }

            //! Elevation of the ground directly beneath
            const CAltitude &getGroundElevation() const { return m_groundElevationPlane.getAltitude(); }

            //! Elevation of the ground directly beneath
            const Geo::CElevationPlane &getGroundElevationPlane() const { return m_groundElevationPlane; }

            //! How did we get gnd.elevation?
            GndElevationInfo getGroundElevationInfo() const;

            //! How did we get gnd.elevation?
            const QString &getGroundElevationInfoAsString() const { return gndElevationInfoToString(this->getGroundElevationInfo()); }

            //! Ground elevation plus info
            QString getGroundElevationAndInfo() const;

            //! How we did get gnd.elevation
            void setGroundElevationInfo(GndElevationInfo details) { m_elvInfo = static_cast<int>(details); }

            //! Can the elevation be transferred to another situation?
            bool canTransferGroundElevation(const CAircraftSituation &otherSituation, const PhysicalQuantities::CLength &radius = Geo::CElevationPlane::singlePointRadius()) const;

            //! Transfer from "this" situation to \c otherSituation
            bool transferGroundElevation(CAircraftSituation &otherSituation, const PhysicalQuantities::CLength &radius = Geo::CElevationPlane::singlePointRadius()) const;

            //! Is on ground by elevation data, requires elevation and CG
            //! @{
            IsOnGround isOnGroundByElevation() const;
            IsOnGround isOnGroundByElevation(const PhysicalQuantities::CLength &cg) const;
            //! @}

            //! Is ground elevation value available
            bool hasGroundElevation() const;

            //! Has inbound ground details
            bool hasInboundGroundDetails() const;

            //! Elevation of the ground directly beneath at the given situation
            void setGroundElevation(const Aviation::CAltitude &altitude, GndElevationInfo info);

            //! Elevation of the ground directly beneath
            void setGroundElevation(const Geo::CElevationPlane &elevationPlane, GndElevationInfo info);

            //! Set elevation of the ground directly beneath, but checked
            //! \remark override if better
            bool setGroundElevationChecked(const Geo::CElevationPlane &elevationPlane, GndElevationInfo info);

            //! Reset ground elevation
            void resetGroundElevation();

            //! Distance of ground elevation
            const PhysicalQuantities::CLength &getGroundElevationRadius() const;

            //! Height above ground.
            PhysicalQuantities::CLength getHeightAboveGround() const;

            //! Get heading
            const CHeading &getHeading() const { return m_heading; }

            //! Set heading
            void setHeading(const CHeading &heading) { m_heading = heading; }

            //! Get altitude
            const CAltitude &getAltitude() const { return m_position.geodeticHeight(); }

            //! Get altitude unit
            const PhysicalQuantities::CLengthUnit &getAltitudeUnit() const { return m_position.geodeticHeight().getUnit(); }

            //! Get altitude unit
            const PhysicalQuantities::CLengthUnit &getAltitudeOrDefaultUnit() const;

            //! Get altitude under consideration of ground elevation and ground flag
            //! \remark with dragToGround it will also compensate overflows, otherwise only underflow
            //! @{
            CAltitude getCorrectedAltitude(bool enableDragToGround = true, AltitudeCorrection *correction = nullptr) const;
            CAltitude getCorrectedAltitude(const PhysicalQuantities::CLength &centerOfGravity, bool enableDragToGround = true, AltitudeCorrection *correction = nullptr) const;
            //! @}

            //! Set the corrected altitude from CAircraftSituation::getCorrectedAltitude
            //! @{
            AltitudeCorrection correctAltitude(bool enableDragToGround = true);
            AltitudeCorrection correctAltitude(const PhysicalQuantities::CLength &centerOfGravity = PhysicalQuantities::CLength::null(), bool enableDragToGround = true);
            //! @}

            //! Set altitude
            void setAltitude(const CAltitude &altitude);

            //! Add offset to altitude
            CAltitude addAltitudeOffset(const PhysicalQuantities::CLength &offset);

            //! Get pressure altitude
            const CAltitude &getPressureAltitude() const { return m_pressureAltitude; }

            //! Set pressure altitude
            void setPressureAltitude(const CAltitude &altitude);

            //! Get pitch
            const PhysicalQuantities::CAngle &getPitch() const { return m_pitch; }

            //! Set pitch
            void setPitch(const PhysicalQuantities::CAngle &pitch) { m_pitch = pitch; }

            //! Get bank (angle)
            const PhysicalQuantities::CAngle &getBank() const { return m_bank; }

            //! Set bank (angle)
            void setBank(const PhysicalQuantities::CAngle &bank) { m_bank = bank; }

            //! Get ground speed
            const PhysicalQuantities::CSpeed &getGroundSpeed() const { return m_groundSpeed; }

            //! Set ground speed
            void setGroundSpeed(const PhysicalQuantities::CSpeed &groundspeed) { m_groundSpeed = groundspeed; }

            //! Is moving? Means ground speed > epsilon
            bool isMoving() const;

            //! Situation looks like an aircraft not near ground
            bool canLikelySkipNearGroundInterpolation() const;

            //! Distance per time
            PhysicalQuantities::CLength getDistancePerTime(const PhysicalQuantities::CTime &time) const;

            //! Distance per milliseconds
            PhysicalQuantities::CLength getDistancePerTime(int milliseconds) const;

            //! Corresponding callsign
            const CCallsign &getCallsign() const { return m_correspondingCallsign; }

            //! Has corresponding callsign
            bool hasCallsign() const { return !this->getCallsign().isEmpty(); }

            //! Corresponding callsign
            void setCallsign(const CCallsign &callsign);

            //! Get CG if any
            const PhysicalQuantities::CLength &getCG() const { return m_cg; }

            //! Set CG
            void setCG(const PhysicalQuantities::CLength &cg);

            //! Has CG set?
            bool hasCG() const { return !m_cg.isNull(); }

            //! Set flag indicating this is an interim position update
            void setInterimFlag(bool flag) { m_isInterim = flag; }

            //! Transfer ground flag from parts
            //! \param parts containing the gnd flag
            //! \param alwaysSetDetails mark as CAircraftSituation::InFromParts regardless of parts
            //! \param timeDeviationFactor 0..1 (of offset time) small deviations from time are accepted
            //! \param differenceMs returns time difference
            bool adjustGroundFlag(const CAircraftParts &parts, bool alwaysSetDetails, double timeDeviationFactor = 0.1, qint64 *differenceMs = nullptr);

            //! Transfer ground flag from parts list
            //! \param partsList containing the gnd flag
            //! \param alwaysSetDetails mark as CAircraftSituation::InFromParts regardless of parts
            //! \param timeDeviationFactor 0..1 (of offset time) small deviations from time are accepted
            //! \param differenceMs returns time difference
            bool adjustGroundFlag(const CAircraftPartsList &partsList, bool alwaysSetDetails, double timeDeviationFactor = 0.1, qint64 *differenceMs = nullptr);

            //! Get flag indicating this is an interim position update
            bool isInterim() const { return m_isInterim; }

            //! Enum to string
            static const QString &isOnGroundToString(IsOnGround onGround);

            //! Enum to string
            static const QString &onGroundDetailsToString(OnGroundDetails reliability);

            //! Enum to string
            static const QString &altitudeCorrectionToString(AltitudeCorrection correction);

            //! Enum to string
            static const QString &gndElevationInfoToString(GndElevationInfo details);

            //! Delta distance, near to ground
            static const PhysicalQuantities::CLength &deltaNearGround();

            //! Null situation
            static const CAircraftSituation &null();

            //! A default CG if not other value is available
            static const PhysicalQuantities::CLength &defaultCG();

            //! Both on ground
            static bool isGfEqualOnGround(double oldGroundFactor, double newGroundFactor)
            {
                return isDoubleEpsilonEqual(1.0, oldGroundFactor) && isDoubleEpsilonEqual(1.0, newGroundFactor);
            }

            //! Ground flag comparisons @{
            //! Both not on ground
            static bool isGfEqualAirborne(double oldGroundFactor, double newGroundFactor)
            {
                return isDoubleEpsilonEqual(0.0, oldGroundFactor) && isDoubleEpsilonEqual(0.0, newGroundFactor);
            }

            //! Aircraft is starting
            static bool isGfStarting(double oldGroundFactor, double newGroundFactor)
            {
                return isDoubleEpsilonEqual(0.0, oldGroundFactor) && isDoubleEpsilonEqual(1.0, newGroundFactor);
            }

            //! Aircraft is landing
            static bool isGfLanding(double oldGroundFactor, double newGroundFactor)
            {
                return isDoubleEpsilonEqual(1.0, oldGroundFactor) && isDoubleEpsilonEqual(0.0, newGroundFactor);
            }
            //! @}

        private:
            CCallsign m_correspondingCallsign;
            Geo::CCoordinateGeodetic m_position; //!< NULL position as default
            Aviation::CAltitude m_pressureAltitude { 0, nullptr };
            CHeading m_heading { 0, nullptr };
            PhysicalQuantities::CAngle m_pitch { 0, nullptr };
            PhysicalQuantities::CAngle m_bank  { 0, nullptr };
            PhysicalQuantities::CSpeed m_groundSpeed { 0, nullptr };
            PhysicalQuantities::CLength m_cg { 0, nullptr };
            Geo::CElevationPlane m_groundElevationPlane; //!< NULL elevation as default
            bool m_isInterim = false;
            int m_onGround = static_cast<int>(CAircraftSituation::OnGroundSituationUnknown);
            int m_onGroundDetails = static_cast<int>(CAircraftSituation::NotSetGroundDetails);
            int m_elvInfo = static_cast<int>(CAircraftSituation::NoElevationInfo); //!< where did we gnd.elevation from?
            double m_onGroundFactor = -1; //!< interpolated ground flag, 1..on ground, 0..not on ground, -1 no info
            QString m_onGroundGuessingDetails; //!< only for debugging, not transferred via DBus etc.

            //! Equal double values?
            static bool isDoubleEpsilonEqual(double d1, double d2)
            {
                return qAbs(d1 - d2) <= std::numeric_limits<double>::epsilon();
            }

            BLACK_METACLASS(
                CAircraftSituation,
                BLACK_METAMEMBER(correspondingCallsign),
                BLACK_METAMEMBER(position),
                BLACK_METAMEMBER(pressureAltitude),
                BLACK_METAMEMBER(heading),
                BLACK_METAMEMBER(pitch),
                BLACK_METAMEMBER(bank),
                BLACK_METAMEMBER(groundSpeed),
                BLACK_METAMEMBER(cg),
                BLACK_METAMEMBER(groundElevationPlane),
                BLACK_METAMEMBER(onGround),
                BLACK_METAMEMBER(onGroundDetails),
                BLACK_METAMEMBER(elvInfo),
                BLACK_METAMEMBER(onGroundFactor),
                BLACK_METAMEMBER(timestampMSecsSinceEpoch),
                BLACK_METAMEMBER(timeOffsetMs),
                BLACK_METAMEMBER(isInterim)
            );
        };
    } // namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Aviation::CAircraftSituation)
Q_DECLARE_METATYPE(BlackMisc::Aviation::CAircraftSituation::IsOnGround)
Q_DECLARE_METATYPE(BlackMisc::Aviation::CAircraftSituation::OnGroundDetails)
Q_DECLARE_METATYPE(BlackMisc::Aviation::CAircraftSituation::AltitudeCorrection)
Q_DECLARE_METATYPE(BlackMisc::Aviation::CAircraftSituation::GndElevationInfo)

#endif // guard
