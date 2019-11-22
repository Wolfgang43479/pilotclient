/* Copyright (C) 2014
 * swift project community / contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#define _CRT_SECURE_NO_WARNINGS

#include "fs9client.h"
#include "multiplayerpackets.h"
#include "multiplayerpacketparser.h"
#include "directplayerror.h"
#include "directplayutils.h"
#include "../fscommon/simulatorfscommonfunctions.h"
#include "blackcore/simulator.h"
#include "blackmisc/aviation/aircraftsituation.h"
#include "blackmisc/geo/coordinategeodetic.h"
#include "blackmisc/logmessage.h"
#include <QScopedArrayPointer>

using namespace BlackMisc;
using namespace BlackMisc::Aviation;
using namespace BlackMisc::Simulation;
using namespace BlackMisc::PhysicalQuantities;
using namespace BlackMisc::Geo;
using namespace BlackCore;
using namespace BlackSimPlugin::FsCommon;

namespace BlackSimPlugin
{
    namespace Fs9
    {
        CFs9Client::CFs9Client(const CCallsign &callsign, const QString &modelName,
                               const CTime &updateInterval,
                               CInterpolationLogger *logger, ISimulator *simulator) :
            CDirectPlayPeer(simulator, callsign),
            m_updateInterval(updateInterval),
            m_interpolator(callsign, simulator, simulator, simulator->getRemoteAircraftProvider(), logger),
            m_modelName(modelName)
        {
            m_interpolator.attachLogger(logger);
            Q_ASSERT_X(this->simulator(), Q_FUNC_INFO, "Wrong owner, expect simulator object");
        }

        MPPositionVelocity aircraftSituationToFS9(const CAircraftSituation &oldSituation, const CAircraftSituation &newSituation, double updateInterval)
        {
            Q_UNUSED(oldSituation);
            Q_UNUSED(updateInterval);
            MPPositionVelocity positionVelocity;

            // Latitude - integer and decimal places
            const double latitude = newSituation.getPosition().latitude().value(CAngleUnit::deg()) * 10001750.0 / 90.0;
            positionVelocity.lat_i = static_cast<qint32>(latitude);
            positionVelocity.lat_f = static_cast<quint16>(qRound(qAbs((latitude - positionVelocity.lat_i) * 65536)));

            // Longitude - integer and decimal places
            const double longitude = newSituation.getPosition().longitude().value(CAngleUnit::deg()) * (65536.0 * 65536.0) / 360.0;
            positionVelocity.lon_hi = static_cast<qint32>(longitude);
            positionVelocity.lon_lo = static_cast<quint16>(qRound(qAbs((longitude - positionVelocity.lon_hi) * 65536)));

            // Altitude - integer and decimal places
            const double altitude = newSituation.getAltitude().value(CLengthUnit::m());
            positionVelocity.alt_i = static_cast<qint32>(altitude);
            positionVelocity.alt_f = static_cast<quint16>(qRound((altitude - positionVelocity.alt_i) * 65536));

            // Pitch, Bank and Heading
            FS_PBH pbhstrct;
            pbhstrct.hdg   = static_cast<unsigned int>(qRound(newSituation.getHeading().value(CAngleUnit::deg()) * CFs9Sdk::headingMultiplier()));
            pbhstrct.pitch = qRound(std::floor(newSituation.getPitch().value(CAngleUnit::deg()) * CFs9Sdk::pitchMultiplier()));
            pbhstrct.bank  = qRound(std::floor(newSituation.getBank().value(CAngleUnit::deg()) * CFs9Sdk::bankMultiplier()));
            // MSFS has inverted pitch and bank angles
            pbhstrct.pitch = ~pbhstrct.pitch;
            pbhstrct.bank  = ~pbhstrct.bank;
            pbhstrct.onground = newSituation.isOnGround() ? 1 : 0;
            positionVelocity.pbh = pbhstrct.pbh;

            // Ground velocity
            positionVelocity.ground_velocity = static_cast<quint16>(newSituation.getGroundSpeed().valueInteger(CSpeedUnit::m_s()));

            // Altitude velocity
            CCoordinateGeodetic oldPosition = oldSituation.getPosition();
            CCoordinateGeodetic newPosition = newSituation.getPosition();
            CCoordinateGeodetic helperPosition;

            // We want the distance in Latitude direction. Longitude must be equal for old and new position.
            helperPosition.setLatitude(newPosition.latitude());
            helperPosition.setLongitude(oldPosition.longitude());
            const CLength distanceLatitudeObj = calculateGreatCircleDistance(oldPosition, helperPosition);

            // Now we want the Longitude distance. Latitude must be equal for old and new position.
            helperPosition.setLatitude(oldPosition.latitude());
            helperPosition.setLongitude(newSituation.longitude());
            const CLength distanceLongitudeObj = calculateGreatCircleDistance(oldPosition, helperPosition);

            // Latitude and Longitude velocity
            positionVelocity.lat_velocity = qRound(distanceLatitudeObj.value(CLengthUnit::ft()) * 65536.0 / updateInterval);
            if (oldPosition.latitude().value() > newSituation.latitude().value()) positionVelocity.lat_velocity *= -1;
            positionVelocity.lon_velocity = qRound(distanceLongitudeObj.value(CLengthUnit::ft()) * 65536.0 / updateInterval);
            if (oldPosition.longitude().value() > newSituation.longitude().value()) positionVelocity.lon_velocity *= -1;

            return positionVelocity;
        }

        MPPositionSlewMode aircraftSituationToFS9(const CAircraftSituation &situation)
        {
            MPPositionSlewMode positionSlewMode;

            // Latitude - integer and decimal places
            const double latitude = situation.getPosition().latitude().value(CAngleUnit::deg()) * 10001750.0 / 90.0;
            positionSlewMode.lat_i = static_cast<qint32>(latitude);
            positionSlewMode.lat_f = static_cast<quint16>(qAbs((latitude - positionSlewMode.lat_i) * 65536));

            // Longitude - integer and decimal places
            const double longitude  = situation.getPosition().longitude().value(CAngleUnit::deg()) * (65536.0 * 65536.0) / 360.0;
            positionSlewMode.lon_hi = static_cast<qint32>(longitude);
            positionSlewMode.lon_lo = static_cast<quint16>(qAbs((longitude - positionSlewMode.lon_hi) * 65536));

            // Altitude - integer and decimal places
            double altitude = situation.getAltitude().value(CLengthUnit::m());
            positionSlewMode.alt_i = static_cast<qint32>(altitude);
            positionSlewMode.alt_f = static_cast<quint16>((altitude - positionSlewMode.alt_i) * 65536);

            // Pitch, Bank and Heading
            FS_PBH pbhstrct;
            pbhstrct.hdg   = static_cast<unsigned int>(qRound(situation.getHeading().value(CAngleUnit::deg()) * CFs9Sdk::headingMultiplier()));
            pbhstrct.pitch = qRound(std::floor(situation.getPitch().value(CAngleUnit::deg()) * CFs9Sdk::pitchMultiplier()));
            pbhstrct.bank  = qRound(std::floor(situation.getBank().value(CAngleUnit::deg()) * CFs9Sdk::bankMultiplier()));
            // MSFS has inverted pitch and bank angles
            pbhstrct.pitch = ~pbhstrct.pitch;
            pbhstrct.bank  = ~pbhstrct.bank;

            pbhstrct.onground = situation.isOnGround() ? 1 : 0;
            positionSlewMode.pbh = pbhstrct.pbh;

            return positionSlewMode;
        }

        CFs9Client::~CFs9Client()
        {
            closeConnection();
            SafeRelease(m_hostAddress);
        }

        void CFs9Client::sendTextMessage(const QString &textMessage)
        {
            MPChatText mpChatText;
            mpChatText.chat_data = textMessage;
            QByteArray message;
            MultiPlayerPacketParser::writeType(message, CFs9Sdk::MPCHAT_PACKET_ID_CHAT_TEXT_SEND);
            MultiPlayerPacketParser::writeSize(message, mpChatText.size());
            message = MultiPlayerPacketParser::writeMessage(message, mpChatText);
            sendMessage(message);
        }

        void CFs9Client::setHostAddress(const QString &hostAddress)
        {
            HRESULT hr = s_ok();

            // Create our IDirectPlay8Address Host Address
            if (isFailure(hr = CoCreateInstance(CLSID_DirectPlay8Address, nullptr,
                                                CLSCTX_INPROC_SERVER,
                                                IID_IDirectPlay8Address,
                                                reinterpret_cast<void **>(&m_hostAddress))))
            {
                logDirectPlayError(hr);
                return;
            }

            if (isFailure(hr = m_hostAddress->BuildFromURLA(hostAddress.toLatin1().data())))
            {
                logDirectPlayError(hr);
                return;
            }
        }

        void CFs9Client::start()
        {
            initDirectPlay();
            createDeviceAddress();
            connectToSession(m_callsign);
        }

        CStatusMessageList CFs9Client::getInterpolationMessages(CInterpolationAndRenderingSetupBase::InterpolatorMode mode) const
        {
            if (!this->getInterpolator()) { return CStatusMessageList(); }
            return this->getInterpolator()->getInterpolationMessages(mode);
        }

        void CFs9Client::timerEvent(QTimerEvent *event)
        {
            Q_UNUSED(event)

            sendMultiplayerPosition();
            sendMultiplayerParamaters();
        }

        HRESULT CFs9Client::enumDirectPlayHosts()
        {
            HRESULT hr = s_ok();
            if (isFailure(hr = createHostAddress()))
            {
                CLogMessage(this).warning(u"FS9Client isFailure to create host address!");
                return hr;
            }

            // Now set up the Application Description
            DPN_APPLICATION_DESC dpAppDesc;
            ZeroMemory(&dpAppDesc, sizeof(DPN_APPLICATION_DESC));
            dpAppDesc.dwSize = sizeof(DPN_APPLICATION_DESC);
            dpAppDesc.guidApplication = CFs9Sdk::guid();

            // We now have the host address so lets enum
            if (isFailure(hr = m_directPlayPeer->EnumHosts(&dpAppDesc,             // pApplicationDesc
                               m_hostAddress,                  // pdpaddrHost
                               m_deviceAddress,                // pdpaddrDeviceInfo
                               nullptr, 0,                     // pvUserEnumData, size
                               0,                              // dwEnumCount
                               0,                              // dwRetryInterval
                               0,                              // dwTimeOut
                               nullptr,                        // pvUserContext
                               nullptr,                        // pAsyncHandle
                               DPNENUMHOSTS_SYNC)))            // dwFlags
            {
                return logDirectPlayError(hr);
            }
            return hr;
        }

        HRESULT CFs9Client::createHostAddress()
        {
            HRESULT hr = s_ok();

            // Create our IDirectPlay8Address Host Address
            if (isFailure(hr = CoCreateInstance(CLSID_DirectPlay8Address, nullptr,
                                                CLSCTX_INPROC_SERVER,
                                                IID_IDirectPlay8Address,
                                                reinterpret_cast<void **>(&m_hostAddress))))
            {
                return logDirectPlayError(hr);
            }

            // Set the SP for our Host Address
            if (isFailure(hr = m_hostAddress->SetSP(&CLSID_DP8SP_TCPIP)))
            {
                return logDirectPlayError(hr);
            }

            // FIXME: Test if this is also working via network or if we have to use the IP address
            const wchar_t hostname[] = L"localhost";

            // Set the hostname into the address
            if (isFailure(hr = m_hostAddress->AddComponent(DPNA_KEY_HOSTNAME, hostname,
                               2 * (wcslen(hostname) + 1), /*bytes*/
                               DPNA_DATATYPE_STRING)))
            {
                return logDirectPlayError(hr);
            }

            return hr;
        }

        HRESULT CFs9Client::connectToSession(const CCallsign &callsign)
        {
            HRESULT hr = s_ok();
            if (m_clientStatus == Connected) { return hr; }

            QScopedArrayPointer<wchar_t> wszPlayername(new wchar_t[static_cast<uint>(callsign.toQString().size() + 1)]);
            callsign.toQString().toWCharArray(wszPlayername.data());
            wszPlayername[callsign.toQString().size()] = 0;

            Q_ASSERT(!m_modelName.isEmpty());
            ZeroMemory(&m_playerInfo, sizeof(PLAYER_INFO_STRUCT));
            strcpy(m_playerInfo.szAircraft, qPrintable(m_modelName));
            m_playerInfo.dwFlags = 6;

            // Prepare and set the player information structure.
            ZeroMemory(&m_player, sizeof(DPN_PLAYER_INFO));
            m_player.dwSize = sizeof(DPN_PLAYER_INFO);
            m_player.pvData = &m_playerInfo;
            m_player.dwDataSize = sizeof(PLAYER_INFO_STRUCT);
            m_player.dwInfoFlags = DPNINFO_NAME | DPNINFO_DATA;
            m_player.pwszName = wszPlayername.data();
            if (isFailure(hr = m_directPlayPeer->SetPeerInfo(&m_player, nullptr, nullptr, DPNSETPEERINFO_SYNC)))
            {
                return logDirectPlayError(hr);
            }

            // Now set up the Application Description
            DPN_APPLICATION_DESC dpAppDesc;
            ZeroMemory(&dpAppDesc, sizeof(DPN_APPLICATION_DESC));
            dpAppDesc.dwSize = sizeof(DPN_APPLICATION_DESC);
            dpAppDesc.guidApplication = CFs9Sdk::guid();

            // We are now ready to host the app
            if (isFailure(hr = m_directPlayPeer->Connect(&dpAppDesc,      // AppDesc
                               m_hostAddress,
                               m_deviceAddress,
                               nullptr,
                               nullptr,
                               nullptr, 0,
                               nullptr,
                               nullptr,
                               nullptr,
                               DPNCONNECT_SYNC)))
            {
                return logDirectPlayError(hr);
            }

            CLogMessage(this).info(u"Callsign '%1' connected to session.") << m_callsign;
            sendMultiplayerChangePlayerPlane();
            sendMultiplayerPosition();
            sendMultiplayerParamaters();
            m_timerId = startTimer(m_updateInterval.valueInteger(CTimeUnit::ms()));

            m_clientStatus = Connected;
            emit statusChanged(m_callsign, m_clientStatus);

            return hr;
        }

        HRESULT CFs9Client::closeConnection()
        {
            HRESULT hr = s_ok();

            if (m_clientStatus == Disconnected) { return hr; }
            CLogMessage(this).info(u"Closing DirectPlay connection for '%1'") << m_callsign;
            if (isFailure(hr = m_directPlayPeer->Close(0)))
            {
                return logDirectPlayError(hr);
            }

            m_clientStatus = Disconnected;
            emit statusChanged(m_callsign, m_clientStatus);
            return hr;
        }

        void CFs9Client::sendMultiplayerPosition()
        {
            // remark: in FS9 there is no central updateRemoteAircraft() function, each FS9 client updates itself
            if (m_clientStatus == Disconnected) { return; }
            const bool forceFullUpdate = false;
            const CInterpolationAndRenderingSetupPerCallsign setup = this->simulator()->getInterpolationSetupConsolidated(m_callsign, forceFullUpdate);
            const CInterpolationResult result = m_interpolator.getInterpolation(QDateTime::currentMSecsSinceEpoch(), setup, 0);

            // Test only for successful position. FS9 requires constant positions
            if (!result.getInterpolationStatus().hasValidSituation()) { return; }
            this->sendMultiplayerPosition(result.getInterpolatedSituation());
        }

        void CFs9Client::sendMultiplayerPosition(const CAircraftSituation &situation)
        {
            MPPositionSlewMode positionSlewMode = aircraftSituationToFS9(situation);

            QByteArray positionMessage;
            MultiPlayerPacketParser::writeType(positionMessage, CFs9Sdk::MULTIPLAYER_PACKET_ID_POSITION_SLEWMODE);
            MultiPlayerPacketParser::writeSize(positionMessage, positionSlewMode.size());
            positionSlewMode.packet_index = m_packetIndex;
            ++m_packetIndex;
            positionMessage = MultiPlayerPacketParser::writeMessage(positionMessage, positionSlewMode);

            sendMessage(positionMessage);
        }

        void CFs9Client::sendMultiplayerParamaters()
        {
            QByteArray paramMessage;
            MPParam param;
            MultiPlayerPacketParser::writeType(paramMessage, CFs9Sdk::MULTIPLAYER_PACKET_ID_PARAMS);
            MultiPlayerPacketParser::writeSize(paramMessage, param.size());
            param.packet_index = m_packetIndex;
            ++m_packetIndex;
            paramMessage = MultiPlayerPacketParser::writeMessage(paramMessage, param);
            sendMessage(paramMessage);
        }

        void CFs9Client::sendMultiplayerChangePlayerPlane()
        {
            MPChangePlayerPlane mpChangePlayerPlane;
            mpChangePlayerPlane.engine = CFs9Sdk::ENGINE_TYPE_JET;
            mpChangePlayerPlane.aircraft_name = m_modelName;
            QByteArray message;
            MultiPlayerPacketParser::writeType(message, CFs9Sdk::MULTIPLAYER_PACKET_ID_CHANGE_PLAYER_PLANE);
            MultiPlayerPacketParser::writeSize(message, mpChangePlayerPlane.size());
            message = MultiPlayerPacketParser::writeMessage(message, mpChangePlayerPlane);
            sendMessage(message);
        }

        const ISimulator *CFs9Client::simulator() const
        {
            return qobject_cast<const ISimulator *>(this->parent());
        }
    }
}
