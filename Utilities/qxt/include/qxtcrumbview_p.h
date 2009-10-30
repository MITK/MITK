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

#ifndef QXTCRUMBVIEW_P_INCLUDED
#define QXTCRUMBVIEW_P_INCLUDED

#include "qxtcrumbview.h"

QT_FORWARD_DECLARE_CLASS(QHBoxLayout)
class QxtCrumbViewList;
class QxtCrumbViewButton;
class QxtCrumbViewPrivate : public QObject, public QxtPrivate<QxtCrumbView> {
Q_OBJECT
public:
    QxtCrumbViewPrivate() {}

    QList<QModelIndex> crumbs;
    QList<QxtCrumbViewButton*> buttons;
    QHBoxLayout* buttonLayout;
    QxtCrumbViewList* view;

    void addCrumb(const QModelIndex& index); 

    QXT_DECLARE_PUBLIC(QxtCrumbView);

public slots:
    void buttonPressed();
};

#endif // QXTCRUMBVIEW_P_INCLUDED

