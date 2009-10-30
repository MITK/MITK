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
#ifndef QXTPROGRESSLABEL_H
#define QXTPROGRESSLABEL_H

#include <QLabel>
#include "qxtglobal.h"
#include "qxtpimpl.h"

class QxtProgressLabelPrivate;

class QXT_GUI_EXPORT QxtProgressLabel : public QLabel
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(QxtProgressLabel);
    Q_PROPERTY(QString contentFormat READ contentFormat WRITE setContentFormat)
    Q_PROPERTY(QString timeFormat READ timeFormat WRITE setTimeFormat)
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval)

public:
    explicit QxtProgressLabel(QWidget* parent = 0, Qt::WindowFlags flags = 0);
    explicit QxtProgressLabel(const QString& text, QWidget* parent = 0, Qt::WindowFlags flags = 0);
    virtual ~QxtProgressLabel();

    QString contentFormat() const;
    void setContentFormat(const QString& format);

    QString timeFormat() const;
    void setTimeFormat(const QString& format);

    int updateInterval() const;
    void setUpdateInterval(int msecs);

public Q_SLOTS:
    void setValue(int value);
    void refresh();
    void restart();

    virtual void timerEvent(QTimerEvent* event);
};

#endif // QXTPROGRESSLABEL_H
