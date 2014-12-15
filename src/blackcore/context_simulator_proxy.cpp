/* Copyright (C) 2013 VATSIM Community / authors
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "blackcore/context_simulator_proxy.h"
#include <QObject>
#include <QMetaEnum>
#include <QDBusConnection>

using namespace BlackMisc;
using namespace BlackMisc::PhysicalQuantities;
using namespace BlackMisc::Aviation;
using namespace BlackMisc::Network;
using namespace BlackMisc::Geo;
using namespace BlackSim;

namespace BlackCore
{

    CContextSimulatorProxy::CContextSimulatorProxy(const QString &serviceName, QDBusConnection &connection, CRuntimeConfig::ContextMode mode, CRuntime *runtime) : IContextSimulator(mode, runtime), m_dBusInterface(nullptr)
    {
        this->m_dBusInterface = new BlackMisc::CGenericDBusInterface(
            serviceName , IContextSimulator::ObjectPath(), IContextSimulator::InterfaceName(),
            connection, this);
        this->relaySignals(serviceName, connection);
    }

    void CContextSimulatorProxy::relaySignals(const QString &serviceName, QDBusConnection &connection)
    {
        bool s = connection.connect(serviceName, IContextSimulator::ObjectPath(), IContextSimulator::InterfaceName(),
                                    "connectionChanged", this, SIGNAL(connectionChanged(bool)));
        Q_ASSERT(s);
        s = connection.connect(serviceName, IContextSimulator::ObjectPath(), IContextSimulator::InterfaceName(),
                               "startedChanged", this, SIGNAL(startedChanged(bool)));
        Q_ASSERT(s);
        s = connection.connect(serviceName, IContextSimulator::ObjectPath(), IContextSimulator::InterfaceName(),
                               "simulatorStatusChanged", this, SIGNAL(simulatorStatusChanged(bool, bool, bool)));
        Q_ASSERT(s);
        s = connection.connect(serviceName, IContextSimulator::ObjectPath(), IContextSimulator::InterfaceName(),
                               "ownAircraftModelChanged", this, SIGNAL(ownAircraftModelChanged(BlackMisc::Network::CAircraftModel)));
        Q_ASSERT(s);

        s = connection.connect(serviceName, IContextSimulator::ObjectPath(), IContextSimulator::InterfaceName(),
                               "modelMatchingCompleted", this, SIGNAL(modelMatchingCompleted(BlackMisc::Network::CAircraftModel)));
        Q_ASSERT(s);
        Q_UNUSED(s);
    }

    CSimulatorInfoList CContextSimulatorProxy::getAvailableSimulatorPlugins() const
    {
        return m_dBusInterface->callDBusRet<CSimulatorInfoList>(QLatin1Literal("getAvailableSimulatorPlugins"));
    }

    bool CContextSimulatorProxy::isConnected() const
    {
        return m_dBusInterface->callDBusRet<bool>(QLatin1Literal("isConnected"));
    }

    bool CContextSimulatorProxy::canConnect() const
    {
        return m_dBusInterface->callDBusRet<bool>(QLatin1Literal("canConnect"));
    }

    bool CContextSimulatorProxy::connectTo()
    {
        return m_dBusInterface->callDBusRet<bool>(QLatin1Literal("connectTo"));
    }

    void CContextSimulatorProxy::asyncConnectTo()
    {
        m_dBusInterface->callDBus(QLatin1Literal("asyncConnectTo"));
    }

    bool CContextSimulatorProxy::disconnectFrom()
    {
        return m_dBusInterface->callDBusRet<bool>(QLatin1Literal("disconnectFrom"));
    }

    BlackMisc::Network::CAircraftModel CContextSimulatorProxy::getOwnAircraftModel() const
    {
        return m_dBusInterface->callDBusRet<BlackMisc::Network::CAircraftModel>(QLatin1Literal("getOwnAircraftModel"));
    }

    CAirportList CContextSimulatorProxy::getAirportsInRange() const
    {
        return m_dBusInterface->callDBusRet<BlackMisc::Aviation::CAirportList>(QLatin1Literal("getAirportsInRange"));
    }

    CAircraftModelList CContextSimulatorProxy::getInstalledModels() const
    {
        return m_dBusInterface->callDBusRet<BlackMisc::Network::CAircraftModelList>(QLatin1Literal("getInstalledModels"));
    }

    CAircraftModelList CContextSimulatorProxy::getCurrentlyMatchedModels() const
    {
        return m_dBusInterface->callDBusRet<BlackMisc::Network::CAircraftModelList>(QLatin1Literal("getCurrentlyMatchedModels"));
    }

    BlackSim::CSimulatorInfo CContextSimulatorProxy::getSimulatorInfo() const
    {
        return m_dBusInterface->callDBusRet<BlackSim::CSimulatorInfo>(QLatin1Literal("getSimulatorInfo"));
    }

    void CContextSimulatorProxy::setTimeSynchronization(bool enable, CTime offset)
    {
        m_dBusInterface->callDBus(QLatin1Literal("setTimeSynchronization"), enable, offset);
    }

    bool CContextSimulatorProxy::isTimeSynchronized() const
    {
        return m_dBusInterface->callDBusRet<bool>(QLatin1Literal("isTimeSynchronized"));
    }

    CTime CContextSimulatorProxy::getTimeSynchronizationOffset() const
    {
        return m_dBusInterface->callDBusRet<BlackMisc::PhysicalQuantities::CTime>(QLatin1Literal("getTimeSynchronizationOffset"));
    }

    bool CContextSimulatorProxy::loadSimulatorPlugin(const BlackSim::CSimulatorInfo &simulatorInfo)
    {
        return m_dBusInterface->callDBusRet<bool>(QLatin1Literal("loadSimulatorPlugin"), simulatorInfo);
    }

    bool CContextSimulatorProxy::loadSimulatorPluginFromSettings()
    {
        return m_dBusInterface->callDBusRet<bool>(QLatin1Literal("loadSimulatorPluginFromSettings"));
    }

    void CContextSimulatorProxy::unloadSimulatorPlugin()
    {
        m_dBusInterface->callDBus(QLatin1Literal("unloadSimulatorPlugin"));
    }

    void CContextSimulatorProxy::settingsChanged(uint type)
    {
        m_dBusInterface->callDBus(QLatin1Literal("settingsChanged"), type);
    }

    bool CContextSimulatorProxy::isPaused() const
    {
        return m_dBusInterface->callDBusRet<bool>(QLatin1Literal("isPaused"));
    }

    bool CContextSimulatorProxy::isRunning() const
    {
        return m_dBusInterface->callDBusRet<bool>(QLatin1Literal("isRunning"));
    }

} // namespace BlackCore
