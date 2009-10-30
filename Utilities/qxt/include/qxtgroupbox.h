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
#ifndef QXTGROUPBOX_H
#define QXTGROUPBOX_H

#include <QGroupBox>
#include "qxtglobal.h"
#include "qxtpimpl.h"

class QxtGroupBoxPrivate;

class QXT_GUI_EXPORT QxtGroupBox : public QGroupBox
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(QxtGroupBox);
    Q_PROPERTY(bool collapsive READ isCollapsive WRITE setCollapsive)

public:
    explicit QxtGroupBox(QWidget* parent = 0);
    explicit QxtGroupBox(const QString& title, QWidget* parent = 0);
    virtual ~QxtGroupBox();

    bool isCollapsive() const;
    void setCollapsive(bool enabled);

public Q_SLOTS:
    void setCollapsed(bool collapsed = true);
    void setExpanded(bool expanded = true);

protected:
    virtual void childEvent(QChildEvent* event);
};

#endif // QXTGROUPBOX_H
