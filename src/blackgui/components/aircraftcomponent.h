/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_AIRCRAFTCOMPONENT_H
#define BLACKGUI_AIRCRAFTCOMPONENT_H

#include "blackcore/network.h"
#include "blackgui/blackguiexport.h"
#include "blackgui/settings/viewupdatesettings.h"
#include "blackgui/components/enablefordockwidgetinfoarea.h"

#include <QObject>
#include <QScopedPointer>
#include <QTabWidget>
#include <QtGlobal>
#include <QTimer>

namespace BlackMisc
{
    namespace Aviation { class CCallsign; }
    namespace Simulation { class CSimulatedAircraft; }
}
namespace Ui { class CAircraftComponent; }
namespace BlackGui
{
    class CDockWidgetInfoArea;
    namespace Components
    {
        //! Aircraft widget
        class BLACKGUI_EXPORT CAircraftComponent :
            public QTabWidget,
            public CEnableForDockWidgetInfoArea
        {
            Q_OBJECT

        public:
            //! Tab
            //! \remark needs to be in sync with tab order
            enum AircraftTab
            {
                TabAircraftInRange = 0,
                TabAirportsInRange
            };

            //! Constructor
            explicit CAircraftComponent(QWidget *parent = nullptr);

            //! Destructor
            virtual ~CAircraftComponent() override;

            //! Aircraft in range
            int countAircraftInView() const;

            //! Airports in range
            int countAirportsInRangeInView() const;

            //! \copydoc CEnableForDockWidgetInfoArea::setParentDockWidgetInfoArea
            virtual bool setParentDockWidgetInfoArea(BlackGui::CDockWidgetInfoArea *parentDockableWidget) override;

            //! Update aircraft/airport view
            void update();

            //! Set tab
            void setTab(AircraftTab tab);

        signals:
            //! Request a text message
            void requestTextMessageWidget(const BlackMisc::Aviation::CCallsign &callsign);

        private:
            //! Info area tab bar has changed
            void onInfoAreaTabBarChanged(int index);

            //! Number of elements changed
            void onRowCountChanged(int count, bool withFilter);

            //! Connection status has been changed
            void onConnectionStatusChanged(BlackCore::INetwork::ConnectionStatus from, BlackCore::INetwork::ConnectionStatus to);

            //! Settings have been changed
            void onSettingsChanged();

            QScopedPointer<Ui::CAircraftComponent> ui;
            BlackMisc::CSettingReadOnly<BlackGui::Settings::TViewUpdateSettings> m_settings { this, &CAircraftComponent::onSettingsChanged }; //!< settings changed
            QTimer m_updateTimer;
            int m_updateCounter = 0;
        };
    } // ns
} // ns

#endif // guard
