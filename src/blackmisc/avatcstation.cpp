/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "avatcstation.h"
#include "aviocomsystem.h"
#include "voiceroom.h"
#include "icon.h"
#include "propertyindex.h"
#include "blackmiscfreefunctions.h"

using namespace BlackMisc::PhysicalQuantities;
using namespace BlackMisc::Geo;
using namespace BlackMisc::Network;
using namespace BlackMisc::Audio;

namespace BlackMisc
{
    namespace Aviation
    {

        /*
         * Constructor
         */
        CAtcStation::CAtcStation()  :
            m_distanceToPlane(0, CLengthUnit::nullUnit()), m_isOnline(false), m_atis(CInformationMessage::ATIS), m_metar(CInformationMessage::METAR)
        {
            // void
        }

        /*
         * Constructor
         */
        CAtcStation::CAtcStation(const QString &callsign)  :
            m_callsign(callsign), m_distanceToPlane(0, CLengthUnit::nullUnit()), m_isOnline(false), m_atis(CInformationMessage::ATIS), m_metar(CInformationMessage::METAR)
        {
            // void
        }

        /*
         * Constructor
         */
        CAtcStation::CAtcStation(const CCallsign &callsign, const CUser &controller, const CFrequency &frequency, const CCoordinateGeodetic &pos, const CLength &range, bool isOnline,
                                 const QDateTime &bookedFromUtc, const QDateTime &bookedUntilUtc, const CInformationMessage &atis, const CInformationMessage &metar) :
            m_callsign(callsign), m_controller(controller), m_frequency(frequency), m_position(pos),
            m_range(range), m_distanceToPlane(0, CLengthUnit::nullUnit()), m_isOnline(isOnline),
            m_bookedFromUtc(bookedFromUtc), m_bookedUntilUtc(bookedUntilUtc), m_atis(atis), m_metar(metar)
        {
            // sync callsigns
            if (!this->m_controller.hasValidCallsign() && !callsign.isEmpty())
                this->m_controller.setCallsign(callsign);
        }

        /*
         * Convert to string
         */
        QString CAtcStation::convertToQString(bool i18n) const
        {
            QString s = i18n ?
                        QCoreApplication::translate("Aviation", "ATC station") :
                        "ATC station";
            s.append(' ').append(this->m_callsign.toQString(i18n));

            // position
            s.append(' ').append(this->m_position.toQString(i18n));

            // Online?
            s.append(' ');
            if (this->m_isOnline)
            {
                i18n ? s.append(QCoreApplication::translate("Aviation", "online")) : s.append("online");
            }
            else
            {
                i18n ? s.append(QCoreApplication::translate("Aviation", "offline")) : s.append("offline");
            }

            // controller name
            if (!this->m_controller.isValid())
            {
                s.append(' ');
                s.append(this->m_controller.toQString(i18n));
            }

            // frequency
            s.append(' ');
            s.append(this->m_frequency.valueRoundedWithUnit(3,  i18n));

            // ATIS
            if (this->hasAtis())
            {
                s.append(' ');
                s.append(this->m_atis.toQString(i18n));
            }

            // METAR
            if (this->hasMetar())
            {
                s.append(' ');
                s.append(this->m_metar.toQString(i18n));
            }

            // range
            s.append(' ');
            i18n ? s.append(s.append(QCoreApplication::translate("Aviation", "range"))) : s.append("range");
            s.append(": ");
            s.append(this->m_range.toQString(i18n));

            // distance to plane
            if (this->m_distanceToPlane.isPositiveWithEpsilonConsidered())
            {
                s.append(' ');
                i18n ? s.append(QCoreApplication::translate("Aviation", "distance")) : s.append("distance");
                s.append(' ');
                s.append(this->m_distanceToPlane.toQString(i18n));
            }

            // from / to
            if (!this->hasBookingTimes()) return s;

            // append from
            s.append(' ');
            i18n ? s.append(s.append(QCoreApplication::translate("Aviation", "from(UTC)"))) : s.append("from(UTC)");
            s.append(": ");
            if (this->m_bookedFromUtc.isNull())
            {
                s.append('-');
            }
            else
            {
                s.append(this->m_bookedFromUtc.toString("yy-MM-dd HH:mm"));
            }

            // append to
            s.append(' ');
            i18n ? s.append(s.append(QCoreApplication::translate("Aviation", "until(UTC)"))) : s.append("to(UTC)");
            s.append(": ");
            if (this->m_bookedFromUtc.isNull())
            {
                s.append('-');
            }
            else
            {
                s.append(this->m_bookedUntilUtc.toString("yy-MM-dd HH:mm"));
            }
            return s;

            // force strings for translation in resource files
            (void)QT_TRANSLATE_NOOP("Aviation", "ATC station");
            (void)QT_TRANSLATE_NOOP("Aviation", "online");
            (void)QT_TRANSLATE_NOOP("Aviation", "offline");
            (void)QT_TRANSLATE_NOOP("Aviation", "from(UTC)");
            (void)QT_TRANSLATE_NOOP("Aviation", "until(UTC)");
            (void)QT_TRANSLATE_NOOP("Aviation", "range");
            (void)QT_TRANSLATE_NOOP("Aviation", "distance");
            (void)QT_TRANSLATE_NOOP("Network", "voiceroom");
        }

