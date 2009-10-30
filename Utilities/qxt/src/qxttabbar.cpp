/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtGui module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/
#include "qxttabbar.h"
#include <QDragMoveEvent>
#include <QApplication>
#include <QTabWidget>
#include <QMimeData>
#include <QIcon>

class QxtTabBarPrivate : public QxtPrivate<QxtTabBar>
{
public:
    QXT_DECLARE_PUBLIC(QxtTabBar);

    QxtTabBarPrivate();

    enum MovementAction
    {
        PressAction,
        MoveAction,
        DragAction,
        DropAction,
    };

    int tabAt(const QPoint& position, MovementAction action) const;
    bool contains(const QRect& rect, const QPoint& position, MovementAction action) const;

    bool shouldMove(const QPoint& pos, int fromIndex, int toIndex, MovementAction action) const;
    bool moveTab(const QPoint& pos, int fromIndex, int toIndex, MovementAction action);

    struct Tab
    {
        QIcon    icon;
        QVariant data;
        QString  text;
        QColor   textColor;
        QString  toolTip;
        QString  whatsThis;
    };
    Tab saveTab(int index) const;
    void restoreTab(int index, const Tab& tab);

    int movingTab;
    QPoint pressPoint;
    QxtTabBar::TabMovementMode mode;
};

QxtTabBarPrivate::QxtTabBarPrivate() : movingTab(-1), mode(QxtTabBar::NoMovement)
{
}

int QxtTabBarPrivate::tabAt(const QPoint& position, MovementAction action) const
{
    const int count = qxt_p().count();
    for (int i = 0; i < count; ++i)
    {
        if (contains(qxt_p().tabRect(i), position, action))
        {
            return i;
        }
    }
    return -1;
}

bool QxtTabBarPrivate::contains(const QRect& rect, const QPoint& position, MovementAction action) const
{
    if (action != PressAction)
    {
        switch (qxt_p().shape())
        {
        case QTabBar::RoundedNorth:
        case QTabBar::RoundedSouth:
        case QTabBar::TriangularNorth:
        case QTabBar::TriangularSouth:
            return position.x() >= rect.x() && position.x() <= rect.x() + rect.width();

        case QTabBar::RoundedWest:
        case QTabBar::RoundedEast:
        case QTabBar::TriangularWest:
        case QTabBar::TriangularEast:
            return position.y() >= rect.y() && position.y() <= rect.y() + rect.height();

        default:
            qWarning("QTabBarPrivate: unknown QTabBar::Shape %i", qxt_p().shape());
            return false;
        }
    }
    else // (action == PressAction)
    {
        return rect.contains(position);
    }
}

bool QxtTabBarPrivate::shouldMove(const QPoint& pos, int fromIndex, int toIndex, MovementAction action) const
{
    if (fromIndex == -1 || toIndex == -1 || fromIndex == toIndex)
        return false;

    QRect sourceRect = qxt_p().tabRect(fromIndex);
    QRect targetRect = qxt_p().tabRect(toIndex);

    QRect finalRect;
    if (action == DropAction)
    {
        finalRect = targetRect;
    }
    else
    {
        finalRect = sourceRect;
        switch (qxt_p().shape())
        {
        case QTabBar::RoundedNorth:
        case QTabBar::RoundedSouth:
        case QTabBar::TriangularNorth:
        case QTabBar::TriangularSouth:
            if (qxt_p().layoutDirection() == Qt::LeftToRight && sourceRect.x() < targetRect.x())
                finalRect.moveRight(targetRect.right());
            else
                finalRect.moveLeft(targetRect.left());
            break;

        case QTabBar::RoundedWest:
        case QTabBar::RoundedEast:
        case QTabBar::TriangularWest:
        case QTabBar::TriangularEast:
            if (sourceRect.y() < targetRect.y())
                finalRect.moveBottom(targetRect.bottom());
            else
                finalRect.moveTop(targetRect.top());
            break;

        default:
            qWarning("QTabBarPrivate: unknown QTabBar::Shape %i", qxt_p().shape());
            return false;
        }
    }

    return contains(finalRect, pos, action);
}

bool QxtTabBarPrivate::moveTab(const QPoint& pos, int fromIndex, int toIndex, MovementAction action)
{
    QxtTabBar* tabBar = &qxt_p();

    if (shouldMove(pos, fromIndex, toIndex, action))
    {
        Tab tab = saveTab(fromIndex);

        // if parent is a QTabWidget we can use it to move the tabs and widgets
        QTabWidget* tabWidget = qobject_cast<QTabWidget*>(tabBar->parent());
        if (tabWidget)
        {
            QWidget* widget = tabWidget->widget(fromIndex);
            tabWidget->removeTab(fromIndex);
            tabWidget->insertTab(toIndex, widget, "");
            tabWidget->setCurrentIndex(toIndex);
        }
        else // tabbar is standalone (not embedded into a QTabWidget)
        {
            tabBar->removeTab(fromIndex);
            tabBar->insertTab(toIndex, "");
            tabBar->setCurrentIndex(toIndex);
        }

        restoreTab(toIndex, tab);
        return true;
    }

    return false;
}

QxtTabBarPrivate::Tab QxtTabBarPrivate::saveTab(int index) const
{
    Tab tab;
    const QTabBar* tabBar = &qxt_p();
    tab.icon      = tabBar->tabIcon(index);
    tab.data      = tabBar->tabData(index);
    tab.text      = tabBar->tabText(index);
    tab.textColor = tabBar->tabTextColor(index);
    tab.toolTip   = tabBar->tabToolTip(index);
    tab.whatsThis = tabBar->tabWhatsThis(index);
    return tab;
}

