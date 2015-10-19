/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_AVIATION_ATCSTATION_H
#define BLACKMISC_AVIATION_ATCSTATION_H

#include "blackmisc/blackmiscexport.h"
#include "blackmisc/audio/voiceroom.h"
#include "blackmisc/aviation/comsystem.h"
#include "blackmisc/aviation/informationmessage.h"
#include "blackmisc/aviation/callsign.h"
#include "blackmisc/network/user.h"
#include "blackmisc/geo/coordinategeodetic.h"
#include "blackmisc/pq/frequency.h"
#include "blackmisc/pq/length.h"
#include "blackmisc/pq/time.h"
#include <QDateTime>
#include <QMap>
#include <functional>

namespace BlackMisc
{
    namespace Aviation
    {
        //! Value object encapsulating information about an ATC station.
        class BLACKMISC_EXPORT CAtcStation : public CValueObject<CAtcStation>, public Geo::ICoordinateWithRelativePosition
        {
        public:
            //! Properties by index
            enum ColumnIndex
            {
                IndexCallsign = BlackMisc::CPropertyIndex::GlobalIndexCAtcStation,
                IndexController,
                IndexFrequency,
                IndexPosition,
                IndexRange,
                IndexIsInRange,
                IndexDistanceToOwnAircraft,
                IndexIsOnline,
                IndexBookedFrom,
                IndexBookedUntil,
                IndexLatitude,
                IndexLongitude,
                IndexAtis,
                IndexMetar,
                IndexVoiceRoom,
            };

            //! Default constructor.
            CAtcStation();

            //! Simplified constructor
            CAtcStation(const QString &callsign);

            //! ATC station constructor
            CAtcStation(const CCallsign &callsign, const BlackMisc::Network::CUser &controller,
                        const BlackMisc::PhysicalQuantities::CFrequency &frequency,
                        const BlackMisc::Geo::CCoordinateGeodetic &pos, const BlackMisc::PhysicalQuantities::CLength &range,
                        bool isOnline = false, const QDateTime &bookedFromUtc = QDateTime(),  const QDateTime &bookedUntilUtc = QDateTime(),
                        const CInformationMessage &atis = CInformationMessage(CInformationMessage::ATIS), const CInformationMessage &metar = CInformationMessage(CInformationMessage::METAR));

            //! \copydoc CValueObject::toIcon()
            BlackMisc::CIcon toIcon() const { return this->m_callsign.toIcon(); }

            //! Has booking times?
            bool hasBookingTimes() const;

            //! Has ATIS?
            bool hasAtis() const
            {
                return this->m_atis.hasMessage();
            }

            //! Has METAR?
            bool hasMetar() const;

            //! Get callsign.
            const CCallsign &getCallsign() const { return m_callsign; }

            //! Get callsign as string.
            QString getCallsignAsString() const { return m_callsign.asString(); }

            //! Callsign suffix (e.g. TWR)
            QString getCallsignSuffix() const;

            //! Set callsign
            void setCallsign(const CCallsign &callsign);

            //! Get controller
            const BlackMisc::Network::CUser &getController() const { return m_controller; }

            //! Get controller name.
            QString getControllerRealName() const { return m_controller.getRealName(); }

            //! Get controller name.
            QString getControllerId() const { return m_controller.getId(); }

            //! Set controller
            void setController(const BlackMisc::Network::CUser &controller) { this->m_controller = controller; this->m_controller.setCallsign(this->m_callsign); }

            //! Set controller's name
            void setControllerRealName(const QString &controllerName) { this->m_controller.setRealName(controllerName); }

            //! Set controller's id
            void setControllerId(const QString &controllerId) { this->m_controller.setId(controllerId); }

            //! Has valid realname?
            bool hasValidRealName() const { return this->m_controller.hasValidRealName(); }

            //! Has valid id?
            bool hasValidId() const { return this->m_controller.hasValidId(); }

            //! Valid COM frequency
            bool hasValidFrequency() const { return BlackMisc::Aviation::CComSystem::isValidCivilAviationFrequency(this->getFrequency()); }

            //! Get frequency.
            const BlackMisc::PhysicalQuantities::CFrequency &getFrequency() const { return m_frequency; }

            //! Set frequency
            void setFrequency(const BlackMisc::PhysicalQuantities::CFrequency &frequency);

            //! Get the position of the center of the controller's area of visibility.
            const BlackMisc::Geo::CCoordinateGeodetic &getPosition() const { return m_position; }

            //! Set position
            void setPosition(const BlackMisc::Geo::CCoordinateGeodetic &position) { this->m_position = position; }

            //! Syncronize controller data
            //! Updates two stations (normally a booked and online ATC station) with complementary data
            void syncronizeControllerData(CAtcStation &otherStation);


            //! Syncronize station data
            //! Updates the two stations (a booked and online ATC station) with complementary data
            //! \pre this object is the online station, the passed station the booked station
            void syncronizeWithBookedStation(CAtcStation &bookedStation);

            //! Get the radius of the controller's area of visibility.
            const BlackMisc::PhysicalQuantities::CLength &getRange() const { return m_range; }

            //! Set range
            void setRange(const BlackMisc::PhysicalQuantities::CLength &range) { this->m_range = range; }

