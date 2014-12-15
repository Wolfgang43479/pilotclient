/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKCORE_CONTEXTSIMULATOR_IMPL_H
#define BLACKCORE_CONTEXTSIMULATOR_IMPL_H

#include "blackcore/context_simulator.h"
#include "blackcore/context_network.h"
#include "blackcore/simulator.h"

#include "blacksim/simulatorinfo.h"
#include "blacksim/simulatorinfolist.h"

#include <QTimer>
#include <QDir>
#include <QtConcurrent/QtConcurrent>

namespace BlackCore
{
    /*!
     * Network simulator concrete implementation
     */
    class CContextSimulator : public IContextSimulator
    {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", BLACKCORE_CONTEXTSIMULATOR_INTERFACENAME)
        friend class CRuntime;
        friend class IContextSimulator;

    public:
        //! Destructor
        virtual ~CContextSimulator();

    public slots:

        //! \copydoc IContextSimulator::getSimulatorPluginList()
        virtual BlackSim::CSimulatorInfoList getAvailableSimulatorPlugins() const override;

        //! \copydoc IContextSimulator::isConnected()
        virtual bool isConnected() const override;

        //! \copydoc IContextSimulator::canConnect
        virtual bool canConnect() const override;

        //! \copydoc IContextSimulator::connectTo
        virtual bool connectTo() override;

        //! \copydoc IContextSimulator::asyncConnectTo
        virtual void asyncConnectTo() override;

        //! \copydoc IContextSimulator::disconnectFrom
        virtual bool disconnectFrom() override;

        //! \copydoc IContextSimulator::isPaused
        virtual bool isPaused() const override;

        //! \copydoc IContextSimulator::isRunning
        virtual bool isRunning() const override;

        //! \copydoc IContextSimulator::getSimulatorInfo()
        virtual BlackSim::CSimulatorInfo getSimulatorInfo() const override;

        //! \copydoc IContextSimulator::getAircraftModel()
        virtual BlackMisc::Network::CAircraftModel getOwnAircraftModel() const override;

        //! \copydoc IContextSimulator::getAirportsInRange()
        virtual BlackMisc::Aviation::CAirportList getAirportsInRange() const override;

        //! \copydoc IContextSimulator::getInstalledModels()
        virtual BlackMisc::Network::CAircraftModelList getInstalledModels() const override;

        //! \copydoc IContextSimulator::getCurrentlyMatchedModels()
        virtual BlackMisc::Network::CAircraftModelList getCurrentlyMatchedModels() const override;

        //! \copydoc IContextSimulator::setTimeSynchronization
        virtual void setTimeSynchronization(bool enable, BlackMisc::PhysicalQuantities::CTime offset) override;

        //! \copydoc IContextSimulator::isTimeSynchronized
        virtual bool isTimeSynchronized() const override;

        //! \copydoc IContextSimulator::getTimeSynchronizationOffset
        virtual BlackMisc::PhysicalQuantities::CTime getTimeSynchronizationOffset() const override;

        //! \copydoc IContextSimulator::loadSimulatorPlugin()
        virtual bool loadSimulatorPlugin(const BlackSim::CSimulatorInfo &simulatorInfo) override;

        //! \copydoc IContextSimulator::loadSimulatorPluginFromSettings()
        virtual bool loadSimulatorPluginFromSettings() override;

        //! \copydoc IContextSimulator::unloadSimulatorPlugin()
        virtual void unloadSimulatorPlugin() override;

        //! \copydoc IContextSimulator::settingsChanged
        virtual void settingsChanged(uint type) override;

    protected:
        //! \brief Constructor
        CContextSimulator(CRuntimeConfig::ContextMode, CRuntime *runtime);

        //! Register myself in DBus
        CContextSimulator *registerWithDBus(CDBusServer *server)
        {
            if (!server || this->m_mode != CRuntimeConfig::LocalInDbusServer) return this;
            server->addObject(CContextSimulator::ObjectPath(), this);
            return this;
        }

    private slots:
        //! Update own aircraft context, because simulator has changed something
        void ps_updateOwnAircraftContext();

        //! \copydoc ISimulator::addRemoteAircraft
        // void ps_addRemoteAircraft(const BlackMisc::Aviation::CCallsign &callsign, const BlackMisc::Aviation::CAircraftSituation &initialSituation);
        void ps_addRemoteAircraft(const BlackMisc::Aviation::CAircraft &remoteAircraft, const BlackMisc::Network::CClient &remoteClient);

        //! \copydoc ISimulator::addAircraftSituation
        void ps_addAircraftSituation(const BlackMisc::Aviation::CCallsign &callsign, const BlackMisc::Aviation::CAircraftSituation &situation);

        //! \copydoc ISimulator::removeRemoteAircraft
        void ps_removeRemoteAircraft(const BlackMisc::Aviation::CCallsign &callsign);

        //! Update simulator cockpit from context, because someone else has changed cockpit (e.g. GUI, 3rd party)
        //! \remarks set by runtime, only to be used locally (not via DBus)
        void ps_updateSimulatorCockpitFromContext(const BlackMisc::Aviation::CAircraft &ownAircraft, const QString &originator);

        //! Handle new connection status
        void ps_onConnectionStatusChanged(ISimulator::ConnectionStatus status);

        //! Text message received
        void ps_textMessagesReceived(const BlackMisc::Network::CTextMessageList &textMessages);

    private:
        //! \brief find and catalog all simulator plugins
        void findSimulatorPlugins();

        BlackCore::ISimulator *m_simulator = nullptr; //!< simulator plugin
        QTimer *m_updateTimer = nullptr;
        QDir m_pluginsDir;
        QSet<ISimulatorFactory *> m_simulatorFactories;
        QFuture<bool> m_canConnectResult;
    };

} // namespace BlackCore

#endif // guard
