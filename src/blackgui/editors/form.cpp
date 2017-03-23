/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackgui/editors/form.h"
#include <QApplication>
#include <QClipboard>
#include <QShortcut>

using namespace BlackMisc;
using namespace BlackMisc::Network;

namespace BlackGui
{
    namespace Editors
    {
        CForm::CForm(QWidget *parent) : COverlayMessagesFrame(parent)
        { }

        CForm::~CForm() { }

        void CForm::setSelectOnly()
        {
            this->setReadOnly(true);
        }

        CStatusMessageList CForm::validate(bool withNestedObjects) const
        {
            Q_UNUSED(withNestedObjects);
            return CStatusMessageList();
        }

        CAuthenticatedUser CForm::getSwiftDbUser() const
        {
            return this->m_swiftDbUser.get();
        }

        void CForm::jsonPasted(const QString &json)
        {
            Q_UNUSED(json);
        }

        void CForm::ps_userChanged()
        {
            // void
        }

        void CForm::ps_pasted()
        {
            if (!QApplication::clipboard()) { return; }
            const QString data = QApplication::clipboard()->text();
            if (!Json::looksLikeSwiftJson(data)) { return; }
            this->jsonPasted(data);
        }
    } // ns
} // ns
