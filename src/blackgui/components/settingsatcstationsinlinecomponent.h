/* Copyright (C) 2016
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_COMPONENTS_SETTINGSATCSTATIONSINLINECOMPONENT_H
#define BLACKGUI_COMPONENTS_SETTINGSATCSTATIONSINLINECOMPONENT_H

#include "blackgui/settings/atcstationssettings.h"
#include <QFrame>

namespace Ui { class CSettingsAtcStationsInlineComponent; }
namespace BlackGui
{
    namespace Components
    {
        /*!
         * How ATC stations will be displayed
         */
        class CSettingsAtcStationsInlineComponent : public QFrame
        {
            Q_OBJECT

        public:
            //! Constructor
            explicit CSettingsAtcStationsInlineComponent(QWidget *parent = nullptr);

            //! Destructor
            virtual ~CSettingsAtcStationsInlineComponent();

        private:
            //! Settings have been changed
            void settingsChanged();

            //! Change the settings
            void changeSettings();

            QScopedPointer<Ui::CSettingsAtcStationsInlineComponent> ui;
            BlackMisc::CSetting<BlackGui::Settings::TAtcStationsSettings> m_atcSettings { this, &CSettingsAtcStationsInlineComponent::settingsChanged };
        };
    } // ns
} // ns
#endif // guard
