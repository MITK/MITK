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
#ifndef QXTPUSHBUTTON_H
#define QXTPUSHBUTTON_H

#include <QPushButton>
#include "qxtnamespace.h"
#include "qxtglobal.h"
#include "qxtpimpl.h"

class QxtPushButtonPrivate;

class QXT_GUI_EXPORT QxtPushButton : public QPushButton
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(QxtPushButton);
    Q_PROPERTY(Qxt::Rotation rotation READ rotation WRITE setRotation)
    Q_PROPERTY(Qt::TextFormat textFormat READ textFormat WRITE setTextFormat)

public:
    explicit QxtPushButton(QWidget* parent = 0);
    explicit QxtPushButton(const QString& text, QWidget* parent = 0);
    explicit QxtPushButton(const QIcon& icon, const QString& text, QWidget* parent = 0);
    explicit QxtPushButton(Qxt::Rotation rotation, const QString& text, QWidget* parent = 0);
    virtual ~QxtPushButton();

    Qxt::Rotation rotation() const;
    void setRotation(Qxt::Rotation rotation);

    Qt::TextFormat textFormat() const;
    void setTextFormat(Qt::TextFormat format);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent* event);
};

#endif // QXTPUSHBUTTON_H
