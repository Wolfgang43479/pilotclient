/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "pluginselector.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QVariant>

namespace BlackGui
{

    CPluginSelector::CPluginSelector(QWidget *parent) : QWidget(parent),
        m_detailsButtonMapper(new QSignalMapper(this)),
        m_configButtonMapper(new QSignalMapper(this))
    {
        setObjectName("CPluginSelector");

        QVBoxLayout *layout = new QVBoxLayout;
        setLayout(layout);

        connect(m_detailsButtonMapper, static_cast<void (QSignalMapper::*)(const QString &)>(&QSignalMapper::mapped), this, &CPluginSelector::pluginDetailsRequested);
        connect(m_configButtonMapper, static_cast<void (QSignalMapper::*)(const QString &)>(&QSignalMapper::mapped), this, &CPluginSelector::pluginConfigRequested);
    }

    void CPluginSelector::addPlugin(const QString& identifier, const QString &name, bool hasConfig, bool enabled)
    {
        QWidget *pw = new QWidget;
        QHBoxLayout *layout = new QHBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        pw->setLayout(layout);

        QCheckBox *cb = new QCheckBox(name);
        cb->setProperty("pluginIdentifier", identifier);
        connect(cb, &QCheckBox::stateChanged, this, &CPluginSelector::ps_handlePluginStateChange);
        if (enabled)
        {
            cb->setCheckState(Qt::Checked);
        }
        else
        {
            cb->setCheckState(Qt::Unchecked);
        }

        pw->layout()->addWidget(cb);

        if (hasConfig) {
            QPushButton *config = new QPushButton("...");
            m_configButtonMapper->setMapping(config, identifier);
            connect(config, &QPushButton::clicked, m_configButtonMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
            pw->layout()->addWidget(config);
        }

        QPushButton *details = new QPushButton("?");
        m_detailsButtonMapper->setMapping(details, identifier);
        connect(details, &QPushButton::clicked, m_detailsButtonMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        pw->layout()->addWidget(details);

        layout->setStretch(0, 1);
        layout->setStretch(1, 0);
        layout->setStretch(2, 0);

        this->layout()->addWidget(pw);
    }

    void CPluginSelector::ps_handlePluginStateChange()
    {
        QCheckBox *cb = qobject_cast<QCheckBox*>(sender());
        Q_ASSERT(cb);

        bool enabled = cb->checkState() != Qt::Unchecked;
        Q_ASSERT(cb->property("pluginIdentifier").isValid());
        QString identifier = cb->property("pluginIdentifier").toString();
        Q_ASSERT(!identifier.isEmpty());

        emit pluginStateChanged(identifier, enabled);
    }


} // namespace
