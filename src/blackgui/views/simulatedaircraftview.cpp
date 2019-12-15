/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "simulatedaircraftview.h"
#include "flightplandialog.h"
#include "blackgui/models/simulatedaircraftlistmodel.h"
#include "blackgui/menus/menuaction.h"
#include "blackgui/guiapplication.h"
#include "blackcore/context/contextsimulator.h"
#include "blackcore/context/contextnetwork.h"
#include "blackmisc/simulation/simulatedaircraftlist.h"
#include "blackmisc/aviation/callsign.h"
#include "blackmisc/icons.h"
#include "blackconfig/buildconfig.h"

#include <QStringBuilder>
#include <QPointer>
#include <QtGlobal>

using namespace BlackConfig;
using namespace BlackMisc;
using namespace BlackMisc::Aviation;
using namespace BlackMisc::Simulation;
using namespace BlackCore::Context;
using namespace BlackGui::Models;
using namespace BlackGui::Menus;

namespace BlackGui
{
    namespace Views
    {
        CSimulatedAircraftView::CSimulatedAircraftView(QWidget *parent) : CViewWithCallsignObjects(parent)
        {
            this->standardInit(new CSimulatedAircraftListModel(this));
            m_menus |= MenuRefresh;
        }

        void CSimulatedAircraftView::setAircraftMode(CSimulatedAircraftListModel::AircraftMode mode)
        {
            Q_ASSERT(m_model);
            m_model->setAircraftMode(mode);
            this->setSortIndicator();
        }

        void CSimulatedAircraftView::configureMenu(bool menuRecalculate, bool menuHighlightAndFollow, bool menuEnableAircraft, bool menuFastPositionUpdates, bool menuGndFlag, bool menuFlightPlan)
        {
            m_withRecalculate            = menuRecalculate;
            m_withMenuEnableAircraft     = menuEnableAircraft;
            m_withMenuFastPosition       = menuFastPositionUpdates;
            m_withMenuHighlightAndFollow = menuHighlightAndFollow;
            m_withMenuEnableGndFlag      = menuGndFlag;
            m_withMenuFlightPlan         = menuFlightPlan;
        }

        void CSimulatedAircraftView::configureMenuFastPositionUpdates(bool menuFastPositionUpdates)
        {
            m_withMenuFastPosition = menuFastPositionUpdates;
        }

        void CSimulatedAircraftView::customMenu(CMenuActions &menuActions)
        {
            // sub menus, only adding the path, the content will be added below
            // => configureMenu
            menuActions.addMenuDisplayModels();
            menuActions.addMenuDataTransfer();

            if (m_menus.testFlag(MenuDisableModelsTemp) && this->hasSelection())
            {
                menuActions.addAction(CIcons::delete16(), "Temp.disable model from set", CMenuAction::pathClientSimulationDisplay(), { this, &CSimulatedAircraftView::requestTempDisable });
            }

            if (m_withRecalculate)
            {
                menuActions.addAction(CIcons::appInterpolation16(), "Re-calculate all aircraft", CMenuAction::pathClientSimulationDisplay(), { this, &CSimulatedAircraftView::recalculateAllAircraft });
                menuActions.addAction(CIcons::appInterpolation16(), "Re-matching all aircraft", CMenuAction::pathClientSimulationDisplay(), { this, &CSimulatedAircraftView::doMatchingsAgain });
                if (this->hasSelection())
                {
                    menuActions.addAction(CIcons::appInterpolation16(), "Re-matching selected", CMenuAction::pathClientSimulationDisplay(), { this, &CSimulatedAircraftView::doMatchingsAgainForSelected });
                }
            }

            if (m_withMenuEnableAircraft && !this->isEmpty())
            {
                menuActions.addAction(CIcons::appAircraft16(), "Enable all aircraft", CMenuAction::pathClientSimulationDisplay(), { this, &CSimulatedAircraftView::enableAllDisabledAircraft });
                menuActions.addAction(CIcons::appAircraft16(), "Re-enable unrendered aircraft", CMenuAction::pathClientSimulationDisplay(), { this, &CSimulatedAircraftView::reEnableAllUnrenderedAircraft });
                menuActions.addAction(CIcons::appAircraft16(), "Disable all aircraft", CMenuAction::pathClientSimulationDisplay(), { this, &CSimulatedAircraftView::disableAllEnabledAircraft });
            }

            if (this->hasSelection())
            {
                const bool hasContexts = sGui && sGui->supportsContexts();
                if (hasContexts)
                {
                    const CSimulatedAircraft aircraft(this->selectedObject());
                    Q_ASSERT(!aircraft.getCallsign().isEmpty());
                    menuActions.addMenuCom();
                    menuActions.addAction(CIcons::appTextMessages16(), "Show text messages", CMenuAction::pathClientCom(), { this, &CSimulatedAircraftView::requestTextMessage });

                    if (m_withMenuFlightPlan && networkContext() && networkContext()->isConnected())
                    {
                        menuActions.addAction(CIcons::appFlightPlan16(), "Flight plan",  CMenuAction::pathClientNetwork(), { this, &CSimulatedAircraftView::showFlightPlanDialog });
                    }
                    if (m_withMenuEnableAircraft)
                    {
                        menuActions.addAction(CIcons::appAircraft16(), aircraft.isEnabled() ? "Disable aircraft" : "Enabled aircraft", CMenuAction::pathClientSimulationDisplay(), { this, &CSimulatedAircraftView::toggleEnabledAircraft });
                    }
                    if (m_withMenuHighlightAndFollow)
                    {
                        menuActions.addAction(CIcons::appAircraft16(), "Follow in simulator", CMenuAction::pathClientSimulationDisplay(), { this, &CSimulatedAircraftView::requestFollowInSimulator });
                        menuActions.addAction(CIcons::appSimulator16(), "Highlight in simulator", CMenuAction::pathClientSimulationDisplay(), { this, &CSimulatedAircraftView::requestHighlightInSimulator });
                    }
                    if (m_withMenuEnableGndFlag)
                    {
                        menuActions.addAction(CIcons::geoPosition16(), aircraft.isSupportingGndFlag() ? "Disable gnd.flag" : "Enabled gnd.flag", CMenuAction::pathClientSimulationTransfer(), { this, &CSimulatedAircraftView::toggleSupportingGndFlag });
                    }
                    if (m_withMenuFastPosition)
                    {
                        menuActions.addAction(CIcons::globe16(), aircraft.fastPositionUpdates() ? "Normal updates" : "Fast position updates (send)",  CMenuAction::pathClientSimulationTransfer(), { this, &CSimulatedAircraftView::toggleFastPositionUpdates });
                    }

                    const bool any = m_withMenuEnableAircraft || m_withMenuFastPosition || m_withMenuHighlightAndFollow || m_withMenuEnableGndFlag || m_withMenuFlightPlan;
                    if (any && (sApp && sApp->isDeveloperFlagSet()))
                    {
                        menuActions.addAction(CIcons::appSimulator16(), "Show position log.", CMenuAction::pathClientSimulationDisplay(), { this, &CSimulatedAircraftView::showPositionLogInSimulator });
                    }
                } // contexts
            }
            CViewBase::customMenu(menuActions);
        }

