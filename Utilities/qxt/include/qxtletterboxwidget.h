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
#ifndef QXTLETTERBOXWIDGET_H
#define QXTLETTERBOXWIDGET_H

#include <QFrame>
#include "qxtglobal.h"
#include "qxtpimpl.h"

class QxtLetterBoxWidgetPrivate;

class QXT_GUI_EXPORT QxtLetterBoxWidget : public QFrame
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(QxtLetterBoxWidget);
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor RESET clearBackgroundColor)
    Q_PROPERTY(int margin READ margin WRITE setMargin)
    Q_PROPERTY(uint resizeDelay READ resizeDelay WRITE setResizeDelay)

public:
    QxtLetterBoxWidget(QWidget* parent = 0);
    ~QxtLetterBoxWidget();

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor& color);
    void clearBackgroundColor();

    int margin() const;
    void setMargin(int margin);

    QWidget* widget() const;
    void setWidget(QWidget* widget);

    uint resizeDelay() const;
    void setResizeDelay(uint delay);

public Q_SLOTS:
    void resizeWidget();

protected:
    void resizeEvent(QResizeEvent* event);
};

#endif // QXTLETTERBOXWIDGET_H
