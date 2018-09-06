/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKCORE_SIMULATOR_H
#define BLACKCORE_SIMULATOR_H

#include "blackcore/application.h"
#include "blackcore/blackcoreexport.h"
#include "blackmisc/simulation/settings/simulatorsettings.h"
#include "blackmisc/simulation/aircraftmodellist.h"
#include "blackmisc/simulation/simulatedaircraft.h"
#include "blackmisc/simulation/simulatorplugininfo.h"
#include "blackmisc/simulation/interpolationrenderingsetup.h"
#include "blackmisc/simulation/simulatorinternals.h"
#include "blackmisc/simulation/interpolatormulti.h"
#include "blackmisc/simulation/ownaircraftprovider.h"
#include "blackmisc/simulation/remoteaircraftprovider.h"
#include "blackmisc/simulation/simulationenvironmentprovider.h"
#include "blackmisc/simulation/interpolationsetupprovider.h"
#include "blackmisc/aviation/airportlist.h"
#include "blackmisc/aviation/callsignset.h"
#include "blackmisc/network/clientprovider.h"
#include "blackmisc/weather/weathergridprovider.h"
#include "blackmisc/geo/elevationplane.h"
#include "blackmisc/pq/length.h"
#include "blackmisc/pq/time.h"
#include "blackmisc/statusmessage.h"
#include "blackmisc/identifiable.h"
#include "blackmisc/identifier.h"
#include "blackmisc/pixmap.h"
#include "blackmisc/simplecommandparser.h"
#include "blackmisc/tokenbucket.h"
#include "blackconfig/buildconfig.h"

#include <QFlags>
#include <QObject>
#include <QString>
#include <atomic>

namespace BlackMisc
{
    namespace Aviation { class CCallsign; }
    namespace Network { class CTextMessage; }
}

