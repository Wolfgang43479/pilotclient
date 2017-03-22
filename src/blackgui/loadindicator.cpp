/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 *
 * Class based on qLed: Copyright (C) 2010 by P. Sereno, http://www.sereno-online.com
 */

#include "loadindicator.h"
#include "guiapplication.h"

#include <QColor>
#include <QPainter>
#include <QRect>
#include <QSizePolicy>
#include <QtGlobal>

namespace BlackGui
{
    CLoadIndicator::CLoadIndicator(int width, int height, QWidget *parent)
        : QWidget(parent)
    {
        this->resize(width, height);
        this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        this->setFocusPolicy(Qt::NoFocus);
        this->setAutoFillBackground(true);
        this->setStyleSheet("background-color: transparent;");
    }

    bool CLoadIndicator::isAnimated() const
    {
        return (this->m_timerId != -1);
    }

    void CLoadIndicator::setDisplayedWhenStopped(bool state)
    {
        this->m_displayedWhenStopped = state;
        this->update();
    }

    bool CLoadIndicator::isDisplayedWhenStopped() const
    {
        return m_displayedWhenStopped;
    }

    int CLoadIndicator::startAnimation(int timeoutMs, bool processEvents)
    {
        this->m_angle = 0;
        this->show();
        this->setEnabled(true);
        if (this->m_timerId == -1) { this->m_timerId = startTimer(m_delayMs); }
        if (processEvents && sGui)
        {
            sGui->processEventsToRefreshGui();
        }

        const int stopId = this->m_currentId++; // copy
        if (timeoutMs > 0)
        {
            QTimer::singleShot(timeoutMs, this, [this, stopId]
            {
                // only timeout myself id
                this->stopAnimation(stopId);
                emit this->timedOut();
            });
        }
        this->m_pendingIds.push_back(stopId);
        return stopId;
    }

    void CLoadIndicator::stopAnimation(int indicatorId)
    {
        if (indicatorId > 0)
        {
            this->m_pendingIds.removeOne(indicatorId);
            // if others pending do not stop
            if (!this->m_pendingIds.isEmpty()) { return; }
        }
        this->m_pendingIds.clear();
        if (this->m_timerId != -1) { killTimer(this->m_timerId); }
        this->m_timerId = -1;
        this->hide();
        this->setEnabled(false);
        this->update();
    }

    void CLoadIndicator::setAnimationDelay(int delay)
    {
        this->m_delayMs = delay;
        if (this->m_timerId != -1) { this->killTimer(this->m_timerId); }
        this->m_timerId = this->startTimer(this->m_delayMs);
    }

    void CLoadIndicator::setColor(const QColor &color)
    {
        this->m_color = color;
        update();
    }

    QSize CLoadIndicator::sizeHint() const
    {
        return QSize(64, 64);
    }

    int CLoadIndicator::heightForWidth(int w) const
    {
        return w;
    }

    void CLoadIndicator::timerEvent(QTimerEvent *event)
    {
        Q_UNUSED(event);
        this->m_angle = (this->m_angle + 30) % 360;
        this->update();
    }

    void CLoadIndicator::paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);
        QPainter p(this);
        this->paint(p);
    }

    bool CLoadIndicator::isParentVisible() const
    {
        if (this->parentWidget()) { return parentWidget()->isVisible(); }
        return false;
    }

    void CLoadIndicator::paint(QPainter &painter) const
    {
        if (!m_displayedWhenStopped && !isAnimated()) { return; }
        if (!this->isVisible() || !this->isEnabled()) { return; }
        if (!isParentVisible()) { return; }

        int width = qMin(this->width(), this->height());
        painter.setRenderHint(QPainter::Antialiasing);

        // painter.setBrush(QBrush(QColor(0, 0, 255)));
        // painter.drawEllipse(0, 0, width, width);

        int outerRadius = (width - 1) * 0.5;
        int innerRadius = (width - 1) * 0.5 * 0.38;

        int capsuleHeight = outerRadius - innerRadius;
        int capsuleWidth  = (width > 32) ? capsuleHeight * .23 : capsuleHeight * .35;
        int capsuleRadius = capsuleWidth / 2;

        for (int i = 0; i < 12; i++)
        {
            QColor color = m_color;
            color.setAlphaF(1.0f - (i / 12.0f));
            painter.setPen(Qt::NoPen);
            painter.setBrush(color);
            painter.save();
            painter.translate(rect().center());
            painter.rotate(m_angle - i * 30.0f);
            painter.drawRoundedRect(-capsuleWidth * 0.5, -(innerRadius + capsuleHeight), capsuleWidth, capsuleHeight, capsuleRadius, capsuleRadius);
            painter.restore();
        }
    }

    void CLoadIndicator::centerLoadIndicator(const QPoint &middle)
    {
        const int w = this->width();
        const int h = this->height();
        const int x = middle.x() - w / 2;
        const int y = middle.y() - h / 2;
        this->setGeometry(x, y, w, h);
    }

    CLoadIndicatorEnabled::CLoadIndicatorEnabled(QWidget *usingWidget) :
        m_usingWidget(usingWidget)
    {
        Q_ASSERT_X(usingWidget, Q_FUNC_INFO, "need widget");
    }

    bool CLoadIndicatorEnabled::isShowingLoadIndicator() const
    {
        return m_loadIndicator && m_usingWidget->isVisible() && m_loadIndicator->isAnimated();
    }

    bool CLoadIndicatorEnabled::isLoadInProgress() const
    {
        return m_loadInProgress;
    }

    void CLoadIndicatorEnabled::showLoading(int timeoutMs, bool processEvents)
    {
        if (!this->m_loadIndicator)
        {
            this->m_loadIndicator = new CLoadIndicator(64, 64, m_usingWidget);
            QObject::connect(this->m_loadIndicator, &CLoadIndicator::timedOut,
                             [this] { this->indicatorTimedOut(); });
        }

        this->centerLoadIndicator();
        m_indicatorId = this->m_loadIndicator->startAnimation(timeoutMs, processEvents);
    }

    void CLoadIndicatorEnabled::hideLoading()
    {
        if (m_loadIndicator)
        {
            m_loadIndicator->stopAnimation();
        }
    }

    void CLoadIndicatorEnabled::centerLoadIndicator()
    {
        if (!m_loadIndicator) { return; }
        const QPoint middle = this->m_usingWidget->visibleRegion().boundingRect().center();
        this->m_loadIndicator->centerLoadIndicator(middle);
    }

    void CLoadIndicatorEnabled::indicatorTimedOut()
    {
        // to be overridden
    }
} // ns
