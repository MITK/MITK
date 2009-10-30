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
#ifndef QXTTREEWIDGET_H
#define QXTTREEWIDGET_H

#include <QTreeWidget>
#include "qxttreewidgetitem.h"
#include "qxtnamespace.h"
#include "qxtglobal.h"
#include "qxtpimpl.h"

class QxtTreeWidgetPrivate;

class QXT_GUI_EXPORT QxtTreeWidget : public QTreeWidget
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(QxtTreeWidget);
    Q_PROPERTY(Qxt::DecorationStyle decorationStyle READ decorationStyle WRITE setDecorationStyle)
    Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode)
    friend class QxtTreeWidgetItem;

public:
    explicit QxtTreeWidget(QWidget* parent = 0);
    virtual ~QxtTreeWidget();

    Qxt::DecorationStyle decorationStyle() const;
    void setDecorationStyle(Qxt::DecorationStyle style);

    Qt::TextElideMode elideMode() const;
    void setElideMode(Qt::TextElideMode mode);

Q_SIGNALS:
    void itemEditingStarted(QTreeWidgetItem* item);
    void itemEditingFinished(QTreeWidgetItem* item);
    void itemCheckStateChanged(QxtTreeWidgetItem* item);
};

#endif // QXTTREEWIDGET_H
