/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "blackgui/views/countryview.h"

using namespace BlackGui::Models;

namespace BlackGui
{
    namespace Views
    {
        CCountryView::CCountryView(QWidget *parent) :
            CViewBase(parent)
        {
            this->standardInit(new CCountryListModel(this));
            this->setMenu(MenuDefaultDbViews);
        }
    } // ns
} // ns