        /*
         * Register metadata
         */
        void CAtcStation::registerMetadata()
        {
            qRegisterMetaType<CAtcStation>();
            qDBusRegisterMetaType<CAtcStation>();
        }

        /*
         * Members
         */
        const QStringList &CAtcStation::jsonMembers()
        {
            return TupleConverter<CAtcStation>::jsonMembers();
        }

        /*
         * To JSON
         */
        QJsonObject CAtcStation::toJson() const
        {
            return BlackMisc::serializeJson(CAtcStation::jsonMembers(), TupleConverter<CAtcStation>::toTuple(*this));
        }

        /*
         * From Json
         */
        void CAtcStation::fromJson(const QJsonObject &json)
        {
            BlackMisc::deserializeJson(json, CAtcStation::jsonMembers(), TupleConverter<CAtcStation>::toTuple(*this));
        }

        /*
         * Compare
         */
        int CAtcStation::compareImpl(const CValueObject &otherBase) const
        {
            const auto &other = static_cast<const CAtcStation &>(otherBase);
            return compare(TupleConverter<CAtcStation>::toTuple(*this), TupleConverter<CAtcStation>::toTuple(other));
        }

        /*
         * Marshall to DBus
         */
        void CAtcStation::marshallToDbus(QDBusArgument &argument) const
        {
            argument << TupleConverter<CAtcStation>::toTuple(*this);
        }

        /*
         * Unmarshall from DBus
         */
        void CAtcStation::unmarshallFromDbus(const QDBusArgument &argument)
        {
            argument >> TupleConverter<CAtcStation>::toTuple(*this);
        }

        /*
         * Equal?
         */
        bool CAtcStation::operator ==(const CAtcStation &other) const
        {
            if (this == &other) return true;
            return TupleConverter<CAtcStation>::toTuple(*this) == TupleConverter<CAtcStation>::toTuple(other);
        }

        /*
         * Hash
         */
        uint CAtcStation::getValueHash() const
        {
            return qHash(TupleConverter<CAtcStation>::toTuple(*this));
        }

        /*
         * Unequal?
         */
        bool CAtcStation::operator !=(const CAtcStation &other) const
        {
            return !((*this) == other);
        }

        /*
         * Frequency
         */
        void CAtcStation::setFrequency(const CFrequency &frequency)
        {
            this->m_frequency = frequency;
            this->m_frequency.setUnit(CFrequencyUnit::MHz());
        }

        /*
         * SyncronizeControllerData
         */
        void CAtcStation::syncronizeControllerData(CAtcStation &otherStation)
        {
            if (this->m_controller == otherStation.getController()) return;
            CUser otherController = otherStation.getController();
            this->m_controller.syncronizeData(otherController);
            otherStation.setController(otherController);
        }

        /*
         * Distance to planne
         */
        CLength CAtcStation::calculcateDistanceToPlane(const CCoordinateGeodetic &position, bool update)
        {
            if (!update) return Geo::greatCircleDistance(this->m_position, position);
            this->m_distanceToPlane = Geo::greatCircleDistance(this->m_position, position);
            return this->m_distanceToPlane;
        }

        /*
         * Booked now
         */
        bool CAtcStation::isBookedNow() const
        {
            if (!this->hasValidBookingTimes()) return false;
            QDateTime now = QDateTime::currentDateTimeUtc();
            if (this->m_bookedFromUtc > now) return false;
            if (now > this->m_bookedUntilUtc) return false;
            return true;
        }

        /*
         * When booked?
         */
        CTime CAtcStation::bookedWhen() const
        {
            if (!this->hasValidBookingTimes()) return CTime(-365.0, CTimeUnit::d());
            QDateTime now = QDateTime::currentDateTimeUtc();
            qint64 diffMs;
            if (this->m_bookedFromUtc > now)
            {
                // future
                diffMs = now.msecsTo(this->m_bookedFromUtc);
                return CTime(diffMs / 1000.0, CTimeUnit::s());
            }
            else if (this->m_bookedUntilUtc > now)
            {
                // now
                return CTime(0.0, CTimeUnit::s());
            }
            else
            {
                // past
                diffMs = m_bookedUntilUtc.msecsTo(now);
                return CTime(-diffMs / 1000.0, CTimeUnit::s());
            }
        }

