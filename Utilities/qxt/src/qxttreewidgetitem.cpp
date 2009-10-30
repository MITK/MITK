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
#include "qxttreewidgetitem.h"
#include "qxttreewidget.h"

/*!
    \class QxtTreeWidgetItem QxtTreeWidgetItem
    \ingroup QxtGui
    \brief An extended QTreeWidgetItem.

    QxtTreeWidgetItem provides means for offering check state change signals and
    convenience methods for testing and setting flags.

    \sa QxtTreeWidget
 */


QxtTreeWidgetItem::QxtTreeWidgetItem(int type)
        : QTreeWidgetItem(type)
{}

QxtTreeWidgetItem::QxtTreeWidgetItem(const QStringList& strings, int type)
        : QTreeWidgetItem(strings, type)
{}

QxtTreeWidgetItem::QxtTreeWidgetItem(QTreeWidget* parent, int type)
        : QTreeWidgetItem(parent, type)
{}

QxtTreeWidgetItem::QxtTreeWidgetItem(QTreeWidget* parent, const QStringList& strings, int type)
        : QTreeWidgetItem(parent, strings, type)
{}

QxtTreeWidgetItem::QxtTreeWidgetItem(QTreeWidget* parent, QTreeWidgetItem* preceding, int type)
        : QTreeWidgetItem(parent, preceding, type)
{}

QxtTreeWidgetItem::QxtTreeWidgetItem(QTreeWidgetItem* parent, int type)
        : QTreeWidgetItem(parent, type)
{}

QxtTreeWidgetItem::QxtTreeWidgetItem(QTreeWidgetItem* parent, const QStringList& strings, int type)
        : QTreeWidgetItem(parent, strings, type)
{}

QxtTreeWidgetItem::QxtTreeWidgetItem(QTreeWidgetItem* parent, QTreeWidgetItem* preceding, int type)
        : QTreeWidgetItem(parent, preceding, type)
{}

QxtTreeWidgetItem::QxtTreeWidgetItem(const QxtTreeWidgetItem& other)
        : QTreeWidgetItem(other)
{}

QxtTreeWidgetItem::~QxtTreeWidgetItem()
{}

/*!
    Returns \b true if the \a flag is set, otherwise \b false.

    \sa setFlag(), QTreeWidgetItem::flags(), Qt::ItemFlag
 */
bool QxtTreeWidgetItem::testFlag(Qt::ItemFlag flag) const
{
    return (flags() & flag);
}

/*!
    If \a enabled is \b true, the item \a flag is enabled; otherwise, it is disabled.

    \sa testFlag(), QTreeWidgetItem::setFlags(), Qt::ItemFlag
 */
void QxtTreeWidgetItem::setFlag(Qt::ItemFlag flag, bool enabled)
{
    if (enabled)
        setFlags(flags() | flag);
    else
        setFlags(flags() & ~flag);
}

/*!
    \reimp
 */
void QxtTreeWidgetItem::setData(int column, int role, const QVariant& value)
{
    if (role == Qt::CheckStateRole)
    {
        const Qt::CheckState newState = static_cast<Qt::CheckState>(value.toInt());
        const Qt::CheckState oldState = static_cast<Qt::CheckState>(data(column, role).toInt());

        QTreeWidgetItem::setData(column, role, value);

        if (newState != oldState)
        {
            QxtTreeWidget* tree = qobject_cast<QxtTreeWidget*>(treeWidget());
            if (tree)
            {
                emit tree->itemCheckStateChanged(this);
            }
        }
    }
    else
    {
        QTreeWidgetItem::setData(column, role, value);
    }
}