namespace BlackCore
{
    //! Interface to a simulator.
    class BLACKCORE_EXPORT ISimulator :
        public QObject,
        public BlackMisc::Simulation::COwnAircraftAware,    // gain access to in memory own aircraft data
        public BlackMisc::Simulation::CRemoteAircraftAware, // gain access to in memory remote aircraft data
        public BlackMisc::Weather::CWeatherGridAware,       // gain access to in memory weather grid
        public BlackMisc::Network::CClientAware,            // the network client with its capabilities
        public BlackMisc::Simulation::ISimulationEnvironmentProvider, // give access to elevation etc.
        public BlackMisc::Simulation::IInterpolationSetupProvider,    // setup
        public BlackMisc::CIdentifiable
    {
        Q_OBJECT
        Q_INTERFACES(BlackMisc::Simulation::ISimulationEnvironmentProvider)
        Q_INTERFACES(BlackMisc::Simulation::IInterpolationSetupProvider)

    public:
        //! ISimulator status
        enum SimulatorStatusFlag
        {
            Unspecified  = 0,      //!< unspecied, needed as default value
            Disconnected = 1 << 0, //!< not connected, and hence not simulating/paused
            Connected    = 1 << 1, //!< Is the plugin connected to the simulator?
            Simulating   = 1 << 2, //!< Is the simulator actually simulating?
            Paused       = 1 << 3, //!< Is the simulator paused?
        };
        Q_DECLARE_FLAGS(SimulatorStatus, SimulatorStatusFlag)
        Q_FLAG(SimulatorStatus)

        //! Log categories
        static const BlackMisc::CLogCategoryList &getLogCategories();

        //! Render all aircraft if number of aircraft >= MaxAircraftInfinite
        const int MaxAircraftInfinite = 100;

        //! Destructor
        virtual ~ISimulator() override;

        //! Combined status
        virtual SimulatorStatus getSimulatorStatus() const;

        //! Is time synchronization on?
        virtual bool isTimeSynchronized() const = 0;

        //! Get the setup (simulator environment)
        virtual const BlackMisc::Simulation::CSimulatorInternals &getSimulatorInternals() const { return m_simulatorInternals; }

        //! Connect to simulator
        virtual bool connectTo() = 0;

        //! Disconnect from simulator
        virtual bool disconnectFrom() = 0;

        //! Logically add a new aircraft.
        //! Depending on max. aircraft, enabled status etc. it will physically added to the simulator.
        //! \sa physicallyAddRemoteAircraft
        virtual bool logicallyAddRemoteAircraft(const BlackMisc::Simulation::CSimulatedAircraft &remoteAircraft);

        //! Logically remove remote aircraft from simulator.
        //! Depending on max. aircraft, enabled status etc. it will physically added to the simulator.
        virtual bool logicallyRemoveRemoteAircraft(const BlackMisc::Aviation::CCallsign &callsign);

        //! Removes and adds again the aircraft
        //! \sa logicallyRemoveRemoteAircraft
        //! \sa logicallyAddRemoteAircraft
        virtual bool logicallyReAddRemoteAircraft(const BlackMisc::Aviation::CCallsign &callsign);

        //! Find the unrendered enabled aircraft
        virtual BlackMisc::Aviation::CCallsignSet unrenderedEnabledAircraft() const;

        //! Find the rendered disabled aircraft
        virtual BlackMisc::Aviation::CCallsignSet renderedDisabledAircraft() const;

        //! Change remote aircraft per property
        virtual bool changeRemoteAircraftModel(const BlackMisc::Simulation::CSimulatedAircraft &aircraft);

        //! Aircraft got enabled / disabled
        virtual bool changeRemoteAircraftEnabled(const BlackMisc::Simulation::CSimulatedAircraft &aircraft);

        //! Update own aircraft cockpit (usually from context)
        virtual bool updateOwnSimulatorCockpit(const BlackMisc::Simulation::CSimulatedAircraft &aircraft, const BlackMisc::CIdentifier &originator) = 0;

        //! Update own aircraft cockpit (usually from context)
        virtual bool updateOwnSimulatorSelcal(const BlackMisc::Aviation::CSelcal &selcal, const BlackMisc::CIdentifier &originator) = 0;

        //! Display a status message in the simulator
        virtual void displayStatusMessage(const BlackMisc::CStatusMessage &message) const = 0;

        //! Display a text message
        virtual void displayTextMessage(const BlackMisc::Network::CTextMessage &message) const = 0;

        //! Airports in range from simulator, or if not available from web service
        virtual BlackMisc::Aviation::CAirportList getAirportsInRange() const;

        //! Set time synchronization between simulator and user's computer time
        //! \remarks not all drivers implement this, e.g. if it is an intrinsic simulator feature
        virtual bool setTimeSynchronization(bool enable, const BlackMisc::PhysicalQuantities::CTime &offset) = 0;

        //! Time synchronization offset
        virtual BlackMisc::PhysicalQuantities::CTime getTimeSynchronizationOffset() const = 0;

        //! Is the aircraft rendered (displayed in simulator)?
        //! This shall only return true if the aircraft is really visible in the simulator
        virtual bool isPhysicallyRenderedAircraft(const BlackMisc::Aviation::CCallsign &callsign) const = 0;

        //! Physically rendered (displayed in simulator)
        //! This shall only return aircraft handled in the simulator
        virtual BlackMisc::Aviation::CCallsignSet physicallyRenderedAircraft() const = 0;

        //! Highlight the aircraft for given time (or disable highlight)
        virtual void highlightAircraft(const BlackMisc::Simulation::CSimulatedAircraft &aircraftToHighlight, bool enableHighlight, const BlackMisc::PhysicalQuantities::CTime &displayTime);

        //! Follow aircraft
        virtual bool followAircraft(const BlackMisc::Aviation::CCallsign &callsign);

        //! Activates or deactivates simulator weather
        virtual void setWeatherActivated(bool activated);

        //! Flight network has been connected
        //! \remark hint if network connected and we expect any planes
        //! \sa ISimulator::isFlightNetworkConnected
        virtual void setFlightNetworkConnected(bool connected);

        //! Is the flight network connected
        bool isFlightNetworkConnected() const { return m_networkConnected; }

        //! Reload weather settings
        void reloadWeatherSettings();

        //! Settings for current simulator
        BlackMisc::Simulation::Settings::CSimulatorSettings getSimulatorSettings() const { return m_settings.getSettings(this->getSimulatorInfo()); }

        //! Driver will be unloaded
        virtual void unload();

        //! Are we connected to the simulator?
        virtual bool isConnected() const = 0;

        //! Simulator paused?
        virtual bool isPaused() const = 0;

        //! Simulator running?
        virtual bool isSimulating() const { return this->isConnected(); }

        //! Clear all aircraft related data
        virtual void clearAllRemoteAircraftData();

        //! Debug function to check state after all aircraft have been removed
        //! \remarks only in local developer builds
        virtual BlackMisc::CStatusMessageList debugVerifyStateAfterAllAircraftRemoved() const;

        //! Is overall (swift) application shutting down
        //! \threadsafe
        virtual bool isShuttingDown() const { return (!sApp || sApp->isShuttingDown()); }

        //! Shutting down or disconnected?
        virtual bool isShuttingDownOrDisconnected() const { return (this->isShuttingDown() || !this->isConnected()); }

        //! \copydoc BlackMisc::Simulation::ISimulationEnvironmentProvider::requestElevation
        //! \remark needs to be overridden if the concrete driver supports such an option
        //! \sa ISimulator::callbackReceivedRequestedElevation
        virtual bool requestElevation(const BlackMisc::Geo::ICoordinateGeodetic &reference, const BlackMisc::Aviation::CCallsign &callsign) override;

        //! A requested elevation has been received
        //! \remark public for testing purposes
        virtual void callbackReceivedRequestedElevation(const BlackMisc::Geo::CElevationPlane &plane, const BlackMisc::Aviation::CCallsign &callsign);

        //! Allows to print out simulator specific statistics
        virtual QString getStatisticsSimulatorSpecific() const { return QString(); }

        //! Reset the statistics
        virtual void resetAircraftStatistics();

        //! \copydoc BlackMisc::IProvider::asQObject
        virtual QObject *asQObject() override { return this; }

        //! \addtogroup swiftdotcommands
        //! @{
        //! <pre>
        //! .drv unload                    unload plugin                           BlackCore::ISimulator
        //! .drv limit number              limit the number of updates             BlackCore::ISimulator
        //! .drv logint callsign           log interpolator for callsign           BlackCore::ISimulator
        //! .drv logint off                no log information for interpolator     BlackCore::ISimulator
        //! .drv logint write              write interpolator log to file          BlackCore::ISimulator
        //! .drv logint clear              clear current log                       BlackCore::ISimulator
        //! .drv pos callsign              shows current position in simulator     BlackCore::ISimulator
        //! .drv spline|linear callsign    interpolator spline or linear           BlackCore::ISimulator
        //! .drv aircraft readd callsign   re-add (add again) aircraft             BlackCore::ISimulator
        //! .drv aircraft readd all        re-add all aircraft                     BlackCore::ISimulator
        //! .drv aircraft rm callsign      remove aircraft                         BlackCore::ISimulator
        //! .drv    unload                 unload driver                           BlackCore::ISimulator
        //! .drv    fsuipc      on|off     enable/disable FSUIPC (if applicable)   BlackSimPlugin::FsCommon::CSimulatorFsCommon
        //! </pre>
        //! @}
        //! Parse command line for simulator drivers, derived classes can add specific parsing by overriding ISimulator::parseDetails
        virtual bool parseCommandLine(const QString &commandLine, const BlackMisc::CIdentifier &originator);

        //! Consolidate setup with other data like from BlackMisc::Simulation::IRemoteAircraftProvider
        //! \threadsafe
        BlackMisc::Simulation::CInterpolationAndRenderingSetupPerCallsign getInterpolationSetupConsolidated(const BlackMisc::Aviation::CCallsign &callsign) const;

        //! \copydoc BlackMisc::Simulation::IInterpolationSetupProvider::setInterpolationSetupGlobal
        virtual bool setInterpolationSetupGlobal(const BlackMisc::Simulation::CInterpolationAndRenderingSetupGlobal &setup) override;

        //!  Counter added aircraft
        int getStatisticsPhysicallyAddedAircraft() const { return m_statsPhysicallyAddedAircraft; }

        //!  Counter removed aircraft
        int getStatisticsPhysicallyRemovedAircraft() const { return m_statsPhysicallyRemovedAircraft; }

        //! Current update time in ms
        double getStatisticsCurrentUpdateTimeMs() const { return m_statsCurrentUpdateTimeMs; }

        //! Average update time in ms
        double getStatisticsAverageUpdateTimeMs() const { return m_statsUpdateAircraftTimeAvgMs; }

        //! Total update time in ms
        qint64 getStatisticsTotalUpdateTimeMs() const { return m_statsUpdateAircraftTimeTotalMs; }

        //! Max.update time in ms
        qint64 getStatisticsMaxUpdateTimeMs() const { return m_statsMaxUpdateTimeMs; }

        //! Number of update runs
        int getStatisticsUpdateRuns() const { return m_statsUpdateAircraftRuns; }

        //! Time between two update requests
        qint64 getStatisticsAircraftUpdatedRequestedDeltaMs() const { return m_statsUpdateAircraftRequestedDeltaMs; }

        //! The traced loopback situations
        BlackMisc::Aviation::CAircraftSituationList getLoopbackSituations(const BlackMisc::Aviation::CCallsign &callsign) const;

        //! Access to logger
        const BlackMisc::Simulation::CInterpolationLogger &interpolationLogger() const { return m_interpolationLogger; }

        //! The latest logged data formatted
        //! \remark public only for log. displays
        QString latestLoggedDataFormatted(const BlackMisc::Aviation::CCallsign &cs) const;

        //! Info about update aircraft limitations
        QString updateAircraftLimitationInfo() const;

        //! Reset the last sent values
        void resetLastSentValues();

        //! Reset the last sent values per callsign
        void resetLastSentValues(const BlackMisc::Aviation::CCallsign &callsign);

        //! Register help
        static void registerHelp();

        //! Status to string
        static QString statusToString(SimulatorStatus status);

        //! Any connected status?
        static bool isAnyConnectedStatus(SimulatorStatus status);

    signals:
        //! Simulator combined status
        void simulatorStatusChanged(SimulatorStatus status);

        //! Emitted when own aircraft model has changed
        void ownAircraftModelChanged(const BlackMisc::Simulation::CAircraftModel &model);

        //! Render restrictions have been changed
        void renderRestrictionsChanged(bool restricted, bool enabled, int maxAircraft, const BlackMisc::PhysicalQuantities::CLength &maxRenderedDistance);

        //! Interpolation or rendering setup changed
        void interpolationAndRenderingSetupChanged();

        //! Aircraft rendering changed
        void aircraftRenderingChanged(const BlackMisc::Simulation::CSimulatedAircraft &aircraft);

        //! Adding the remote model failed
        void physicallyAddingRemoteModelFailed(const BlackMisc::Simulation::CSimulatedAircraft &remoteAircraft, const BlackMisc::CStatusMessage &message);

        //! An airspace snapshot was handled
        void airspaceSnapshotHandled();

        //! Relevant simulator messages to be explicitly displayed
        void driverMessages(const BlackMisc::CStatusMessageList &messages);

        //! Request a console message (whatever the console maybe)
        void requestUiConsoleMessage(const QString &driverMessage, bool clear);

        //! Requested elevation, call pending
        void requestedElevation(const BlackMisc::Aviation::CCallsign &callsign);

        //! A requested elevation has been received
        void receivedRequestedElevation(const BlackMisc::Geo::CElevationPlane &plane, const BlackMisc::Aviation::CCallsign &callsign);

    protected:
        //! Constructor with all the providers
        ISimulator(const BlackMisc::Simulation::CSimulatorPluginInfo &pluginInfo,
                   BlackMisc::Simulation::IOwnAircraftProvider *ownAircraftProvider,
                   BlackMisc::Simulation::IRemoteAircraftProvider *remoteAircraftProvider,
                   BlackMisc::Weather::IWeatherGridProvider *weatherGridProvider,
                   BlackMisc::Network::IClientProvider *clientProvider,
                   QObject *parent = nullptr);

        //! \name When swift DB data are read
        //! @{
        virtual void onSwiftDbAllDataRead();
        virtual void onSwiftDbModelMatchingEntitiesRead();
        virtual void onSwiftDbAirportsRead();
        //! @}

        //! Init the internals info
        virtual void initSimulatorInternals();

        //! Parsed in derived classes
        virtual bool parseDetails(const BlackMisc::CSimpleCommandParser &parser) = 0;

        //! Airports from web services
        BlackMisc::Aviation::CAirportList getWebServiceAirports() const;

        //! Airport from web services by ICAO code
        BlackMisc::Aviation::CAirport getWebServiceAirport(const BlackMisc::Aviation::CAirportIcaoCode &icao) const;

        //! Max.airports in range
        int maxAirportsInRange() const;

        //! \name Connected with remote aircraft provider signals
        //! @{
        //! Recalculate the rendered aircraft, this happens when restrictions are applied (max. aircraft, range)
        virtual void onRecalculatedRenderedAircraft(const BlackMisc::Simulation::CAirspaceAircraftSnapshot &snapshot);
        //! @}

        //! Add new remote aircraft physically to the simulator
        //! \sa changeRemoteAircraftEnabled to hide a remote aircraft
        virtual bool physicallyAddRemoteAircraft(const BlackMisc::Simulation::CSimulatedAircraft &remoteAircraft) = 0;

        //! Remove remote aircraft from simulator
        virtual bool physicallyRemoveRemoteAircraft(const BlackMisc::Aviation::CCallsign &callsign) = 0;

        //! Remove remote aircraft from simulator
        virtual int physicallyRemoveMultipleRemoteAircraft(const BlackMisc::Aviation::CCallsignSet &callsigns);

        //! Remove all remote aircraft
        virtual int physicallyRemoveAllRemoteAircraft() = 0;

        //! Set elevation and CG in the providers
        void rememberElevationAndCG(const BlackMisc::Aviation::CCallsign &callsign, const QString &modelString, const BlackMisc::Geo::CElevationPlane &elevation, const BlackMisc::PhysicalQuantities::CLength &cg);

        //! Emit the combined status
        //! \param oldStatus optionally one can capture and provide the old status for comparison. In case of equal status values no signal will be sent
        //! \sa simulatorStatusChanged;
        void emitSimulatorCombinedStatus(SimulatorStatus oldStatus = Unspecified);

        //! \copydoc BlackMisc::Simulation::IInterpolationSetupProvider::emitInterpolationSetupChanged
        virtual void emitInterpolationSetupChanged() override;

        //! Display a debug log message based on BlackMisc::Simulation::CInterpolationAndRenderingSetup
        //! remark shows log messages of functions calls
        void debugLogMessage(const QString &msg);

        //! Display a debug log message based on BlackMisc::Simulation::CInterpolationAndRenderingSetup
        //! remark shows log messages of functions calls
        void debugLogMessage(const QString &funcInfo, const QString &msg);

        //! Show log messages?
        bool showDebugLogMessage() const;

        //! Restore aircraft from the provider data
        void resetAircraftFromProvider(const BlackMisc::Aviation::CCallsign &callsign);

        //! Clear the related data as statistics etc.
        virtual void clearData(const BlackMisc::Aviation::CCallsign &callsign);

        //! Add a loopback situation if logging is enabled
        bool addLoopbackSituation(const BlackMisc::Aviation::CAircraftSituation &situation);

        //! Add a loopback situation if logging is enabled
        bool addLoopbackSituation(const BlackMisc::Aviation::CCallsign &callsign, const BlackMisc::Geo::CElevationPlane &elevationPlane, const BlackMisc::PhysicalQuantities::CLength &cg);

        //! Full reset of state
        //! \remark reset as it was unloaded without unloading
        //! \sa ISimulator::clearAllRemoteAircraftData
        virtual void reset();

        //! Reset highlighting
        void resetHighlighting();

        //! Restore all highlighted aircraft
        void stopHighlighting();

        //! Slow timer used to highlight aircraft, can be used for other things too
        virtual void oneSecondTimerTimeout();

        //! Kill timer if id is valid
        void safeKillTimer();

        //! Inject weather grid to simulator
        virtual void injectWeatherGrid(const BlackMisc::Weather::CWeatherGrid &weatherGrid);

        //! Blink the highlighted aircraft
        void blinkHighlightedAircraft();

        //! Equal to last sent situation
        bool isEqualLastSent(const BlackMisc::Aviation::CAircraftSituation &compare) const;

        //! Equal to last sent situation
        bool isEqualLastSent(const BlackMisc::Aviation::CAircraftParts &compare, const BlackMisc::Aviation::CCallsign &callsign) const;

        //! Remember as last sent
        void rememberLastSent(const BlackMisc::Aviation::CAircraftSituation &sent);

        //! Remember as last sent
        void rememberLastSent(const BlackMisc::Aviation::CAircraftParts &sent, const BlackMisc::Aviation::CCallsign &callsign);

        //! Last sent situations
        BlackMisc::Aviation::CAircraftSituationList getLastSentCanLikelySkipNearGroundInterpolation() const;

        //! Limit reached (max number of updates by token bucket if enabled)
        bool isUpdateAircraftLimited(qint64 timestamp = -1);

        //! Limited as ISimulator::isUpdateAircraftLimited plus updating statistics
        bool isUpdateAircraftLimitedWithStats(qint64 startTime = -1);

        //! Limit to updates per second
        bool limitToUpdatesPerSecond(int numberPerSecond);

        //! Set own model
        void reverseLookupAndUpdateOwnAircraftModel(const BlackMisc::Simulation::CAircraftModel &model);

        //! Set own model
        void reverseLookupAndUpdateOwnAircraftModel(const QString &modelString);

        //! Info about invalid situation
        QString getInvalidSituationLogMessage(const BlackMisc::Aviation::CCallsign &callsign, const BlackMisc::Simulation::CInterpolationStatus &status, const QString &details = {}) const;

        //! Can a new log message be generated without generating a "message" overflow
        //! \remark works per callsign
        //! \remark use this function when there is a risk that a lot of log. messages will be generated in a short time
        bool clampedLog(const BlackMisc::Aviation::CCallsign &callsign, const BlackMisc::CStatusMessage &message);

        //! Mark as justed logged
        //! \remark touch, but also return if it can be logged
        //! \remark use this function when there is a risk that a lot of log. messages will be generated in a short time
        void removedClampedLog(const BlackMisc::Aviation::CCallsign &callsign);

        //! Update stats and flags
        void setStatsRemoteAircraftUpdate(qint64 startTime, bool limited = false);

        //! Lookup against DB data
        static BlackMisc::Simulation::CAircraftModel reverseLookupModel(const BlackMisc::Simulation::CAircraftModel &model);

        bool   m_pausedSimFreezesInterpolation = false;   //!< paused simulator will also pause interpolation (so AI aircraft will hold)
        bool   m_autoCalcAirportDistance = true;          //!< automatically calculate airport distance and bearing
        bool   m_updateRemoteAircraftInProgress = false;  //!< currently updating remote aircraft
        int    m_timerId = -1;                            //!< dispatch timer id
        int    m_statsUpdateAircraftRuns = 0;             //!< statistics update count
        int    m_statsUpdateAircraftLimited = 0;          //!< skipped because of max.update limitations
        double m_statsUpdateAircraftTimeAvgMs = 0;        //!< statistics average update time
        qint64 m_statsUpdateAircraftTimeTotalMs = 0;      //!< statistics total update time
        qint64 m_statsCurrentUpdateTimeMs = 0;            //!< statistics current update time
        qint64 m_statsMaxUpdateTimeMs = 0;                //!< statistics max.update time
        qint64 m_statsLastUpdateAircraftRequestedMs  = 0; //!< when was the last aircraft update requested
        qint64 m_statsUpdateAircraftRequestedDeltaMs = 0; //!< delta time between 2 aircraft updates

        BlackMisc::Simulation::CSimulatorInternals  m_simulatorInternals;  //!< setup object
        BlackMisc::Simulation::CInterpolationLogger m_interpolationLogger; //!< log.interpolation
        BlackMisc::Aviation::CTimestampPerCallsign  m_clampedLogMsg;       //!< when logged last for this callsign, can be used so there is no log message overflow
        BlackMisc::Aviation::CAircraftSituationPerCallsign m_lastSentSituations; //!< last situation sent to simulator
        BlackMisc::Aviation::CAircraftPartsPerCallsign     m_lastSentParts;      //!< last parts sent to simulator

        // some optional functionality which can be used by the simulators as needed
        BlackMisc::Simulation::CSimulatedAircraftList m_addAgainAircraftWhenRemoved; //!< add this model again when removed, normally used to change model

        // loopback situations, situations which are received from simulator for remote aircraft
        BlackMisc::Aviation::CAircraftSituationListPerCallsign m_loopbackSituations; //!< traced loopback situations

        // limit the update aircraft to a maximum per second
        BlackMisc::CTokenBucket m_limitUpdateAircraftBucket { 5, 100, 5 }; //!< means 50 per second
        bool m_limitUpdateAircraft = false; //!< limit the update frequency by using BlackMisc::CTokenBucket

        // general settings
        BlackMisc::Simulation::Settings::CMultiSimulatorSettings m_settings { this }; //!< simulator settings for all simulators

        // weather
        bool m_isWeatherActivated = false;                         //!< Is simulator weather activated?
        BlackMisc::Geo::CCoordinateGeodetic m_lastWeatherPosition; //!< Own aircraft position at which weather was fetched and injected last
        BlackMisc::CSetting<BlackMisc::Simulation::Settings::TSelectedWeatherScenario> m_weatherScenarioSettings { this, &ISimulator::reloadWeatherSettings }; //!< Selected weather scenario

    private:
        // remote aircraft provider ("rap") bound
        void rapOnRecalculatedRenderedAircraft(const BlackMisc::Simulation::CAirspaceAircraftSnapshot &snapshot);
        void rapOnRemoteProviderRemovedAircraft(const BlackMisc::Aviation::CCallsign &callsign);

        // call with counters updated
        void callPhysicallyAddRemoteAircraft(const BlackMisc::Simulation::CSimulatedAircraft &remoteAircraft);
        void callPhysicallyRemoveRemoteAircraft(const BlackMisc::Aviation::CCallsign &remoteCallsign, bool blinking = false);

        //! Display a logged situation in simulator
        void displayLoggedSituationInSimulator(const BlackMisc::Aviation::CCallsign &cs, bool stopLogging, int times = 40);

        // statistics values of how often those functions are called
        // those are the added counters, overflow will not be an issue here (discussed in T171 review)
        int m_statsPhysicallyAddedAircraft = 0;   //!< statistics, how many aircraft added
        int m_statsPhysicallyRemovedAircraft = 0; //!< statistics, how many aircraft removed

        // highlighting
        bool m_blinkCycle = false;                //!< used for highlighting
        qint64 m_highlightEndTimeMsEpoch = 0;     //!< end highlighting
        BlackMisc::Simulation::CSimulatedAircraftList m_highlightedAircraft; //!< all other aircraft are to be ignored

        // timer
        int m_timerCounter = 0;                   //!< allows to calculate n seconds
        QTimer m_oneSecondTimer;                  //!< multi purpose timer with 1 sec. interval

        // misc. as callsigns
        bool   m_networkConnected = false;        //!< flight network connected
        BlackMisc::Aviation::CCallsignSet m_callsignsToBeRendered;             //!< callsigns which will be rendered
        BlackMisc::CConnectionGuard       m_remoteAircraftProviderConnections; //!< connected signal/slots
    };