        /*
         * Property by index
         */
        QVariant CAtcStation::propertyByIndex(const BlackMisc::CPropertyIndex &index) const
        {
            if (index.isMyself()) { return this->toQVariant(); }
            ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexBookedFrom:
                return QVariant(this->m_bookedFromUtc);
            case IndexBookedUntil:
                return QVariant(this->m_bookedUntilUtc);
            case IndexCallsign:
                return this->m_callsign.propertyByIndex(index.copyFrontRemoved());
            case IndexController:
                return this->m_controller.propertyByIndex(index.copyFrontRemoved());
            case IndexFrequency:
                return this->m_frequency.propertyByIndex(index.copyFrontRemoved());
            case IndexIsOnline:
                return QVariant(this->m_isOnline);
            case IndexLatitude:
                return this->latitude().propertyByIndex(index.copyFrontRemoved());
            case IndexDistance:
                return this->m_distanceToPlane.propertyByIndex(index.copyFrontRemoved());
            case IndexLongitude:
                return this->longitude().propertyByIndex(index.copyFrontRemoved());
            case IndexPosition:
                return this->m_position.propertyByIndex(index.copyFrontRemoved());
            case IndexRange:
                return this->m_range.propertyByIndex(index.copyFrontRemoved());
            case IndexAtis:
                return this->m_atis.propertyByIndex(index.copyFrontRemoved());
            case IndexMetar:
                return this->m_metar.propertyByIndex(index.copyFrontRemoved());
            case IndexVoiceRoom:
                return QVariant(this->m_voiceRoom.propertyByIndex(index.copyFrontRemoved()));
            default:
                if (ICoordinateGeodetic::canHandleIndex(index))
                {
                    return ICoordinateGeodetic::propertyByIndex(index);
                }
                return CValueObject::propertyByIndex(index);
            }
        }

        /*
         * Set property as index
         */
        void CAtcStation::setPropertyByIndex(const QVariant &variant, const BlackMisc::CPropertyIndex &index)
        {
            if (index.isMyself()) { this->fromQVariant(variant); return; }
            ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexBookedFrom:
                this->setBookedFromUtc(variant.value<QDateTime>());
                break;
            case IndexBookedUntil:
                this->setBookedUntilUtc(variant.value<QDateTime>());
                break;
            case IndexCallsign:
                this->m_callsign.setPropertyByIndex(variant, index.copyFrontRemoved());
                break;
            case IndexController:
                this->m_controller.setPropertyByIndex(variant, index.copyFrontRemoved());
                break;
            case IndexFrequency:
                this->m_frequency.setPropertyByIndex(variant, index.copyFrontRemoved());
                break;
            case IndexIsOnline:
                this->setOnline(variant.value<bool>());
                break;
            case IndexPosition:
                this->m_position.setPropertyByIndex(variant, index.copyFrontRemoved());
                break;
            case IndexRange:
                this->m_range.setPropertyByIndex(variant, index.copyFrontRemoved());
                break;
            case IndexDistance:
                this->m_distanceToPlane.setPropertyByIndex(variant, index.copyFrontRemoved());
                break;
            case IndexAtis:
                this->m_atis.setPropertyByIndex(variant, index.copyFrontRemoved());
                break;
            case IndexMetar:
                this->m_metar.setPropertyByIndex(variant, index.copyFrontRemoved());
                break;
            case IndexVoiceRoom:
                this->m_voiceRoom.setPropertyByIndex(variant, index.copyFrontRemoved());
                break;
            default:
                CValueObject::setPropertyByIndex(variant, index);
                break;
            }
        }

        /*
         * metaTypeId
         */
        int CAtcStation::getMetaTypeId() const
        {
            return qMetaTypeId<CAtcStation>();
        }

        /*
         * is a
         */
        bool CAtcStation::isA(int metaTypeId) const
        {
            if (metaTypeId == qMetaTypeId<CAtcStation>()) { return true; }

            return this->CValueObject::isA(metaTypeId);
        }

        /*
         * Property as string by index
         */
        QString CAtcStation::propertyByIndexAsString(const BlackMisc::CPropertyIndex &index, bool i18n) const
        {
            QVariant qv = this->propertyByIndex(index);
            ColumnIndex i = index.frontCasted<ColumnIndex>();

            // special treatment
            // this is required as it is possible an ATC station is not containing all
            // properties
            switch (i)
            {
            case IndexFrequency:
                if (!CComSystem::isValidCivilAviationFrequency(qv.value<CFrequency>()))
                    return "";
                else
                    return qv.value<CFrequency>().valueRoundedWithUnit(3, i18n);
                break;
            case IndexDistance:
                {
                    CLength distance = qv.value<CLength>();
                    if (distance.isNegativeWithEpsilonConsidered()) return "";
                    return distance.toQString(i18n);
                }

            case IndexBookedFrom:
            case IndexBookedUntil:
                {
                    QDateTime dt = qv.value<QDateTime>();
                    if (dt.isNull() || !dt.isValid()) return "";
                    return dt.toString("yyyy-MM-dd HH:mm");
                    break;
                }
            default:
                break;
            }
            return BlackMisc::qVariantToString(qv, i18n);
        }

    } // namespace
} // namespace
