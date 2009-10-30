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

#ifndef QXTCRUMBVIEW_H
#define QXTCRUMBVIEW_H

#include "qxtglobal.h"
#include "qxtpimpl.h"
#include <QAbstractItemView>

QT_FORWARD_DECLARE_CLASS(QListView)
class QxtCrumbViewPrivate;
class QXT_GUI_EXPORT QxtCrumbView : public QAbstractItemView {
Q_OBJECT
public:
    QxtCrumbView(QWidget* parent = 0);

    virtual void setModel(QAbstractItemModel* model);

    virtual QModelIndex indexAt(const QPoint& point) const;
    virtual void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible);
    virtual QRect visualRect(const QModelIndex& index) const;

    void setItemDelegate(QAbstractItemDelegate* delegate);

    QListView* listView() const;

public slots:
    virtual void reset();
    void back();
    void enterTree(const QModelIndex& index); 

protected:
    virtual int horizontalOffset() const;
    virtual int verticalOffset() const;
    virtual bool isIndexHidden(const QModelIndex& index) const;
    virtual QModelIndex moveCursor(CursorAction action, Qt::KeyboardModifiers mods);
    virtual void setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags flags);
    virtual QRegion visualRegionForSelection(const QItemSelection& selection) const;

private:
    QXT_DECLARE_PRIVATE(QxtCrumbView)
};

#endif
