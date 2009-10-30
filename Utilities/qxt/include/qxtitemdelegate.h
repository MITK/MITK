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
#ifndef QXTITEMDELEGATE_H
#define QXTITEMDELEGATE_H

#include <QItemDelegate>
#include "qxtglobal.h"
#include "qxtnamespace.h"
#include "qxtpimpl.h"

class QxtItemDelegatePrivate;

class QXT_GUI_EXPORT QxtItemDelegate : public QItemDelegate
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(QxtItemDelegate);
    Q_PROPERTY(Qxt::DecorationStyle decorationStyle READ decorationStyle WRITE setDecorationStyle)
    Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode)
    Q_PROPERTY(QString progressTextFormat READ progressTextFormat WRITE setProgressTextFormat)
    Q_PROPERTY(bool progressTextVisible READ isProgressTextVisible WRITE setProgressTextVisible)

public:
    explicit QxtItemDelegate(QObject* parent = 0);
    virtual ~QxtItemDelegate();

    enum Role
    {
        ProgressValueRole = Qt::UserRole + 328,
        ProgressMinimumRole,
        ProgressMaximumRole
    };

    Qxt::DecorationStyle decorationStyle() const;
    void setDecorationStyle(Qxt::DecorationStyle style);

    Qt::TextElideMode elideMode() const;
    void setElideMode(Qt::TextElideMode mode);

    QString progressTextFormat() const;
    void setProgressTextFormat(const QString& format);

    bool isProgressTextVisible() const;
    void setProgressTextVisible(bool visible);

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

Q_SIGNALS:
    void editingStarted(const QModelIndex& index);
    void editingFinished(const QModelIndex& index);
};

#endif // QXTITEMDELEGATE_H
