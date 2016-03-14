/* Copyright (C) 2015
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_DISTRIBUTORLISTMODEL_H
#define BLACKGUI_DISTRIBUTORLISTMODEL_H

#include "blackgui/blackguiexport.h"
#include "blackmisc/simulation/distributorlist.h"
#include "blackgui/models/listmodeldbobjects.h"
#include <QAbstractItemModel>

namespace BlackGui
{
    namespace Models
    {
        //! Distributor list model
        class BLACKGUI_EXPORT CDistributorListModel :
            public CListModelDbObjects<BlackMisc::Simulation::CDistributor, BlackMisc::Simulation::CDistributorList, QString, true>
        {
        public:
            //! What kind of stations
            enum DistributorMode
            {
                NotSet,
                Normal,
                Minimal
            };

            //! Constructor
            explicit CDistributorListModel(QObject *parent = nullptr);

            //! Destructor
            virtual ~CDistributorListModel() {}

            //! Set mode
            void setDistributorMode(DistributorMode distributorMode);

            //! Mode
            DistributorMode getDistributorMode() const { return this->m_distributorMode; }

        private:
            DistributorMode m_distributorMode = NotSet;
        };
    } // ns
} // ns

#endif // guard
