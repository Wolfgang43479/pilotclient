/* Copyright (C) 2014
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "infoarea.h"
#include "stylesheetutility.h"
#include "guiutility.h"
#include "blackmisc/icons.h"
#include <QMenu>
#include <QListIterator>
#include <QSignalMapper>
#include <QCloseEvent>
#include <QStatusBar>

using namespace BlackMisc;

namespace BlackGui
{
    CInfoArea::CInfoArea(QWidget *parent) : QMainWindow(parent)
    {
        this->ps_setInfoAreaFloating(this->m_infoAreaFloating);
        this->setWindowIcon(CIcons::swift24());
    }

    CInfoArea::~CInfoArea()
    { }

    void CInfoArea::initInfoArea()
    {
        // after(!) GUI is setup
        if (this->m_dockWidgetInfoAreas.isEmpty())
        {
            this->m_dockWidgetInfoAreas = this->initOwnDockWidgetInfoAreas();
            Q_ASSERT(!this->m_dockWidgetInfoAreas.isEmpty());
        }

        this->ps_setDockArea(Qt::TopDockWidgetArea);
        this->setMarginsWhenFloating(5, 5, 5, 5); // left, top, right bottom
        this->setMarginsWhenDocked(1, 1, 1, 1);   // top has no effect
        this->connectAllWidgets();
        this->setFeaturesForDockableWidgets(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
        this->tabifyAllWidgets();

        // context menu
        this->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, &CInfoArea::customContextMenuRequested, this, &CInfoArea::ps_showContextMenu);
        connect(&CStyleSheetUtility::instance(), &CStyleSheetUtility::styleSheetsChanged, this, &CInfoArea::ps_onStyleSheetChanged);

        // initial style sheet setting
        this->ps_onStyleSheetChanged();

        // status bar
        if (this->statusBar())
        {
            this->statusBar()->hide();
            this->statusBar()->setMaximumHeight(0);
        }
    }

    void CInfoArea::addToContextMenu(QMenu *menu) const
    {
        if (!menu) return;
        bool hasDockedWidgets = this->countDockedWidgetInfoAreas() > 0;
        if (hasDockedWidgets)
        {
            menu->addAction(CIcons::dockTop16(), "Dock all", this, SLOT(dockAllWidgets()));
            menu->addAction(CIcons::floatAll16(), "Float all", this, SLOT(floatAllWidgets()));
            menu->addAction(CIcons::floatOne16(), QString("Dock / float '%1'").arg(this->windowTitle()), this, SLOT(toggleFloating()));
            QAction *lockTabBarMenuAction = new QAction(menu);
            lockTabBarMenuAction->setObjectName(this->objectName().append("LockTabBar"));
            lockTabBarMenuAction->setIconText("Lock tab bar");
            lockTabBarMenuAction->setIcon(CIcons::lockClosed16());
            lockTabBarMenuAction->setCheckable(true);
            lockTabBarMenuAction->setChecked(this->m_lockTabBar);
            menu->addAction(lockTabBarMenuAction);
            connect(lockTabBarMenuAction, &QAction::toggled, this, &CInfoArea::ps_toggleTabBarLocked);
            menu->addSeparator();
            QMenu *subMenuToggleFloat = new QMenu("Toggle Float/Dock", menu);
            QMenu *subMenuDisplay = new QMenu("Display", menu);

            QSignalMapper *signalMapperToggleFloating = new QSignalMapper(menu);
            QSignalMapper *signalMapperDisplay = new QSignalMapper(menu);
            bool c = false;

            for (int i = 0; i < this->m_dockWidgetInfoAreas.size(); i++)
            {
                const CDockWidgetInfoArea *dw = this->m_dockWidgetInfoAreas.at(i);
                const QString t = dw->windowTitleBackup();
                const QPixmap pm = indexToPixmap(i);
                QAction *toggleFloatingMenuAction = new QAction(menu);
                toggleFloatingMenuAction->setObjectName(QString(t).append("ToggleFloatingAction"));
                toggleFloatingMenuAction->setIconText(t);
                toggleFloatingMenuAction->setIcon(pm);
                toggleFloatingMenuAction->setData(QVariant(i));
                toggleFloatingMenuAction->setCheckable(true);
                toggleFloatingMenuAction->setChecked(!dw->isFloating());
                subMenuToggleFloat->addAction(toggleFloatingMenuAction);
                c = connect(toggleFloatingMenuAction, SIGNAL(toggled(bool)), signalMapperToggleFloating, SLOT(map()));
                Q_ASSERT(c);
                signalMapperToggleFloating->setMapping(toggleFloatingMenuAction, i);

                QAction *displayMenuAction = new QAction(menu);
                displayMenuAction->setObjectName(QString(t).append("DisplayAction"));
                displayMenuAction->setIconText(t);
                displayMenuAction->setIcon(pm);
                displayMenuAction->setData(QVariant(i));
                displayMenuAction->setCheckable(false);

                subMenuDisplay->addAction(displayMenuAction);
                c = connect(displayMenuAction, SIGNAL(triggered(bool)), signalMapperDisplay, SLOT(map()));
                Q_ASSERT(c);
                signalMapperDisplay->setMapping(displayMenuAction, i); // action to index
            }
            c = connect(signalMapperToggleFloating, SIGNAL(mapped(int)), this, SLOT(toggleFloating(int)));
            Q_ASSERT(c);

            c = connect(signalMapperDisplay, SIGNAL(mapped(int)), this, SLOT(selectArea(int)));
            Q_ASSERT(c);

            menu->addMenu(subMenuDisplay);
            menu->addMenu(subMenuToggleFloat);

            // where and how to display tab bar
            menu->addSeparator();
            QAction *showMenuText = new QAction(menu);
            showMenuText->setObjectName("ShowDockedWidgetTextAction");
            showMenuText->setIconText("Show tab text");
            showMenuText->setIcon(CIcons::headingOne16());
            showMenuText->setCheckable(true);
            showMenuText->setChecked(this->m_showTabTexts);
            menu->addAction(showMenuText);
            connect(showMenuText, &QAction::toggled, this, &CInfoArea::ps_showTabTexts);

            // auto adjust floating widgets
            QAction *showTabbar = new QAction(menu);
            showTabbar->setObjectName("ShowTabBar");
            showTabbar->setIconText("Show tab bar");
            showTabbar->setIcon(CIcons::dockBottom16());
            showTabbar->setCheckable(true);
            showTabbar->setChecked(this->m_showTabBar);
            menu->addAction(showTabbar);
            connect(showTabbar, &QAction::toggled, this, &CInfoArea::ps_showTabBar);

            // tab bar position
            menu->addAction(CIcons::dockBottom16(), "Toogle tabbar position", this, SLOT(ps_toggleTabBarPosition()));
        }
    }

    bool CInfoArea::isSelectedDockWidgetInfoArea(const CDockWidgetInfoArea *infoArea) const
    {
        if (!infoArea) return false;
        if (infoArea->isFloating()) return false;

        return infoArea == this->getSelectedDockInfoArea();
    }

    const CDockWidgetInfoArea *CInfoArea::getSelectedDockInfoArea() const
    {
        // we assume that there can only be 1, non floating info area,
        // which is the only visible one
        if (this->m_tabBar->count() < 1) { return nullptr; }
        foreach(const CDockWidgetInfoArea * ia, m_dockWidgetInfoAreas)
        {
            if (ia->isFloating()) { continue; }
            if (ia->isWidgetVisible()) { return ia; }
        }
        return nullptr;
    }

    QList<const CDockWidgetInfoArea *> CInfoArea::getDockWidgetInfoAreas() const
    {
        QList<const CDockWidgetInfoArea *> constDockWidgets;
        foreach(const CDockWidgetInfoArea * dockWidgetInfoArea, m_dockWidgetInfoAreas)
        {
            constDockWidgets.append(dockWidgetInfoArea);
        }
        return constDockWidgets;
    }

    void CInfoArea::paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);
        CStyleSheetUtility::useStyleSheetInDerivedWidget(this);
    }

    void CInfoArea::dockAllWidgets()
    {
        this->tabifyAllWidgets();
    }

    void CInfoArea::adjustSizeForAllDockWidgets()
    {
        for (CDockWidgetInfoArea *dw : this->m_dockWidgetInfoAreas)
        {
            dw->adjustSize();
        }
    }

    void CInfoArea::floatAllWidgets()
    {
        for (CDockWidgetInfoArea *dw : this->m_dockWidgetInfoAreas)
        {
            if (dw->isFloating()) continue;
            dw->toggleFloating();
        }
    }

    void CInfoArea::toggleFloating()
    {
        this->ps_setInfoAreaFloating(!this->m_infoAreaFloating);
    }

    void CInfoArea::toggleFloating(int areaIndex)
    {
        if (!this->isValidAreaIndex(areaIndex)) { return; }
        CDockWidgetInfoArea *dw = this->m_dockWidgetInfoAreas.at(areaIndex);
        Q_ASSERT(dw);
        if (!dw) return;
        dw->toggleFloating();
    }

    void CInfoArea::toggleVisibility(int areaIndex)
    {
        if (!this->isValidAreaIndex(areaIndex)) { return; }
        CDockWidgetInfoArea *dw = this->m_dockWidgetInfoAreas.at(areaIndex);
        Q_ASSERT(dw);
        if (!dw) return;
        dw->toggleVisibility();
    }

    void CInfoArea::selectArea(int areaIndex)
    {
        CDockWidgetInfoArea *dw = this->m_dockWidgetInfoAreas.at(areaIndex);
        Q_ASSERT(dw);
        if (!dw) return;
        Q_ASSERT(this->m_tabBar);
        if (m_tabBar->count() < 1) return;

        if (dw->isFloating())
        {
            dw->show();
        }
        else
        {
            this->setCurrentTabIndex(dw);
        }
    }

    void CInfoArea::ps_setDockArea(Qt::DockWidgetArea area)
    {
        for (CDockWidgetInfoArea *dw : this->m_dockWidgetInfoAreas)
        {
            Qt::DockWidgetAreas newAreas = static_cast<Qt::DockWidgetAreas>(area);
            Qt::DockWidgetAreas oldAreas = dw->allowedAreas();
            if (oldAreas == newAreas) continue;
            dw->setAllowedAreas(newAreas);
            this->addDockWidget(area, dw);
        }
    }

    void CInfoArea::ps_setInfoAreaFloating(bool floating)
    {
        // float whole info area
        this->m_infoAreaFloating = floating;
        if (this->m_infoAreaFloating)
        {
            QPoint p = CGuiUtility::mainWindowPosition();
            this->setWindowFlags(Qt::Dialog);
            this->move(p.rx() + 20, p.ry() + 20);
            this->show(); // not working without show
        }
        else
        {
            // make this compliant as QWidget
            // https://qt-project.org/forums/viewthread/17519
            // http://www.qtcentre.org/threads/12569-QMainWindow-as-a-child-of-QMainWindow

            // this->setParent(this->m_originalParent, this->windowFlags() & ~Qt::Window);
            this->setWindowFlags(this->windowFlags() & ~Qt::Window);
            this->setVisible(true); // after redocking this is required
        }
    }

    void CInfoArea::tabifyAllWidgets()
    {
        // this->setDockArea(Qt::LeftDockWidgetArea);
        this->setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::East);
        bool init = this->m_tabBar ? false : true;

        for (int i = 0; i < this->m_dockWidgetInfoAreas.size(); i++)
        {
            CDockWidgetInfoArea *first = i > 0 ? this->m_dockWidgetInfoAreas.at(i - 1) : nullptr;
            CDockWidgetInfoArea *after = this->m_dockWidgetInfoAreas.at(i);
            Q_ASSERT(after);

            // trick, init widget as floating
            // this completely initializes the tab bar and all docked widgets
            if (init)
            {
                QPoint offset(i * 25, i * 20);
                after->setVisible(false);
                after->setFloating(true);
                after->setOffsetWhenFloating(offset);
                after->setPreferredSizeWhenFloating(this->getPreferredSizeWhenFloating(i));
                after->setFloating(false);
                after->setVisible(true);
                after->resetWasAlreadyFLoating();
            }
            else
            {
                after->setFloating(false);
            }
            if (!first) { continue; }
            this->tabifyDockWidget(first, after);
        }

        // as now tabified, now set tab
        if (init)
        {
            this->m_tabBar = this->findChild<QTabBar *>();

            // if we have > 1 docked widgets, we have a tab bar
            if (this->m_tabBar)
            {
                QString qss = CStyleSheetUtility::instance().style(CStyleSheetUtility::fileNameDockWidgetTab());
                this->m_tabBar->setStyleSheet(qss);
                this->m_tabBar->setObjectName("comp_MainInfoAreaDockWidgetTab");
                this->m_tabBar->setMovable(false);
                this->m_tabBar->setElideMode(Qt::ElideNone);

                // East / West does not work (shown, but area itself empty)
                // South does not have any effect
                this->m_tabBar->setShape(QTabBar::TriangularSouth);

                // signals
                connect(this->m_tabBar, &QTabBar::tabBarDoubleClicked, this, &CInfoArea::ps_tabBarDoubleClicked);
                connect(this->m_tabBar, &QTabBar::currentChanged, this, &CInfoArea::tabBarCurrentChanged);
            }
            else
            {
                // <= 1 dock widget
                this->m_tabBar = new QTabBar(this);
                this->m_tabBar->hide();
            }
        }

        // set current index, and always set pixmaps
        if (this->countDockedWidgetInfoAreas() > 0) { this->m_tabBar->setCurrentIndex(0); }
        if (this->m_tabBar->count() > 0) { this->setTabPixmaps(); }
    }

    void CInfoArea::unTabifyAllWidgets()
    {
        if (this->m_dockWidgetInfoAreas.size() < 2) return;
        CDockWidgetInfoArea *first = this->m_dockWidgetInfoAreas.first();
        for (int i = 1; i < this->m_dockWidgetInfoAreas.size(); i++)
        {
            CDockWidgetInfoArea *after = this->m_dockWidgetInfoAreas.at(i);
            Q_ASSERT(after);
            this->splitDockWidget(first, after, Qt::Horizontal);
        }
    }

    bool CInfoArea::isValidAreaIndex(int areaIndex) const
    {
        if (!this->m_tabBar) { return false; }
        return areaIndex >= 0 && areaIndex < this->m_dockWidgetInfoAreas.size();
    }

    void CInfoArea::connectAllWidgets()
    {
        for (CDockWidgetInfoArea *dw : this->m_dockWidgetInfoAreas)
        {
            connect(dw, &CDockWidgetInfoArea::widgetTopLevelChanged, this, &CInfoArea::ps_onWidgetTopLevelChanged);
        }
    }

    void CInfoArea::setMarginsWhenFloating(int left, int top, int right, int bottom)
    {
        for (CDockWidgetInfoArea *dw : this->m_dockWidgetInfoAreas)
        {
            //! Margins when window is floating
            dw->setMarginsWhenFloating(left, top, right, bottom);
        }
    }

    void CInfoArea::setMarginsWhenDocked(int left, int top, int right, int bottom)
    {
        for (CDockWidgetInfoArea *dw : this->m_dockWidgetInfoAreas)
        {
            //! Margins when window is docked
            dw->setMarginsWhenDocked(left, top, right, bottom);
        }
    }

    QList<CDockWidgetInfoArea *> CInfoArea::initOwnDockWidgetInfoAreas()
    {
        QList<CDockWidgetInfoArea *> infoAreas = this->findChildren<CDockWidgetInfoArea *>();
        if (infoAreas.isEmpty()) { return infoAreas; }

        // nested info areas?
        QList<CInfoArea *> childInfoAreas = this->getChildInfoAreas();
        if (childInfoAreas.isEmpty()) { return infoAreas; }

        // we have child info areas (nested), we need to remove those from the list
        for (CInfoArea *ia : childInfoAreas)
        {
            QList<CDockWidgetInfoArea *> nestedDockWidgets = ia->m_dockWidgetInfoAreas;
            if (nestedDockWidgets.isEmpty()) { continue; }
            for (CDockWidgetInfoArea *ndw : nestedDockWidgets)
            {
                bool r = infoAreas.removeOne(ndw); // remove nested
                Q_ASSERT(r);
                Q_UNUSED(r);
            }
        }
        return infoAreas;
    }

    int CInfoArea::countDockedWidgetInfoAreas() const
    {
        if (!this->m_tabBar) return 0;
        return this->m_tabBar->count();
    }

    CDockWidgetInfoArea *CInfoArea::getDockWidgetInfoAreaByTabBarIndex(int tabBarIndex)
    {
        if (tabBarIndex >= this->m_dockWidgetInfoAreas.count() || tabBarIndex < 0) return nullptr;
        return getDockWidgetInfoAreaByWindowTitle(this->m_tabBar->tabText(tabBarIndex));
    }

    CDockWidgetInfoArea *CInfoArea::getDockWidgetInfoAreaByWindowTitle(const QString &title)
    {
        Q_ASSERT(!title.isEmpty());
        for (CDockWidgetInfoArea *dw : this->m_dockWidgetInfoAreas)
        {
            if (dw->windowTitle() == title) { return dw; }
        }
        return nullptr;
    }

    int CInfoArea::getAreaIndexByWindowTitle(const QString &title)
    {
        Q_ASSERT(!title.isEmpty());
        for (int i = 0; i < m_dockWidgetInfoAreas.size(); i++)
        {
            const QString t(m_dockWidgetInfoAreas.at(i)->windowTitle());
            if (t == title) { return i; }
        }
        Q_ASSERT(false);
        return -1;
    }

    int CInfoArea::getTabBarIndexByTitle(const QString &title) const
    {
        Q_ASSERT(!title.isEmpty());
        if (m_tabBar->count() < 1) { return -1;}
        for (int i = 0; i < m_tabBar->count(); i++)
        {
            if (m_tabBar->tabText(i) == title) { return i; }
        }
        return -1;
    }

    int CInfoArea::dockWidgetInfoAreaToTabBarIndex(const CDockWidgetInfoArea *dockWidgetInfoArea) const
    {
        if (!dockWidgetInfoArea) return -1;
        if (dockWidgetInfoArea->isFloating()) return -1;
        return getTabBarIndexByTitle(dockWidgetInfoArea->windowTitle());
    }

    void CInfoArea::setCurrentTabIndex(const CDockWidgetInfoArea *dockWidgetInfoArea)
    {
        if (!this->m_tabBar) { return; }
        int tabIndex = this->dockWidgetInfoAreaToTabBarIndex(dockWidgetInfoArea);
        if (tabIndex >= 0 && tabIndex < m_tabBar->count())
        {
            m_tabBar->setCurrentIndex(tabIndex);
        }
    }

    void CInfoArea::setFeaturesForDockableWidgets(QDockWidget::DockWidgetFeatures features)
    {
        for (CDockWidgetInfoArea *dw : this->m_dockWidgetInfoAreas)
        {
            dw->setFeatures(features);
        }
    }

    void CInfoArea::setTabPixmaps()
    {
        if (!this->m_tabBar) return;
        for (int i = 0; i < this->m_tabBar->count(); i++)
        {
            const QString t(this->m_tabBar->tabText(i));
            int areaIndex = this->getAreaIndexByWindowTitle(t);
            const QPixmap p = indexToPixmap(areaIndex);
            this->m_tabBar->setTabIcon(i, p);
        }
    }

    void CInfoArea::ps_tabBarDoubleClicked(int tabBarIndex)
    {
        if (this->m_lockTabBar) return;
        CDockWidgetInfoArea *dw = this->getDockWidgetInfoAreaByTabBarIndex(tabBarIndex);
        if (!dw) return;
        dw->toggleFloating();
    }

    void CInfoArea::ps_onWidgetTopLevelChanged(CDockWidget *dockWidget, bool topLevel)
    {
        Q_ASSERT(dockWidget);
        Q_UNUSED(topLevel);
        if (!dockWidget) return;

        // fix pixmaps
        this->setTabPixmaps();

        // select index
        if (topLevel) { return; }
        CDockWidgetInfoArea *dwia = dynamic_cast<CDockWidgetInfoArea *>(dockWidget);
        this->setCurrentTabIndex(dwia);
    }

    void CInfoArea::ps_onStyleSheetChanged()
    {
        if (this->m_tabBar)
        {
            QString qss = CStyleSheetUtility::instance().style(CStyleSheetUtility::fileNameDockWidgetTab());
            this->m_tabBar->setStyleSheet(qss);
        }
    }

    void CInfoArea::ps_showContextMenu(const QPoint &pos)
    {
        QPoint globalPos = this->mapToGlobal(pos);
        QScopedPointer<QMenu> contextMenu(new QMenu(this));
        this->addToContextMenu(contextMenu.data());

        QAction *selectedItem = contextMenu.data()->exec(globalPos);
        Q_UNUSED(selectedItem);
    }

    void CInfoArea::ps_showTabTexts(bool show)
    {
        if (show == this->m_showTabTexts) return;
        this->m_showTabTexts = show;
        for (CDockWidgetInfoArea *dw : this->m_dockWidgetInfoAreas)
        {
            dw->showTitleWhenDocked(show);
        }
    }

    void CInfoArea::ps_showTabBar(bool show)
    {
        if (show == this->m_showTabBar) return;
        this->m_showTabBar = show;
        if (!this->m_tabBar) return;
        this->m_tabBar->setVisible(show); // not working, but setting right value will not harm anything
        this->m_tabBar->setMaximumHeight(show ? 10000 : 0); // does the trick
        this->adjustSizeForAllDockWidgets();
    }

    void CInfoArea::ps_toggleTabBarLocked(bool locked)
    {
        this->m_lockTabBar = locked;
    }

    void CInfoArea::ps_setTabBarPosition(QTabWidget::TabPosition position)
    {
        Q_ASSERT(position == QTabWidget::North || position == QTabWidget::South);
        this->setTabPosition(Qt::TopDockWidgetArea, position);
    }

    void CInfoArea::ps_toggleTabBarPosition()
    {
        QTabWidget::TabPosition p = (this->tabPosition(Qt::TopDockWidgetArea) == QTabWidget::North) ?
                                    QTabWidget::South : QTabWidget::North;
        this->ps_setTabBarPosition(p);
    }

    void CInfoArea::closeEvent(QCloseEvent *event)
    {
        if (this->isFloating())
        {
            this->toggleFloating();
            event->setAccepted(false); // refuse -> do not close
        }
        else
        {
            QMainWindow::closeEvent(event);
        }
    }
} // namespace
