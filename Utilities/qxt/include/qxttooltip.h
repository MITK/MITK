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
#ifndef QXTTOOLTIP_H
#define QXTTOOLTIP_H

#include <QRect>
#include <QPointer>
#include <QWidget>
#include "qxtglobal.h"

class QXT_GUI_EXPORT QxtToolTip
{
    QxtToolTip();

public:
    static void show(const QPoint& pos, QWidget* tooltip, QWidget* parent = 0, const QRect& rect = QRect());
    static void hide();

    static QWidget* toolTip(QWidget* parent);
    static void setToolTip(QWidget* parent, QWidget* tooltip, const QRect& rect = QRect());

    static QRect toolTipRect(QWidget* parent);
    static void setToolTipRect(QWidget* parent, const QRect& rect);

    static int margin();
    static void setMargin(int margin);

    static qreal opacity();
    static void setOpacity(qreal level);
};

QT_BEGIN_NAMESPACE
inline uint qHash(const QPointer<QWidget> key)
{
    return reinterpret_cast<quint64>(key ? (&*key) : 0);
}
QT_END_NAMESPACE

#endif // QXTTOOLTIP_H
