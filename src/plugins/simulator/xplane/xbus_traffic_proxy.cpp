/* Copyright (C) 2013 VATSIM Community / contributors
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xbus_traffic_proxy.h"
#include "blackcore/dbus_server.h"
#include <QMetaMethod>

#define XBUS_SERVICENAME "org.swift-project.xbus"

namespace BlackSimPlugin
{
    namespace XPlane
    {

        CXBusTrafficProxy::CXBusTrafficProxy(QDBusConnection &connection, QObject *parent, bool dummy) : QObject(parent)
        {
            m_dbusInterface = new BlackMisc::CGenericDBusInterface(XBUS_SERVICENAME, ObjectPath(), InterfaceName(), connection, this);
            if (! dummy) { relaySignals(); }
        }

        void CXBusTrafficProxy::relaySignals()
        {
            // TODO can this be refactored into CGenericDBusInterface?
            for (int i = 0, count = metaObject()->methodCount(); i < count; ++i)
            {
                auto method = metaObject()->method(i);
                if (method.methodType() == QMetaMethod::Signal)
                {
                    m_dbusInterface->connection().connect(m_dbusInterface->service(), m_dbusInterface->path(), m_dbusInterface->interface(),
                        method.name(), this, method.methodSignature().prepend("2"));
                }
            }
        }

        bool CXBusTrafficProxy::initialize()
        {
            return m_dbusInterface->callDBusRet<bool>(QLatin1String("initialize"));
        }

        void CXBusTrafficProxy::cleanup()
        {
            m_dbusInterface->callDBus(QLatin1String("cleanup"));
        }

        bool CXBusTrafficProxy::loadPlanesPackage(const QString &path)
        {
            return m_dbusInterface->callDBusRet<bool>(QLatin1String("loadPlanesPackage"), path);
        }

        void CXBusTrafficProxy::setDefaultIcao(const QString &defaultIcao)
        {
            m_dbusInterface->callDBus(QLatin1String("setDefaultIcao"), defaultIcao);
        }

        void CXBusTrafficProxy::setDrawingLabels(bool drawing)
        {
            m_dbusInterface->callDBus(QLatin1String("setDrawingLabels"), drawing);
        }

        bool CXBusTrafficProxy::isDrawingLabels() const
        {
            return m_dbusInterface->callDBusRet<bool>(QLatin1String("isDrawingLabels"));
        }

        void CXBusTrafficProxy::updateInstalledModels() const
        {
            m_dbusInterface->callDBus(QLatin1String("updateInstalledModels"));
        }

        void CXBusTrafficProxy::addPlane(const QString &callsign, const QString &aircraftIcao, const QString &airlineIcao, const QString &livery)
        {
            m_dbusInterface->callDBus(QLatin1String("addPlane"), callsign, aircraftIcao, airlineIcao, livery);
        }

        void CXBusTrafficProxy::removePlane(const QString &callsign)
        {
            m_dbusInterface->callDBus(QLatin1String("removePlane"), callsign);
        }

        void CXBusTrafficProxy::removeAllPlanes()
        {
            m_dbusInterface->callDBus(QLatin1String("removeAllPlanes"));
        }

        void CXBusTrafficProxy::setPlanePosition(const QString &callsign, double latitude, double longitude, double altitude, double pitch, double roll, double heading)
        {
            m_dbusInterface->callDBus(QLatin1String("setPlanePosition"), callsign, latitude, longitude, altitude, pitch, roll, heading);
        }

        void CXBusTrafficProxy::setPlaneSurfaces(const QString &callsign, double gear, double flap, double spoiler, double speedBrake, double slat, double wingSweep, double thrust,
            double elevator, double rudder, double aileron, bool landLight, bool beaconLight, bool strobeLight, bool navLight, int lightPattern)
        {
            m_dbusInterface->callDBus(QLatin1String("setPlaneSurfaces"), callsign, gear, flap, spoiler, speedBrake, slat, wingSweep, thrust, elevator, rudder, aileron,
                landLight, beaconLight, strobeLight, navLight, lightPattern);
        }

        void CXBusTrafficProxy::setPlaneTransponder(const QString &callsign, int code, bool modeC, bool ident)
        {
            m_dbusInterface->callDBus(QLatin1String("setPlaneTransponder"), callsign, code, modeC, ident);
        }

    }
}