    //! \brief Interface to a simulator listener.
    //! \details The simulator listener is responsible for letting the core know,
    //!          when the corresponding simulator is started.
    //! \note Will be moved to a background thread (context)
    class BLACKCORE_EXPORT ISimulatorListener : public QObject
    {
        Q_OBJECT

    public:
        //! Destructor
        virtual ~ISimulatorListener() = default;

        //! Corresponding info
        const BlackMisc::Simulation::CSimulatorPluginInfo &getPluginInfo() const { return m_info; }

        //! Info about the backend system (if available)
        virtual QString backendInfo() const;

        //! Running?
        bool isRunning() const { return m_isRunning; }

    public slots:
        //! Start listening for the simulator to start.
        //! \threadsafe
        void start();

        //! Stops listening.
        //! \threadsafe
        void stop();

        //! Check simulator availability
        //! \threadsafe
        void check();

    signals:
        //! Emitted when the listener discovers the simulator running.
        void simulatorStarted(const BlackMisc::Simulation::CSimulatorPluginInfo &info);

    protected:
        //! Constructor
        //! \sa ISimulatorFactory::createListener().
        ISimulatorListener(const BlackMisc::Simulation::CSimulatorPluginInfo &info);

        //! Overall (swift) application shutting down
        virtual bool isShuttingDown() const;

