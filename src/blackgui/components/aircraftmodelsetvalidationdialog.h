/* Copyright (C) 2019
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_COMPONENTS_AIRCRAFTMODELSETVALIDATIONDIALOG_H
#define BLACKGUI_COMPONENTS_AIRCRAFTMODELSETVALIDATIONDIALOG_H

#include "blackgui/blackguiexport.h"

#include <QDialog>
#include <QScopedPointer>

namespace Ui { class CAircraftModelSetValidationDialog; }
namespace BlackMisc
{
    class CStatusMessageList;
    namespace Simulation
    {
        class CSimulatorInfo;
        class CAircraftModelList;
    }
}
namespace BlackGui
{
    namespace Components
    {
        /**
         * Dialog for CAircraftModelValidationComponent
         */
        class BLACKGUI_EXPORT CAircraftModelSetValidationDialog : public QDialog
        {
            Q_OBJECT

        public:
            //! Constructor
            explicit CAircraftModelSetValidationDialog(QWidget *parent = nullptr);

            //! Destructor
            virtual ~CAircraftModelSetValidationDialog();

            //! \copydoc CAircraftModelValidationComponent::validatedModelSet
            void validatedModelSet(const BlackMisc::Simulation::CSimulatorInfo &simulator, const BlackMisc::Simulation::CAircraftModelList &valid, const BlackMisc::Simulation::CAircraftModelList &invalid, bool stopped, const BlackMisc::CStatusMessageList &msgs);

        private:
            QScopedPointer<Ui::CAircraftModelSetValidationDialog> ui;
        };
    } // ms
} // ns

#endif // guard