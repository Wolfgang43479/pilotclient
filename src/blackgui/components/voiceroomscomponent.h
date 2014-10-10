/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#ifndef BLACKGUI_VOICEROOMSCOMPONENT_H
#define BLACKGUI_VOICEROOMSCOMPONENT_H

#include <QFrame>
#include <QScopedPointer>

namespace Ui { class CVoiceRoomsComponent; }

namespace BlackGui
{
    namespace Components
    {
        //! Displays the voice rooms
        class CVoiceRoomsComponent :
            public QFrame
        {
            Q_OBJECT

        public:
            //! Constructor
            explicit CVoiceRoomsComponent(QWidget *parent = nullptr);

            //! Destructor
            ~CVoiceRoomsComponent();

        private:
            QScopedPointer<Ui::CVoiceRoomsComponent> ui;
        };

    } // namespace
} // namespace

#endif // guard
