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
#ifndef QXTHEADERVIEW_H
#define QXTHEADERVIEW_H

#include <QHeaderView>
#include "qxtglobal.h"
#include "qxtpimpl.h"

class QxtHeaderViewPrivate;

class QXT_GUI_EXPORT QxtHeaderView : public QHeaderView
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(QxtHeaderView);
    Q_PROPERTY(bool proportionalSectionSizes READ hasProportionalSectionSizes WRITE setProportionalSectionSizes)

public:
    explicit QxtHeaderView(Qt::Orientation orientation, QWidget* parent = 0);

    bool hasProportionalSectionSizes() const;
    void setProportionalSectionSizes(bool enabled);

    int sectionStretchFactor(int logicalIndex) const;
    void setSectionStretchFactor(int logicalIndex, int factor);

protected:
    void resizeEvent(QResizeEvent* event);
};

#endif // QXTHEADERVIEW_H
