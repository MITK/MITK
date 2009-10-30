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
#include "qxtlistwidgetitem.h"
#include "qxtlistwidget.h"

/*!
    \class QxtListWidgetItem QxtListWidgetItem
    \ingroup QxtGui
    \brief An extended QListWidgetItem.

    QxtListWidgetItem provides means for offering check state change signals and
    convenience methods for testing and setting flags.

    \sa QxtListWidget
 */


QxtListWidgetItem::QxtListWidgetItem(QListWidget* parent, int type)
        : QListWidgetItem(parent, type)
{}

QxtListWidgetItem::QxtListWidgetItem(const QString& text, QListWidget* parent, int type)
        : QListWidgetItem(text, parent, type)
{}

QxtListWidgetItem::QxtListWidgetItem(const QIcon& icon, const QString& text, QListWidget* parent, int type)
        : QListWidgetItem(icon, text, parent, type)
{}

QxtListWidgetItem::QxtListWidgetItem(const QxtListWidgetItem& other)
        : QListWidgetItem(other)
{}

QxtListWidgetItem::~QxtListWidgetItem()
{}

/*!
    Returns \b true if the \a flag is set, otherwise \b false.

    \sa setFlag(), QListWidgetItem::flags(), Qt::ItemFlag
 */
bool QxtListWidgetItem::testFlag(Qt::ItemFlag flag) const
{
    return (flags() & flag);
}

/*!
    If \a enabled is \b true, the item \a flag is enabled; otherwise, it is disabled.

    \sa testFlag(), QListWidgetItem::setFlags(), Qt::ItemFlag
 */
void QxtListWidgetItem::setFlag(Qt::ItemFlag flag, bool enabled)
{
    if (enabled)
        setFlags(flags() | flag);
    else
        setFlags(flags() & ~flag);
}

/*!
    \reimp
 */
void QxtListWidgetItem::setData(int role, const QVariant& value)
{
    if (role == Qt::CheckStateRole)
    {
        const Qt::CheckState newState = static_cast<Qt::CheckState>(value.toInt());
        const Qt::CheckState oldState = static_cast<Qt::CheckState>(data(role).toInt());

        QListWidgetItem::setData(role, value);

        if (newState != oldState)
        {
            QxtListWidget* list = qobject_cast<QxtListWidget*>(listWidget());
            if (list)
            {
                emit list->itemCheckStateChanged(this);
            }
        }
    }
    else
    {
        QListWidgetItem::setData(role, value);
    }
}
