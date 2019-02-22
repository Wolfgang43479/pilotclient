/* Copyright (C) 2016
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "blackcore/application.h"
#include "blackcore/pluginmanagerweatherdata.h"
#include "blackcore/weatherdata.h"
#include "blackmisc/directoryutils.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QStringBuilder>

namespace BlackCore
{

    using namespace BlackMisc;

    CPluginManagerWeatherData::CPluginManagerWeatherData(QObject *parent) : IPluginManager(parent)
    {
    }

    IWeatherDataFactory *CPluginManagerWeatherData::getFactory(const QString &pluginId)
    {
        return getPluginById<IWeatherDataFactory>(pluginId);
    }

    Weather::CWeatherDataPluginInfoList CPluginManagerWeatherData::getAvailableWeatherDataPlugins() const
    {
        BlackMisc::Weather::CWeatherDataPluginInfoList list;
        for (const auto &i : m_plugins)
        {
            list.push_back(i.info);
        }
        return list;
    }

    void CPluginManagerWeatherData::collectPlugins()
    {
        IPluginManager::collectPlugins();

        const CSequence<QJsonObject> &plugins = getPlugins();
        for (const QJsonObject &json : plugins)
        {
            QString iid = json["IID"].toString();
            if (iid == QStringLiteral("org.swift-project.blackcore.weatherdata"))
            {
                // PluginExtended() instead of {} to silence wrong warning for gcc < 5.X
                auto it = m_plugins.insert(pluginIdentifier(json), PluginExtended());
                it->info.convertFromJson(json);
            }
        }
    }

    BlackMisc::CSequence<QString> CPluginManagerWeatherData::acceptedIids() const
    {
        return
        {
            QStringLiteral("org.swift-project.blackcore.weatherdata")
        };
    }

    const QString &CPluginManagerWeatherData::pluginDirectory() const
    {
        static const QString d(CFileUtils::appendFilePaths(CDirectoryUtils::pluginsDirectory(), "weatherdata"));
        return d;
    }
}
