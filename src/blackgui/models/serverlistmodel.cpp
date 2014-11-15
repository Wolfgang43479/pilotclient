/*  Copyright (C) 2013 VATSIM Community / contributors
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "serverlistmodel.h"
#include "blackmisc/blackmiscfreefunctions.h"
#include <QMetaProperty>
#include <QBrush>

using namespace BlackMisc::Network;

namespace BlackGui
{
    namespace Models
    {

        /*
         * Constructor
         */
        CServerListModel::CServerListModel(QObject *parent) :
            CListModelBase("ViewServerList", parent)
        {
            this->m_columns.addColumn(CColumn::standardString("name", CServer::IndexName));
            this->m_columns.addColumn(CColumn::standardString("description", CServer::IndexDescription));
            this->m_columns.addColumn(CColumn::standardString("address", CServer::IndexAddress));
            this->m_columns.addColumn(CColumn::standardString("port", CServer::IndexPort));
            this->m_columns.addColumn(CColumn::standardString("realname", { CServer::IndexUser, CUser::IndexRealName}));
            this->m_columns.addColumn(CColumn::standardString("userid", { CServer::IndexUser, CUser::IndexId}));

            // force strings for translation in resource files
            (void)QT_TRANSLATE_NOOP("ViewServerList", "name");
            (void)QT_TRANSLATE_NOOP("ViewServerList", "description");
            (void)QT_TRANSLATE_NOOP("ViewServerList", "address");
            (void)QT_TRANSLATE_NOOP("ViewServerList", "port");
            (void)QT_TRANSLATE_NOOP("ViewServerList", "realname");
            (void)QT_TRANSLATE_NOOP("ViewServerList", "userid");
        }

    } // class
} // namespace
