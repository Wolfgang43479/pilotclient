/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackgui/models/userlistmodel.h"
#include "blackgui/views/userview.h"

#include <QtGlobal>

using namespace BlackMisc;
using namespace BlackGui::Models;

namespace BlackGui
{
    namespace Views
    {
        CUserView::CUserView(QWidget *parent) : CViewBase(parent)
        {
            this->standardInit(new CUserListModel(CUserListModel::UserDetailed, this));
        }

        void CUserView::setUserMode(CUserListModel::UserMode userMode)
        {
            Q_ASSERT(m_model);
            m_model->setUserMode(userMode);
        }
    } // ns
} // ns
