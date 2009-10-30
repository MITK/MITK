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
#ifndef QXTTABWIDGET_H
#define QXTTABWIDGET_H

#include <QTabWidget>
#include "qxtnamespace.h"
#include "qxtglobal.h"
#include "qxtpimpl.h"

class QxtTabBar;
class QxtTabWidgetPrivate;

class QXT_GUI_EXPORT QxtTabWidget : public QTabWidget
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(QxtTabWidget);
    Q_PROPERTY(bool alwaysShowTabBar READ alwaysShowTabBar WRITE setAlwaysShowTabBar)
    Q_PROPERTY(Qt::ContextMenuPolicy tabContextMenuPolicy READ tabContextMenuPolicy WRITE setTabContextMenuPolicy)
    Q_PROPERTY(TabMovementMode tabMovementMode READ tabMovementMode WRITE setTabMovementMode)

public:
    explicit QxtTabWidget(QWidget* parent = 0);
    virtual ~QxtTabWidget();

    bool alwaysShowTabBar() const;
    void setAlwaysShowTabBar(bool always);

    Qt::ContextMenuPolicy tabContextMenuPolicy() const;
    void setTabContextMenuPolicy(Qt::ContextMenuPolicy policy);

    enum TabMovementMode
    {
        NoMovement,
        InPlaceMovement,
        DragDropMovement
    };

    TabMovementMode tabMovementMode() const;
    void setTabMovementMode(TabMovementMode mode);

    void addTabAction(int index, QAction* action);
    QAction* addTabAction(int index, const QString& text);
    QAction* addTabAction(int index, const QIcon& icon, const QString& text);
    QAction* addTabAction(int index, const QString& text, const QObject* receiver, const char* member, const QKeySequence& shortcut = 0);
    QAction* addTabAction(int index, const QIcon& icon, const QString& text, const QObject* receiver, const char* member, const QKeySequence& shortcut = 0);
    void addTabActions(int index, QList<QAction*> actions);
    void clearTabActions(int index);
    void insertTabAction(int index, QAction* before, QAction* action);
    void insertTabActions(int index, QAction* before, QList<QAction*> actions);
    void removeTabAction(int index, QAction* action);
    QList<QAction*> tabActions(int index) const;

    QMovie* tabAnimation(int index) const;
    void setTabAnimation(int index, QMovie* animation, bool start = true);
    void setTabAnimation(int index, const QString& fileName, bool start = true);
    QMovie* takeTabAnimation(int index);

Q_SIGNALS:
    void tabContextMenuRequested(int index, const QPoint& globalPos);

protected:
    QxtTabBar* tabBar() const;

    virtual void tabInserted(int index);
    virtual void tabRemoved(int index);

    virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void tabContextMenuEvent(int index, QContextMenuEvent* event);
};

#endif // QXTTABWIDGET_H
