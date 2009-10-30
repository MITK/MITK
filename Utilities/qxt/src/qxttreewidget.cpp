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
#include "qxttreewidget.h"
#include "qxttreewidget_p.h"
#include "qxtitemdelegate.h"
#include <QHeaderView>

QxtTreeWidgetPrivate::QxtTreeWidgetPrivate()
{}

QxtItemDelegate* QxtTreeWidgetPrivate::delegate() const
{
    QxtItemDelegate* del = qobject_cast<QxtItemDelegate*>(qxt_p().itemDelegate());
    Q_ASSERT(del);
    return del;
}

void QxtTreeWidgetPrivate::informStartEditing(const QModelIndex& index)
{
    QTreeWidgetItem* item = qxt_p().itemFromIndex(index);
    Q_ASSERT(item);
    emit qxt_p().itemEditingStarted(item);
}

void QxtTreeWidgetPrivate::informFinishEditing(const QModelIndex& index)
{
    QTreeWidgetItem* item = qxt_p().itemFromIndex(index);
    Q_ASSERT(item);
    emit qxt_p().itemEditingFinished(item);
}

void QxtTreeWidgetPrivate::expandCollapse(QTreeWidgetItem* item)
{
    if (item && !item->parent() && delegate()->decorationStyle() != Qxt::NoDecoration)
        qxt_p().setItemExpanded(item, !qxt_p().isItemExpanded(item));
}

/*!
    \class QxtTreeWidget QxtTreeWidget
    \ingroup QxtGui
    \brief An extended QTreeWidget with additional signals.

    QxtTreeWidget offers an optional top level item decoration
    and a few most commonly requested signals.

    \image html qxttreewidget.png "QxtTreeWidget with Qxt::Menulike and Qxt::Buttonlike decoration styles, respectively."
 */

/*!
    \fn QxtTreeWidget::itemEditingStarted(QTreeWidgetItem* item)

    This signal is emitted after the editing of \a item has been started.

    \sa itemEditingFinished()
 */

/*!
    \fn QxtTreeWidget::itemEditingFinished(QTreeWidgetItem* item)

    This signal is emitted after the editing of \a item has been finished.

    \sa itemEditingStarted()
 */

/*!
    \fn QxtTreeWidget::itemCheckStateChanged(QxtTreeWidgetItem* item)

    This signal is emitted whenever the check state of \a item has changed.

    \note Use QxtTreeWidgetItem in order to enable this feature.

    \sa QxtTreeWidgetItem, QTreeWidgetItem::checkState()
 */

/*!
    Constructs a new QxtTreeWidget with \a parent.
 */
QxtTreeWidget::QxtTreeWidget(QWidget* parent) : QTreeWidget(parent)
{
    QXT_INIT_PRIVATE(QxtTreeWidget);
    QxtItemDelegate* delegate = new QxtItemDelegate(this);
    connect(delegate, SIGNAL(editingStarted(const QModelIndex&)),
            &qxt_d(), SLOT(informStartEditing(const QModelIndex&)));
    connect(delegate, SIGNAL(editingFinished(const QModelIndex&)),
            &qxt_d(), SLOT(informFinishEditing(const QModelIndex&)));
    connect(this, SIGNAL(itemPressed(QTreeWidgetItem*, int)),
            &qxt_d(), SLOT(expandCollapse(QTreeWidgetItem*)));
    setItemDelegate(delegate);
}

/*!
    Destructs the tree widget.
 */
QxtTreeWidget::~QxtTreeWidget()
{}

/*!
    \property QxtTreeWidget::decorationStyle
    \brief This property holds the top level item decoration style

    Top level items are decorated according to this property.
    The default value is \b Qxt::NoDecoration.

    \note Setting the property to anything else than \b Qxt::NoDecoration
    hides the header and sets \b QTreeView::rootIsDecorated to \b false
    (to avoid multiple branch indicators).

    \sa Qxt::DecorationStyle QTreeView::rootIsDecorated
 */
Qxt::DecorationStyle QxtTreeWidget::decorationStyle() const
{
    return qxt_d().delegate()->decorationStyle();
}

void QxtTreeWidget::setDecorationStyle(Qxt::DecorationStyle style)
{
    if (qxt_d().delegate()->decorationStyle() != style)
    {
        qxt_d().delegate()->setDecorationStyle(style);

        if (style != Qxt::NoDecoration)
        {
            setRootIsDecorated(false);
            header()->hide();
        }
        reset();
    }
}

/*!
    \property QxtTreeWidget::elideMode
    \brief This property holds the text elide mode

    The text of a decorated top level item is elided according to this property.
    The default value is \b Qt::ElideMiddle.

    \note The property has effect only for decorated top level items.

    \sa decorationStyle, Qt::TextElideMode
 */
Qt::TextElideMode QxtTreeWidget::elideMode() const
{
    return qxt_d().delegate()->elideMode();
}

void QxtTreeWidget::setElideMode(Qt::TextElideMode mode)
{
    if (qxt_d().delegate()->elideMode() != mode)
    {
        qxt_d().delegate()->setElideMode(mode);
        reset();
    }
}
