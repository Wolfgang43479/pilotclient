/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_COMPONENTS_MODELMATCHERLOGENABLE_H
#define BLACKGUI_COMPONENTS_MODELMATCHERLOGENABLE_H

#include "blackcore/network.h"
#include <QFrame>
#include <QScopedPointer>

namespace Ui { class CModelMatcherLogEnable; }
namespace BlackGui
{
    namespace Components
    {
        //! Enable/disable matching logs
        class CModelMatcherLogEnable : public QFrame
        {
            Q_OBJECT

        public:
            //! Constructor
            explicit CModelMatcherLogEnable(QWidget *parent = nullptr);

            //! Destructor
            virtual ~CModelMatcherLogEnable();

            //! Reverse lookup enabled?
            bool isReverseLookupLogEnabled() const;

            //! Matching log.enabled
            bool isMatchingLogEnabled() const;

        private:
            QScopedPointer<Ui::CModelMatcherLogEnable> ui;

            //! Flag changed
            void enabledCheckboxChanged(bool enabled);

            //! Init GUI
            void initGui();

            //! Has required context
            bool hasContexts() const;

            //! When values changed elsewhere
            void valuesChanged();

            //! Connection status has been changed
            void connectionStatusChanged(BlackCore::INetwork::ConnectionStatus from, BlackCore::INetwork::ConnectionStatus to);
        };
    } // ns
} // ns

#endif // guard