        //! Plugin specific implementation to start listener
        virtual void startImpl() = 0;

        //! Plugin specific implementation to stop listener
        virtual void stopImpl() = 0;

        //! Plugin specific implementation to check
        virtual void checkImpl() = 0;

    private:
        BlackMisc::Simulation::CSimulatorPluginInfo m_info;
        std::atomic_bool m_isRunning { false };
    };

    //! Factory pattern class to create instances of ISimulator
    class BLACKCORE_EXPORT ISimulatorFactory
    {
    public:
        //! ISimulatorVirtual destructor
        virtual ~ISimulatorFactory() {}

        //! Not copyable @{
        ISimulatorFactory(const ISimulatorFactory &) = delete;
        ISimulatorFactory &operator=(const ISimulatorFactory &) = delete;
        //! @}

        //! Create a new instance of a driver
        //! \param info                      metadata about simulator
        //! \param ownAircraftProvider       in memory access to own aircraft data
        //! \param remoteAircraftProvider    in memory access to rendered aircraft data such as situation history and aircraft itself
        //! \param weatherGridProvider       in memory access to weather grid data
        //! \param clientProvider            in memory access to client data
        //! \return driver instance
        //!
        virtual ISimulator *create(
            const BlackMisc::Simulation::CSimulatorPluginInfo &info,
            BlackMisc::Simulation::IOwnAircraftProvider *ownAircraftProvider,
            BlackMisc::Simulation::IRemoteAircraftProvider *remoteAircraftProvider,
            BlackMisc::Weather::IWeatherGridProvider *weatherGridProvider,
            BlackMisc::Network::IClientProvider *clientProvider) = 0;

        //! Simulator listener instance
        virtual ISimulatorListener *createListener(const BlackMisc::Simulation::CSimulatorPluginInfo &info) = 0;

    protected:
        //! Default ctor
        ISimulatorFactory() {}
    };
} // namespace

Q_DECLARE_INTERFACE(BlackCore::ISimulator, "org.swift-project.blackcore.simulator")
Q_DECLARE_INTERFACE(BlackCore::ISimulatorFactory, "org.swift-project.blackcore.simulatorfactory")
Q_DECLARE_OPERATORS_FOR_FLAGS(BlackCore::ISimulator::SimulatorStatus)

#endif // guard