        void CSimulatedAircraftView::requestTextMessage()
        {
            const CSimulatedAircraft aircraft(this->selectedObject());
            if (aircraft.getCallsign().isEmpty()) { return; }
            emit this->requestTextMessageWidget(aircraft.getCallsign());
        }

        void CSimulatedAircraftView::toggleEnabledAircraft()
        {
            CSimulatedAircraft aircraft(this->selectedObject());
            if (aircraft.getCallsign().isEmpty()) { return; }
            aircraft.setEnabled(!aircraft.isEnabled());
            this->updateAircraftEnabled(aircraft);
        }

        void CSimulatedAircraftView::toggleFastPositionUpdates()
        {
            CSimulatedAircraft aircraft(this->selectedObject());
            if (aircraft.getCallsign().isEmpty()) { return; }
            aircraft.toggleFastPositionUpdates();
            this->enableFastPositionUpdates(aircraft);
        }

        void CSimulatedAircraftView::toggleSupportingGndFlag()
        {
            CSimulatedAircraft aircraft(this->selectedObject());
            if (aircraft.getCallsign().isEmpty()) { return; }
            aircraft.setSupportingGndFlag(!aircraft.isSupportingGndFlag());
            this->updateAircraftSupportingGndFLag(aircraft);
        }

        void CSimulatedAircraftView::requestHighlightInSimulator()
        {
            const CSimulatedAircraft aircraft(this->selectedObject());
            if (aircraft.getCallsign().isEmpty()) { return; }
            this->highlightInSimulator(aircraft);
        }

        void CSimulatedAircraftView::requestFollowInSimulator()
        {
            const CSimulatedAircraft aircraft(this->selectedObject());
            if (aircraft.getCallsign().isEmpty()) { return; }
            this->followAircraftInSimulator(aircraft);
        }

        void CSimulatedAircraftView::requestTempDisable()
        {
            if (!m_menus.testFlag(MenuDisableModelsTemp)) { return; }
            if (!this->hasSelection()) { return; }
            const CAircraftModelList models(this->selectedObjects().getModels());
            emit this->requestTempDisableModelsForMatching(models);
            sGui->displayInStatusBar(CStatusMessage(CStatusMessage::SeverityInfo, u"Temp.disabled " % models.getModelStringList(true).join(" ")));
        }

        void CSimulatedAircraftView::showPositionLogInSimulator()
        {
            IContextSimulator *simContext = simulatorContext();
            if (!simContext) { return; }
            const CSimulatedAircraft aircraft(this->selectedObject());
            if (aircraft.getCallsign().isEmpty()) { return; }

            const CIdentifier i(this->objectName());
            const QString dotCmd(".drv pos " + aircraft.getCallsignAsString());
            simContext->parseCommandLine(dotCmd, i);
        }