void QxtTabBarPrivate::restoreTab(int index, const Tab& tab)
{
    QTabBar* tabBar = &qxt_p();
    tabBar->setTabIcon(index, tab.icon);
    tabBar->setTabData(index, tab.data);
    tabBar->setTabText(index, tab.text);
    tabBar->setTabTextColor(index, tab.textColor);
    tabBar->setTabToolTip(index, tab.toolTip);
    tabBar->setTabWhatsThis(index, tab.whatsThis);
}

/*!
    \class QxtTabBar QxtTabBar
    \ingroup QxtGui
    \brief An extended QTabBar.

    QxtTabBar provides tabs that can be moved. QxtTabWidget uses QxtTabBar internally
    and thus its tabs are also optionally movable.

    Example usage:
    \code
    QxtTabBar* tabBar = new QxtTabBar(tabWidget);
    tabBar->setMovableTabs(true); // tabs are movable now
    \endcode
 */

/*!
    \fn QxtTabBar::tabMoved(int fromIndex, int toIndex)

    This signal is emitted whenever a tab is moved \a fromIndex \a toIndex.
 */

/*!
    Constructs a new QxtTabBar with \a parent.
 */

QxtTabBar::QxtTabBar(QWidget* parent)
        : QTabBar(parent)
{
    QXT_INIT_PRIVATE(QxtTabBar);
}

QxtTabBar::~QxtTabBar() {}

/*!
    \property QxtTabBar::tabMovementMode
    \brief This property holds how tabs can be moved.

    The default value of this property is \b QxtTabBar::NoMovement.

    \sa tabMoved()
 */

QxtTabBar::TabMovementMode QxtTabBar::tabMovementMode() const
{
    return qxt_d().mode;
}

void QxtTabBar::setTabMovementMode(TabMovementMode mode)
{
    qxt_d().mode = mode;
    if (mode == DragDropMovement)
        setAcceptDrops(true);
}

/*!
    \reimp
 */
void QxtTabBar::mousePressEvent(QMouseEvent* event)
{
    QTabBar::mousePressEvent(event);
    if (event->button() == Qt::LeftButton)
    {
        qxt_d().movingTab = qxt_d().tabAt(event->pos(), QxtTabBarPrivate::PressAction);
        qxt_d().pressPoint = event->pos();
    }
}

/*!
    \reimp
 */
void QxtTabBar::mouseMoveEvent(QMouseEvent* event)
{
    QTabBar::mouseMoveEvent(event);

    if (qxt_d().movingTab == -1)
        return;

    if (qxt_d().mode == InPlaceMovement)
    {
        int hoverTab = qxt_d().tabAt(event->pos(), QxtTabBarPrivate::MoveAction);
        if (qxt_d().moveTab(event->pos(), qxt_d().movingTab, hoverTab, QxtTabBarPrivate::MoveAction))
        {
            emit tabMoved(qxt_d().movingTab, hoverTab);
            qxt_d().movingTab = hoverTab;
            event->accept();
        }
    }
    else if (qxt_d().mode == DragDropMovement)
    {
        if ((event->pos() - qxt_d().pressPoint).manhattanLength()
                >= QApplication::startDragDistance())
        {
            QDrag* drag = new QDrag(this);
            QMimeData* mimeData = new QMimeData;

            // a crude way to distinguish tab movement drops from other ones
            mimeData->setData("action", "tabMovement");
            drag->setMimeData(mimeData);
            drag->start(); // to support < Qt 4.3
        }
    }
}

/*!
    \reimp
 */
void QxtTabBar::mouseReleaseEvent(QMouseEvent* event)
{
    QTabBar::mouseReleaseEvent(event);
    qxt_d().movingTab = -1;
}

/*!
    \reimp
 */
void QxtTabBar::dragEnterEvent(QDragEnterEvent* event)
{
    // only accept if it's a tab movement request
    const QMimeData* mimeData = event->mimeData();
    QStringList formats = mimeData->formats();
    if (formats.contains("action") && (mimeData->data("action") == "tabMovement"))
        event->acceptProposedAction();
    else
        QTabBar::dragEnterEvent(event);
}

void QxtTabBar::dragMoveEvent(QDragMoveEvent* event)
{
    // only accept if it's a tab movement request
    const QMimeData* mimeData = event->mimeData();
    QStringList formats = mimeData->formats();
    if (formats.contains("action") && (mimeData->data("action") == "tabMovement"))
        event->acceptProposedAction();
    else
        QTabBar::dragMoveEvent(event);
}

/*!
    \reimp
 */
void QxtTabBar::dropEvent(QDropEvent* event)
{
    // only accept if it's a tab movement request
    const QMimeData* mimeData = event->mimeData();
    QStringList formats = mimeData->formats();
    if (formats.contains("action") && (mimeData->data("action") == "tabMovement"))
    {
        int dropTab = qxt_d().tabAt(event->pos(), QxtTabBarPrivate::DropAction);
        if (qxt_d().moveTab(event->pos(), qxt_d().movingTab, dropTab, QxtTabBarPrivate::DropAction))
        {
            emit tabMoved(qxt_d().movingTab, dropTab);
            qxt_d().movingTab = -1;
            event->acceptProposedAction();
        }
    }
    else
    {
        QTabBar::dropEvent(event);
    }
}
