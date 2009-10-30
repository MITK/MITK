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
#ifndef QXTLABEL_H
#define QXTLABEL_H

#include <QFrame>
#include "qxtnamespace.h"
#include "qxtglobal.h"
#include "qxtpimpl.h"

class QxtLabelPrivate;

class QXT_GUI_EXPORT QxtLabel : public QFrame
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(QxtLabel);
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode)
    Q_PROPERTY(Qxt::Rotation rotation READ rotation WRITE setRotation)

public:
    explicit QxtLabel(QWidget* parent = 0, Qt::WindowFlags flags = 0);
    explicit QxtLabel(const QString& text, QWidget* parent = 0, Qt::WindowFlags flags = 0);
    virtual ~QxtLabel();

    QString text() const;

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment alignment);

    Qt::TextElideMode elideMode() const;
    void setElideMode(Qt::TextElideMode mode);

    Qxt::Rotation rotation() const;
    void setRotation(Qxt::Rotation rotation);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

public Q_SLOTS:
    void setText(const QString& text);

Q_SIGNALS:
    void clicked();
    void textChanged(const QString& text);

protected:
    virtual void changeEvent(QEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void paintEvent(QPaintEvent* event);
};

#endif // QXTLABEL_H
