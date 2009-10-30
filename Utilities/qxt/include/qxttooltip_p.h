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
#ifndef QXTTOOLTIP_P_H
#define QXTTOOLTIP_P_H

#include <QPointer>
#include <QWidget>
#include <QHash>
#include "qxttooltip.h"

QT_FORWARD_DECLARE_CLASS(QVBoxLayout)

typedef QPointer<QWidget> WidgetPtr;
typedef QPair<WidgetPtr, QRect> WidgetArea;

class QxtToolTipPrivate : public QWidget
{
    Q_OBJECT

public:
    QxtToolTipPrivate();
    ~QxtToolTipPrivate();

    static QxtToolTipPrivate* instance();
    void show(const QPoint& pos, QWidget* tooltip, QWidget* parent = 0, const QRect& rect = QRect());
    void setToolTip(QWidget* tooltip);
    bool eventFilter(QObject* parent, QEvent* event);
    void hideLater();
    QPoint calculatePos(int scr, const QPoint& eventPos) const;
    QHash<WidgetPtr, WidgetArea> tooltips;
    QVBoxLayout* vbox;

protected:
    void enterEvent(QEvent* event);
    void paintEvent(QPaintEvent* event);

private:
    static QxtToolTipPrivate* self;
    QWidget* currentParent;
    QRect currentRect;
};

#endif // QXTTOOLTIP_P_H
