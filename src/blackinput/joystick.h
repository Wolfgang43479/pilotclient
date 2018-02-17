/* Copyright (C) 2014
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKINPUT_JOYSTICK_H
#define BLACKINPUT_JOYSTICK_H

#include "blackinputexport.h"
#include "blackmisc/input/hotkeycombination.h"
#include <QObject>
#include <memory>

namespace BlackInput
{
    /*!
     * Abstract interface for native joystick handling.
     */
    class BLACKINPUT_EXPORT IJoystick : public QObject
    {
        Q_OBJECT

    public:
        //! Constructor
        IJoystick(QObject *parent = nullptr);

        //! Destructor
        virtual ~IJoystick() {}

        //! Creates a native joystick handler object
        static std::unique_ptr<IJoystick> create(QObject *parent = nullptr);

    signals:
        //! Joystick button combination has changed
        void buttonCombinationChanged(const BlackMisc::Input::CHotkeyCombination &);
    };
}

#endif // guard
