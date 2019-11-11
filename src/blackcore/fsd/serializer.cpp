/* Copyright (C) 2019
 * swift project community / contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "serializer.h"

using namespace BlackMisc::Aviation;

namespace BlackCore
{
    namespace Fsd
    {
        //! private @{

        template<>
        QString toQString(const AtcRating &value)
        {
            switch (value)
            {
            case AtcRating::Unknown:        return "0";
            case AtcRating::Observer:       return "1";
            case AtcRating::Student:        return "2";
            case AtcRating::Student2:       return "3";
            case AtcRating::Student3:       return "4";
            case AtcRating::Controller1:    return "5";
            case AtcRating::Controller2:    return "6";
            case AtcRating::Controller3:    return "7";
            case AtcRating::Instructor1:    return "8";
            case AtcRating::Instructor2:    return "9";
            case AtcRating::Instructor3:    return "10";
            case AtcRating::Supervisor:     return "11";
            case AtcRating::Administrator:  return "12";
            }
            Q_UNREACHABLE();
            return {};
        }

        template<>
        AtcRating fromQString(const QString &str)
        {
            if (str == "1") return AtcRating::Observer;
            else if (str == "2")  return AtcRating::Student;
            else if (str == "3")  return AtcRating::Student2;
            else if (str == "4")  return AtcRating::Student3;
            else if (str == "5")  return AtcRating::Controller1;
            else if (str == "6")  return AtcRating::Controller2;
            else if (str == "7")  return AtcRating::Controller3;
            else if (str == "8")  return AtcRating::Instructor1;
            else if (str == "9")  return AtcRating::Instructor2;
            else if (str == "10") return AtcRating::Instructor3;
            else if (str == "11") return AtcRating::Supervisor;
            else if (str == "12") return AtcRating::Administrator;
            else return AtcRating::Unknown;
        }

        template<>
        QString toQString(const PilotRating &value)
        {
            switch (value)
            {
            case PilotRating::Unknown:      return "0";
            case PilotRating::Student:      return "1";
            case PilotRating::VFR:          return "2";
            case PilotRating::IFR:          return "3";
            case PilotRating::Instructor:   return "4";
            case PilotRating::Supervisor:   return "5";
            }
            Q_UNREACHABLE();
            return {};
        }

        template<>
        PilotRating fromQString(const QString &str)
        {
            if (str == "0") return PilotRating::Unknown;
            else if (str == "1") return PilotRating::Student;
            else if (str == "2") return PilotRating::VFR;
            else if (str == "3") return PilotRating::IFR;
            else if (str == "4") return PilotRating::Instructor;
            else if (str == "5") return PilotRating::Supervisor;
            Q_UNREACHABLE();
            return {};
        }

        template<>
        QString toQString(const SimType &value)
        {
            switch (value)
            {
            case SimType::Unknown:      return "0";
            case SimType::MSFS95:       return "1";
            case SimType::MSFS98:       return "2";
            case SimType::MSCFS:        return "3";
            case SimType::MSFS2000:     return "4";
            case SimType::MSCFS2:       return "5";
            case SimType::MSFS2002:     return "6";
            case SimType::MSCFS3:       return "7";
            case SimType::MSFS2004:     return "8";
            case SimType::MSFSX:        return "9";
            case SimType::XPLANE8:      return "12";
            case SimType::XPLANE9:      return "13";
            case SimType::XPLANE10:     return "14";
            case SimType::XPLANE11:     return "16";
            case SimType::FlightGear:   return "25";
            case SimType::P3Dv1:        return "30";
            case SimType::P3Dv2:        return "30";
            case SimType::P3Dv3:        return "30";
            case SimType::P3Dv4:        return "30";
            }
            Q_UNREACHABLE();
            return {};
        }

        template<>
        SimType fromQString(const QString &str)
        {
            if (str == "0") return SimType::Unknown;
            else if (str == "1") return SimType::MSFS95;
            else if (str == "2") return SimType::MSFS98;
            else if (str == "3") return SimType::MSCFS;
            else if (str == "4") return SimType::MSFS2000;
            else if (str == "5") return SimType::MSCFS2;
            else if (str == "6") return SimType::MSFS2002;
            else if (str == "7") return SimType::MSCFS3;
            else if (str == "8") return SimType::MSFS2004;
            else if (str == "9") return SimType::MSFSX;
            else if (str == "12") return SimType::XPLANE8;
            else if (str == "13") return SimType::XPLANE9;
            else if (str == "14") return SimType::XPLANE10;
            else if (str == "16") return SimType::XPLANE11;
            else if (str == "25") return SimType::FlightGear;
            // Still ambigious which P3D version. No standard defined yet.
            else if (str == "30") return SimType::P3Dv4;
            else return SimType::Unknown;
        }

        template<>
        QString toQString(const BlackMisc::Network::CFacilityType &value)
        {
            using namespace BlackMisc::Network;
            switch (value.getFacilityType())
            {
            case CFacilityType::OBS: return "0";
            case CFacilityType::FSS: return "1";
            case CFacilityType::DEL: return "2";
            case CFacilityType::GND: return "3";
            case CFacilityType::TWR: return "4";
            case CFacilityType::APP: return "5";
            case CFacilityType::CTR: return "6";
            case CFacilityType::Unknown: return {};
            }
            Q_UNREACHABLE();
            return {};
        }

        template<>
        BlackMisc::Network::CFacilityType fromQString(const QString &str)
        {
            using namespace BlackMisc::Network;
            if (str == "0") return CFacilityType::OBS;
            if (str == "1") return CFacilityType::FSS;
            else if (str == "2") return CFacilityType::DEL;
            else if (str == "3") return CFacilityType::GND;
            else if (str == "4") return CFacilityType::TWR;
            else if (str == "5") return CFacilityType::APP;
            else if (str == "6") return CFacilityType::CTR;
            else return CFacilityType::Unknown;
        }

        template<>
        QString toQString(const ClientQueryType &value)
        {
            switch (value)
            {
            case ClientQueryType::IsValidATC:     return "ATC";
            case ClientQueryType::Capabilities:   return "CAPS";
            case ClientQueryType::Com1Freq:       return "C?";
            case ClientQueryType::RealName:       return "RN";
            case ClientQueryType::Server:         return "SV";
            case ClientQueryType::ATIS:           return "ATIS";
            case ClientQueryType::PublicIP:       return "IP";
            case ClientQueryType::INF:            return "INF";
            case ClientQueryType::FP:             return "FP";
            case ClientQueryType::AircraftConfig: return "ACC";
            case ClientQueryType::Unknown:        return "Unknown query type";
            }
            Q_UNREACHABLE();
            return {};
        }

        template<>
        ClientQueryType fromQString(const QString &str)
        {
            if (str == "ATC")       return ClientQueryType::IsValidATC;
            else if (str == "CAPS") return ClientQueryType::Capabilities;
            else if (str == "C?")   return ClientQueryType::Com1Freq;
            else if (str == "RN")   return ClientQueryType::RealName;
            else if (str == "SV")   return ClientQueryType::Server;
            else if (str == "ATIS") return ClientQueryType::ATIS;
            else if (str == "IP")   return ClientQueryType::PublicIP;
            else if (str == "INF")  return ClientQueryType::INF;
            else if (str == "FP")   return ClientQueryType::FP;
            else if (str == "ACC")  return ClientQueryType::AircraftConfig;
            else
            {
                // networkLog(vatSeverityDebug, "ClientQueryType fromString(str)", "Unknown client query type");
                return ClientQueryType::Unknown;
            }
        }

        template<>
        QString toQString(const FlightType &value)
        {
            switch (value)
            {
            case FlightType::IFR:  return "I";
            case FlightType::VFR:  return "V";
            case FlightType::SVFR: return "S";
            case FlightType::DVFR: return "D";
            }
            Q_UNREACHABLE();
            return {};
        }

        template<>
        FlightType fromQString(const QString &str)
        {
            if (str == QLatin1String("I"))       return FlightType::IFR;
            else if (str == QLatin1String("V"))  return FlightType::VFR;
            else if (str == QLatin1String("S"))  return FlightType::SVFR;
            else if (str == QLatin1String("D"))  return FlightType::DVFR;
            else                                 return FlightType::IFR;
        }

        template<>
        QString toQString(const CTransponder::TransponderMode &value)
        {
            switch (value)
            {
            case CTransponder::StateStandby:
                return QStringLiteral("S");
            case CTransponder::ModeMil1:
            case CTransponder::ModeMil2:
            case CTransponder::ModeMil3:
            case CTransponder::ModeMil4:
            case CTransponder::ModeMil5:
            case CTransponder::ModeA:
            case CTransponder::ModeC:
            case CTransponder::ModeS:
                return QStringLiteral("N");
            case CTransponder::StateIdent:
                return QStringLiteral("Y");
            }
            Q_UNREACHABLE();
            return {};
        }

        template<>
        CTransponder::TransponderMode fromQString(const QString &str)
        {
            if (str == "S")       return CTransponder::StateStandby;
            else if (str == "N")  return CTransponder::ModeC;
            else if (str == "Y")  return CTransponder::StateIdent;
            else                  return CTransponder::StateStandby;
        }

        template<>
        QString toQString(const Capabilities& value)
        {
            switch (value)
            {
            case Capabilities::None: return {};
            case Capabilities::AtcInfo: return "ATCINFO";
            case Capabilities::SecondaryPos: return "SECPOS";
            case Capabilities::AircraftInfo: return "MODELDESC";
            case Capabilities::OngoingCoord: return "ONGOINGCOORD";
            case Capabilities::InterminPos: return "INTERIMPOS";
            case Capabilities::FastPos: return "FASTPOS";
            case Capabilities::Stealth: return "STEALTH";
            case Capabilities::AircraftConfig: return "ACCONFIG";
            }
            return {};
        }

        template<>
        Capabilities fromQString(const QString &str)
        {
            if (str == "ATCINFO") return Capabilities::AtcInfo;
            else if (str == "SECPOS") return Capabilities::SecondaryPos;
            else if (str == "MODELDESC") return Capabilities::AircraftInfo;
            else if (str == "ONGOINGCOORD") return Capabilities::OngoingCoord;
            else if (str == "INTERIMPOS") return Capabilities::InterminPos;
            else if (str == "FASTPOS") return Capabilities::FastPos;
            else if (str == "STEALTH") return Capabilities::Stealth;
            else if (str == "ACCONFIG") return Capabilities::AircraftConfig;
            else return Capabilities::None;
        }

        template<>
        AtisLineType fromQString(const QString &str)
        {
            if (str == "V") return AtisLineType::VoiceRoom;
            else if (str == "Z") return AtisLineType::ZuluLogoff;
            else if (str == "T") return AtisLineType::TextMessage;
            else if (str == "E") return AtisLineType::LineCount;
            else return AtisLineType::Unknown;
        }

        //! @}
    }
}