            //! In range? If range and distance to own aircraft are not available false
            bool isInRange() const;

            //! Is station online (or just booked)?
            bool isOnline() const { return m_isOnline; }

            //! Set online
            void setOnline(bool online) { this->m_isOnline = online; }

            //! Get voice room
            const BlackMisc::Audio::CVoiceRoom &getVoiceRoom() const { return this->m_voiceRoom; }

            //! Set voice room
            void setVoiceRoom(const BlackMisc::Audio::CVoiceRoom &voiceRoom) { this->m_voiceRoom = voiceRoom; }

            //! Set voice room URL
            void setVoiceRoomUrl(const QString &url) { this->m_voiceRoom.setVoiceRoomUrl(url); }

            //! Valid voice room?
            bool hasValidVoiceRoom() const { return this->m_voiceRoom.isValid(); }

            //! Booked date/time if any.
            //! This represents the closest booking within a time frame as there can be multiple bookings.
            const QDateTime &getBookedFromUtc() const { return m_bookedFromUtc; }

            //! Booked date/time if any.
            //! This represents the closest booking within a time frame as there can be multiple bookings.
            const QDateTime &getBookedUntilUtc() const { return m_bookedUntilUtc; }

            //! Has valid booking times?
            bool hasValidBookingTimes() const;

            //! Set booked from
            void setBookedFromUtc(const QDateTime &from) { this->m_bookedFromUtc = from; }

            //! Transfer booking times
            void setBookedFromUntil(const CAtcStation &otherStation);

            //! Booked now?
            bool isBookedNow() const;

            //! Tuned in within 25KHz channel spacing
            bool isComUnitTunedIn25KHz(const BlackMisc::Aviation::CComSystem &comUnit) const;

            //! When booked, 0 means now,
            //! negative values mean booking in past,
            //! positive values mean booking in future,
            //! no booking dates will result in null time
            BlackMisc::PhysicalQuantities::CTime bookedWhen() const;

            //! Get ATIS
            const CInformationMessage &getAtis() const { return m_atis; }

            //! Set ATIS
            void setAtis(const CInformationMessage &atis) { this->m_atis = atis;}

            //! Set ATIS Message
            void setAtisMessage(const QString &atis) { this->m_atis.setMessage(atis); }

            //! Get METAR
            const CInformationMessage &getMetar() const { return m_metar; }

            //! Set METAR
            void setMetar(const CInformationMessage &metar) { this->m_metar = metar;}

            //! Set METAR Message
            void setMetarMessage(const QString &metar) { this->m_metar.setMessage(metar); }

            //! Set booked until
            void setBookedUntilUtc(const QDateTime &until) { this->m_bookedUntilUtc = until; }

            //! \copydoc ICoordinateGeodetic::latitude
            virtual BlackMisc::Geo::CLatitude latitude() const override;

            //! \copydoc ICoordinateGeodetic::longitude
            virtual BlackMisc::Geo::CLongitude longitude() const override;

            //! \copydoc ICoordinateGeodetic::geodeticHeight
            //! \remarks this should be used for elevation as depicted here: http://en.wikipedia.org/wiki/Altitude#mediaviewer/File:Vertical_distances.svg
            const BlackMisc::PhysicalQuantities::CLength &geodeticHeight() const override;

            //! \copydoc ICoordinateGeodetic::normalVector
            virtual QVector3D normalVector() const override;

            //! \copydoc CValueObject::propertyByIndex
            CVariant propertyByIndex(const BlackMisc::CPropertyIndex &index) const;

            //! \copydoc CValueObject::setPropertyByIndex
            void setPropertyByIndex(const CVariant &variant, const CPropertyIndex &index);

            //! \copydoc CValueObject::convertToQString
            QString convertToQString(bool i18n = false) const;

        private:
            BLACK_ENABLE_TUPLE_CONVERSION(CAtcStation)
            CCallsign                                 m_callsign;
            BlackMisc::Network::CUser                 m_controller;
            BlackMisc::PhysicalQuantities::CFrequency m_frequency;
            BlackMisc::Geo::CCoordinateGeodetic       m_position;
            BlackMisc::PhysicalQuantities::CLength    m_range;
            bool                                      m_isOnline = false;
            QDateTime                                 m_bookedFromUtc;
            QDateTime                                 m_bookedUntilUtc;
            CInformationMessage m_atis  { CInformationMessage::ATIS };
            CInformationMessage m_metar { CInformationMessage::METAR };
            BlackMisc::Audio::CVoiceRoom              m_voiceRoom;
        };
    } // namespace
} // namespace

BLACK_DECLARE_TUPLE_CONVERSION(BlackMisc::Aviation::CAtcStation, (
                                   o.m_callsign,
                                   o.m_controller,
                                   o.m_frequency,
                                   o.m_position,
                                   o.m_range,
                                   o.m_isOnline,
                                   o.m_atis,
                                   o.m_bookedFromUtc,
                                   o.m_bookedUntilUtc,
                                   o.m_metar,
                                   o.m_voiceRoom,
                                   o.m_distanceToOwnAircraft,
                                   o.m_bearingToOwnAircraft
                               ))
Q_DECLARE_METATYPE(BlackMisc::Aviation::CAtcStation)

#endif // guard
