/* Copyright (C) 2016
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_COMPONENTS_DBOWNMODELSETCOMPONENT_H
#define BLACKGUI_COMPONENTS_DBOWNMODELSETCOMPONENT_H

#include "blackgui/menus/menudelegate.h"
#include "blackmisc/simulation/aircraftmodellist.h"
#include "blackmisc/simulation/aircraftmodelsetloader.h"
#include "dbmappingcomponentaware.h"
#include <QFrame>
#include <QScopedPointer>
#include <QMenu>

namespace Ui { class CDbOwnModelSetComponent; }

namespace BlackGui
{
    namespace Views { class CAircraftModelView; }
    namespace Components
    {
        class CDbOwnModelSetDialog;

        /*!
         * Handling of the own model set
         */
        class CDbOwnModelSetComponent :
            public QFrame,
            public CDbMappingComponentAware
        {
            Q_OBJECT

        public:
            //! Constructor
            explicit CDbOwnModelSetComponent(QWidget *parent = nullptr);

            //! Destructor
            virtual ~CDbOwnModelSetComponent();

            //! Corresponding view
            Views::CAircraftModelView *view() const;

            //! Add to model set
            BlackMisc::CStatusMessage addToModelSet(const BlackMisc::Simulation::CAircraftModelList &models, const BlackMisc::Simulation::CSimulatorInfo &simulator);

            //! Add to model set
            BlackMisc::CStatusMessage addToModelSet(const BlackMisc::Simulation::CAircraftModel &model, const BlackMisc::Simulation::CSimulatorInfo &simulator);

            //! Current model set for simulator CDbOwnModelSetComponent::getModelSetSimulator
            const BlackMisc::Simulation::CAircraftModelList &getModelSet() const;

            //! Model set is for simulator
            const BlackMisc::Simulation::CSimulatorInfo &getModelSetSimulator() const { return m_simulator; }

            //! \copydoc CDbMappingComponentAware::setMappingComponent
            virtual void setMappingComponent(CDbMappingComponent *component) override;

        public slots:
            //! Set the model set
            void setModelSet(const BlackMisc::Simulation::CAircraftModelList &models, const BlackMisc::Simulation::CSimulatorInfo &simulator);

        private slots:
            //! Tab has been changed
            void ps_tabChanged(int index);

            //! Button was clicked
            void ps_buttonClicked();

            //! Change current simulator
            void ps_changeSimulator(const BlackMisc::Simulation::CSimulatorInfo &simulator);

            //! Simulator has been changed (in loader)
            void ps_onSimulatorChanged(const BlackMisc::Simulation::CSimulatorInfo &simulator);

            //! View has changed row count
            void ps_onRowCountChanged(int count, bool withFilter);

        private:
            //! Default file name
            void setSaveFileName(const BlackMisc::Simulation::CSimulatorInfo &sim);

            //! Simulator
            void setSimulator(const BlackMisc::Simulation::CSimulatorInfo &sim);

            QScopedPointer<Ui::CDbOwnModelSetComponent> ui;
            QScopedPointer<CDbOwnModelSetDialog>        m_modelSetDialog;
            BlackMisc::Simulation::CSimulatorInfo       m_simulator;
            BlackMisc::Simulation::CModelSetLoader      m_modelSetLoader { BlackMisc::Simulation::CSimulatorInfo(BlackMisc::Simulation::CSimulatorInfo::FSX), this };

            //! The menu for loading and handling own models for mapping tasks
            //! \note This is specific for that very component
            class CLoadModelsMenu : public BlackGui::Menus::IMenuDelegate
            {
            public:
                //! Constructor
                CLoadModelsMenu(CDbOwnModelSetComponent *ownModelSetComponent, bool separator = true) :
                    BlackGui::Menus::IMenuDelegate(ownModelSetComponent, separator)
                {}

                //! \copydoc IMenuDelegate::customMenu
                virtual void customMenu(QMenu &menu) const override;
            };
        };
    } // ns
} // ns

#endif // guard