        void CSimulatedAircraftView::enableAllDisabledAircraft()
        {
            if (!sGui || sGui->isShuttingDown())  { return; }
            const CSimulatedAircraftList aircraft = this->container().findByEnabled(false);
            this->enableOrDisableAircraft(aircraft, true);
        }

        void CSimulatedAircraftView::disableAllEnabledAircraft()
        {
            if (!sGui || sGui->isShuttingDown())  { return; }
            const CSimulatedAircraftList aircraft = this->container().findByEnabled(true);
            this->enableOrDisableAircraft(aircraft, false);
        }

        void CSimulatedAircraftView::reEnableAllUnrenderedAircraft()
        {
            if (!sGui || sGui->isShuttingDown())  { return; }
            const CSimulatedAircraftList aircraft = this->container().findByRendered(false);
            this->enableOrDisableAircraft(aircraft, true);
        }

        void CSimulatedAircraftView::enableOrDisableAircraft(const CSimulatedAircraftList &aircraft, bool newEnabled)
        {
            if (aircraft.isEmpty())  { return; }

            const QPointer<CSimulatedAircraftView> myself(this);
            for (const CSimulatedAircraft &sa : aircraft)
            {
                QTimer::singleShot(10, this, [ = ]
                {
                    if (!myself) { return; }
                    if (!sGui || sGui->isShuttingDown()) { return; }
                    CSimulatedAircraft enabledAircraft(sa);
                    enabledAircraft.setEnabled(newEnabled);
                    this->updateAircraftEnabled(enabledAircraft);
                });
            }
        }

        void CSimulatedAircraftView::followAircraftInSimulator(const CSimulatedAircraft &aircraft)
        {
            IContextSimulator *simContext = simulatorContext();
            if (!simContext) { return; }
            simContext->followAircraft(aircraft.getCallsign());
        }

        void CSimulatedAircraftView::highlightInSimulator(const CSimulatedAircraft &aircraft)
        {
            IContextSimulator *simContext = simulatorContext();
            if (!simContext) { return; }
            simContext->highlightAircraft(aircraft, true, IContextSimulator::HighlightTime());
        }

        void CSimulatedAircraftView::recalculateAllAircraft()
        {
            IContextSimulator *simContext = simulatorContext();
            if (!simContext) { return; }
            simContext->recalculateAllAircraft();
        }

        void CSimulatedAircraftView::doMatchingsAgain()
        {
            IContextSimulator *simContext = simulatorContext();
            if (!simContext) { return; }
            const int rematchedNumber = simContext->doMatchingsAgain();
            CLogMessage(this).info(u"Triggered re-matching of %1 aircraft") << rematchedNumber;
        }

        void CSimulatedAircraftView::doMatchingsAgainForSelected()
        {
            IContextSimulator *simContext = simulatorContext();
            if (!simContext) { return; }
            if (!this->hasSelection()) { return; }
            const CCallsign cs = this->selectedObject().getCallsign();
            if (simContext->doMatchingAgain(cs))
            {
                CLogMessage(this).info(u"Triggered re-matching of '%1'") << cs.asString();
            }
        }

        void CSimulatedAircraftView::enableFastPositionUpdates(const CSimulatedAircraft &aircraft)
        {
            IContextNetwork *nwContext = networkContext();
            if (!nwContext) { return; }
            nwContext->updateFastPositionEnabled(aircraft.getCallsign(), aircraft.fastPositionUpdates());
        }

        void CSimulatedAircraftView::updateAircraftEnabled(const CSimulatedAircraft &aircraft)
        {
            IContextNetwork *nwContext = networkContext();
            if (!nwContext) { return; }
            nwContext->updateAircraftEnabled(aircraft.getCallsign(), aircraft.isEnabled());
        }

        void CSimulatedAircraftView::updateAircraftSupportingGndFLag(const CSimulatedAircraft &aircraft)
        {
            IContextNetwork *nwContext = networkContext();
            if (!nwContext) { return; }
            nwContext->updateAircraftSupportingGndFLag(aircraft.getCallsign(), aircraft.isSupportingGndFlag());
        }

        void CSimulatedAircraftView::showFlightPlanDialog()
        {
            if (!m_withMenuFlightPlan) { return; }
            if (!networkContext() || !networkContext()->isConnected()) { return; }

            const CSimulatedAircraft aircraft = this->selectedObject();
            if (!aircraft.hasCallsign()) { return; }

            const CCallsign cs = aircraft.getCallsign();
            if (!m_fpDialog) { m_fpDialog = new CFlightPlanDialog(this); }
            m_fpDialog->showFlightPlan(cs);
        }

        IContextSimulator *CSimulatedAircraftView::simulatorContext()
        {
            if (!sGui || sGui->isShuttingDown() || !sGui->getIContextSimulator()) { return nullptr; }
            return sGui->getIContextSimulator();
        }

        IContextNetwork *CSimulatedAircraftView::networkContext()
        {
            if (!sGui || sGui->isShuttingDown() || !sGui->getIContextNetwork()) { return nullptr; }
            return sGui->getIContextNetwork();
        }
    } // ns
} // ns
