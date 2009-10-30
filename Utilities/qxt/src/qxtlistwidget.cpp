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
#include "qxtlistwidget.h"
#include "qxtlistwidget_p.h"
#include "qxtitemdelegate.h"

QxtListWidgetPrivate::QxtListWidgetPrivate()
{}

void QxtListWidgetPrivate::informStartEditing(const QModelIndex& index)
{
    QListWidgetItem* item = qxt_p().itemFromIndex(index);
    Q_ASSERT(item);
    emit qxt_p().itemEditingStarted(item);
}

void QxtListWidgetPrivate::informFinishEditing(const QModelIndex& index)
{
    QListWidgetItem* item = qxt_p().itemFromIndex(index);
    Q_ASSERT(item);
    emit qxt_p().itemEditingFinished(item);
}

/*!
    \class QxtListWidget QxtListWidget
    \ingroup QxtGui
    \brief An extended QListWidget with additional signals.

    QxtListWidget offers a few most commonly requested signals.

    \image html qxtlistwidget.png "QxtListWidget in Plastique style."
 */

/*!
    \fn QxtListWidget::itemEditingStarted(QListWidgetItem* item)

    This signal is emitted after the editing of \a item has been started.

    \sa itemEditingFinished()
 */

/*!
    \fn QxtListWidget::itemEditingFinished(QListWidgetItem* item)

    This signal is emitted after the editing of \a item has been finished.

    \sa itemEditingStarted()
 */

/*!
    \fn QxtListWidget::itemCheckStateChanged(QxtListWidgetItem* item)

    This signal is emitted whenever the check state of \a item has changed.

    \note Use QxtListWidgetItem in order to enable this feature.

    \sa QxtListWidgetItem, QListWidgetItem::checkState()
 */

/*!
    Constructs a new QxtListWidget with \a parent.
 */
QxtListWidget::QxtListWidget(QWidget* parent) : QListWidget(parent)
{
    QXT_INIT_PRIVATE(QxtListWidget);
    QxtItemDelegate* delegate = new QxtItemDelegate(this);
    connect(delegate, SIGNAL(editingStarted(const QModelIndex&)),
            &qxt_d(), SLOT(informStartEditing(const QModelIndex&)));
    connect(delegate, SIGNAL(editingFinished(const QModelIndex&)),
            &qxt_d(), SLOT(informFinishEditing(const QModelIndex&)));
    setItemDelegate(delegate);
}

/*!
    Destructs the list widget.
 */
QxtListWidget::~QxtListWidget()
{}
