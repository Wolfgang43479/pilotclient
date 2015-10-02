/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "ui_datainfoareacomponent.h"
#include "blackgui/components/logcomponent.h"
#include "blackgui/components/datainfoareacomponent.h"
#include "blackcore/webdataservices.h"
#include "blackmisc/icons.h"
#include "blackmisc/logmessage.h"

using namespace BlackMisc;
using namespace BlackGui;
using namespace BlackGui::Components;

namespace BlackGui
{
    namespace Components
    {
        CDataInfoAreaComponent::CDataInfoAreaComponent(QWidget *parent) :
            CInfoArea(parent),
            ui(new Ui::CDataInfoAreaComponent)
        {
            ui->setupUi(this);
            initInfoArea(); // init base class
            this->setWindowIcon(CIcons::swiftDatabase24());
            this->ps_setTabBarPosition(QTabWidget::North);
        }

        CDataInfoAreaComponent::~CDataInfoAreaComponent()
        { }

        CDbModelComponent *CDataInfoAreaComponent::getModelComponent() const
        {
            return this->ui->comp_DbModels;
        }

        CDbLiveryComponent *CDataInfoAreaComponent::getLiveryComponent() const
        {
            return this->ui->comp_DbLiveries;
        }

        CDbDistributorComponent *CDataInfoAreaComponent::getDistributorComponent() const
        {
            return this->ui->comp_DbDistributors;
        }

        CDbAircraftIcaoComponent *CDataInfoAreaComponent::getAircraftComponent() const
        {
            return this->ui->comp_DbAircraftIcao;
        }

        CDbAirlineIcaoComponent *CDataInfoAreaComponent::getAirlineComponent() const
        {
            return this->ui->comp_DbAirlineIcao;
        }

        CDbCountryComponent *CDataInfoAreaComponent::getCountryComponent() const
        {
            return this->ui->comp_DbCountries;
        }

        void CDataInfoAreaComponent::setProvider(BlackMisc::Network::IWebDataServicesProvider *provider)
        {
            Q_ASSERT_X(provider, Q_FUNC_INFO, "Missing provider");
            this->ui->comp_DbAircraftIcao->setProvider(provider);
            this->ui->comp_DbAirlineIcao->setProvider(provider);
            this->ui->comp_DbDistributors->setProvider(provider);
            this->ui->comp_DbLiveries->setProvider(provider);
            this->ui->comp_DbModels->setProvider(provider);
            this->ui->comp_DbCountries->setProvider(provider);
            CWebDataServicesAware::setProvider(provider);
        }

        bool CDataInfoAreaComponent::writeDbDataToResourceDir() const
        {
            bool s = hasProvider() &&
                     this->writeDbDataToDisk(CProject::getSwiftStaticDbFilesDir());
            if (s)
            {
                CLogMessage(this).info("Written DB data");
            }
            else
            {
                CLogMessage(this).error("Cannot write DB data");
            }
            return s;
        }

        bool CDataInfoAreaComponent::readDbDataFromResourceDir()
        {
            bool s = hasProvider() &&
                     this->readDbDataFromDisk(CProject::getSwiftStaticDbFilesDir(), true);

            // info
            if (s)
            {
                CLogMessage(this).info("Read DB data");
                this->ui->comp_DbAircraftIcao->showLoadIndicator();
                this->ui->comp_DbAirlineIcao->showLoadIndicator();
                this->ui->comp_DbCountries->showLoadIndicator();
                this->ui->comp_DbDistributors->showLoadIndicator();
                this->ui->comp_DbLiveries->showLoadIndicator();
                this->ui->comp_DbModels->showLoadIndicator();
            }
            else
            {
                CLogMessage(this).error("Failed to load DB data");
            }
            return s;
        }

        QSize CDataInfoAreaComponent::getPreferredSizeWhenFloating(int areaIndex) const
        {
            InfoArea area = static_cast<InfoArea>(areaIndex);
            switch (area)
            {
            case InfoAreaAircraftIcao:
            case InfoAreaAirlineIcao:
            case InfoAreaLiveries:
            case InfoAreaModels:
            case InfoAreaCountries:
            default:
                return QSize(800, 600);
            }
        }

        const QPixmap &CDataInfoAreaComponent::indexToPixmap(int areaIndex) const
        {
            InfoArea area = static_cast<InfoArea>(areaIndex);
            switch (area)
            {
            case InfoAreaAircraftIcao:
                return CIcons::appAircraftIcao16();
            case InfoAreaAirlineIcao:
                return CIcons::appAirlineIcao16();
            case InfoAreaLiveries:
                return CIcons::appLiveries16();
            case InfoAreaDistributors:
                return CIcons::appDistributors16();
            case InfoAreaModels:
                return CIcons::appModels16();
            case InfoAreaCountries:
                return CIcons::appCountries16();
            default:
                return CIcons::empty();
            }
        }
    } // ns
} // ns
