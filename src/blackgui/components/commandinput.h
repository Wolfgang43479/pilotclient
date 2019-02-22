/* Copyright (C) 2015
 * swift project community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_COMPONENTS_COMMANDINPUT_H
#define BLACKGUI_COMPONENTS_COMMANDINPUT_H

#include "blackgui/lineedithistory.h"
#include "blackgui/blackguiexport.h"
#include "blackmisc/digestsignal.h"
#include "blackmisc/identifiable.h"
#include "blackmisc/identifier.h"

#include <QLineEdit>
#include <QObject>
#include <QString>

class QWidget;
namespace BlackMisc
{
    namespace Network { class CServer; }
    namespace Simulation { class CSimulatorPluginInfo; }
}
namespace BlackGui
{
    namespace Components
    {
        //! Specialized line edit for command inputs
        class BLACKGUI_EXPORT CCommandInput :
            public BlackGui::CLineEditHistory,
            public BlackMisc::CIdentifiable
        {
            Q_OBJECT

        public:
            //! Constructor
            CCommandInput(QWidget *parent = nullptr);

            //! Destructor
            virtual ~CCommandInput() {}

        signals:
            //! Command was entered
            void commandEntered(const QString &command, const BlackMisc::CIdentifier &originator);

            //! Text entered (which is not a command)
            void textEntered(const QString &command, const BlackMisc::CIdentifier &originator);

        private:
            //! Basic command validation
            void validateCommand();

            //! Command tooltip
            void setCommandTooltip();

            //! Simulator plugin loaded / unloaded (default info)
            void onSimulatorPluginChanged(const BlackMisc::Simulation::CSimulatorPluginInfo &info);

            //! Connected network server has been changed
            void onConnectedServerChanged(const BlackMisc::Network::CServer &server);

            BlackMisc::CDigestSignal m_dsCommandTooltip { this, &CCommandInput::setCommandTooltip, 5000, 2 };
        };
    } // ns
} // ns
#endif // guard
