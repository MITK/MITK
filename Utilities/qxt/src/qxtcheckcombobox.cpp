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
#include "qxtcheckcombobox.h"
#include "qxtcheckcombobox_p.h"
#include <QLineEdit>
#include <QKeyEvent>

QxtCheckComboBoxPrivate::QxtCheckComboBoxPrivate() : containerMousePress(false)
{
    separator = QLatin1String(",");
}

bool QxtCheckComboBoxPrivate::eventFilter(QObject* receiver, QEvent* event)
{
    switch (event->type())
    {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (receiver == &qxt_p() && (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down))
        {
            qxt_p().showPopup();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Enter ||
                 keyEvent->key() == Qt::Key_Return ||
                 keyEvent->key() == Qt::Key_Escape)
        {
            // it is important to call QComboBox implementation
            qxt_p().QComboBox::hidePopup();
            if (keyEvent->key() != Qt::Key_Escape)
                return true;
        }
    }
    case QEvent::MouseButtonPress:
        containerMousePress = (receiver == qxt_p().view()->window());
        break;
    case QEvent::MouseButtonRelease:
        containerMousePress = false;;
        break;
    default:
        break;
    }
    return false;
}

void QxtCheckComboBoxPrivate::updateCheckedItems()
{
    QStringList items = qxt_p().checkedItems();
    if (items.isEmpty())
        qxt_p().setEditText(defaultText);
    else
        qxt_p().setEditText(items.join(separator));

    // TODO: find a way to recalculate a meaningful size hint

    emit qxt_p().checkedItemsChanged(items);
}

void QxtCheckComboBoxPrivate::toggleCheckState(int index)
{
    QVariant value = qxt_p().itemData(index, Qt::CheckStateRole);
    if (value.isValid())
    {
        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        qxt_p().setItemData(index, (state == Qt::Unchecked ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
    }
}

QxtCheckComboModel::QxtCheckComboModel(QObject* parent)
        : QStandardItemModel(0, 1, parent) // rows,cols
{
}

Qt::ItemFlags QxtCheckComboModel::flags(const QModelIndex& index) const
{
    return QStandardItemModel::flags(index) | Qt::ItemIsUserCheckable;
}

QVariant QxtCheckComboModel::data(const QModelIndex& index, int role) const
{
    QVariant value = QStandardItemModel::data(index, role);
    if (index.isValid() && role == Qt::CheckStateRole && !value.isValid())
        value = Qt::Unchecked;
    return value;
}

bool QxtCheckComboModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    bool ok = QStandardItemModel::setData(index, value, role);
    if (ok && role == Qt::CheckStateRole)
    {
        emit dataChanged(index, index);
        emit checkStateChanged();
    }
    return ok;
}

/*!
    \class QxtCheckComboBox QxtCheckComboBox
    \ingroup QxtGui
    \brief An extended QComboBox with checkable items.

    QxtComboBox is a specialized combo box with checkable items.
    Checked items are collected together in the line edit.

    \image html qxtcheckcombobox.png "QxtCheckComboBox in Plastique style."
 */

/*!
    \enum QxtCheckComboBox::CheckMode

    This enum describes the check mode.

    \sa QxtCheckComboBox::checkMode
 */

/*!
    \var QxtCheckComboBox::CheckMode QxtCheckComboBox::CheckIndicator

    The check state changes only via the check indicator (like in item views).
 */

/*!
    \var QxtCheckComboBox::CheckMode QxtCheckComboBox::CheckWholeItem

    The check state changes via the whole item (like with a combo box).
 */

/*!
    \fn QxtCheckComboBox::checkedItemsChanged(const QStringList& items)

    This signal is emitted whenever the checked items have been changed.
 */

/*!
    Constructs a new QxtCheckComboBox with \a parent.
 */
QxtCheckComboBox::QxtCheckComboBox(QWidget* parent) : QComboBox(parent)
{
    QXT_INIT_PRIVATE(QxtCheckComboBox);
    setModel(new QxtCheckComboModel(this));
    connect(this, SIGNAL(activated(int)), &qxt_d(), SLOT(toggleCheckState(int)));
    connect(model(), SIGNAL(checkStateChanged()), &qxt_d(), SLOT(updateCheckedItems()));
    connect(model(), SIGNAL(rowsInserted(const QModelIndex &, int, int)), &qxt_d(), SLOT(updateCheckedItems()));
    connect(model(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)), &qxt_d(), SLOT(updateCheckedItems()));

    // read-only contents
    QLineEdit* lineEdit = new QLineEdit(this);
    lineEdit->setReadOnly(true);
    setLineEdit(lineEdit);
    setInsertPolicy(QComboBox::NoInsert);

    view()->installEventFilter(&qxt_d());
    view()->window()->installEventFilter(&qxt_d());
    view()->viewport()->installEventFilter(&qxt_d());
    this->installEventFilter(&qxt_d());
}

/*!
    Destructs the combo box.
 */
QxtCheckComboBox::~QxtCheckComboBox()
{
}

/*!
    \reimp
 */
void QxtCheckComboBox::hidePopup()
{
    if (qxt_d().containerMousePress)
        QComboBox::hidePopup();
}

/*!
    Returns the check state of the item at \a index.
 */
Qt::CheckState QxtCheckComboBox::itemCheckState(int index) const
{
    return static_cast<Qt::CheckState>(itemData(index, Qt::CheckStateRole).toInt());
}

/*!
    Sets the check state of the item at \a index to \a state.
 */
void QxtCheckComboBox::setItemCheckState(int index, Qt::CheckState state)
{
    setItemData(index, state, Qt::CheckStateRole);
}

/*!
    \property QxtCheckComboBox::checkedItems
    \brief This property holds the checked items.
 */
QStringList QxtCheckComboBox::checkedItems() const
{
    QStringList items;
    if (model())
    {
        QModelIndex index = model()->index(0, modelColumn(), rootModelIndex());
        QModelIndexList indexes = model()->match(index, Qt::CheckStateRole, Qt::Checked, -1, Qt::MatchExactly);
        foreach(const QModelIndex& index, indexes)
        items += index.data().toString();
    }
    return items;
}

void QxtCheckComboBox::setCheckedItems(const QStringList& items)
{
    // not the most efficient solution but most likely nobody
    // will put too many items into a combo box anyway so...
    foreach(const QString& text, items)
    {
        const int index = findText(text);
        setItemCheckState(index, index != -1 ? Qt::Checked : Qt::Unchecked);
    }
}

/*!
    \property QxtCheckComboBox::defaultText
    \brief This property holds the default text.

    The default text is shown when there are no checked items.
    The default value is an empty string.
 */
QString QxtCheckComboBox::defaultText() const
{
    return qxt_d().defaultText;
}

void QxtCheckComboBox::setDefaultText(const QString& text)
{
    if (qxt_d().defaultText != text)
    {
        qxt_d().defaultText = text;
        qxt_d().updateCheckedItems();
    }
}

/*!
    \property QxtCheckComboBox::separator
    \brief This property holds the default separator.

    The checked items are joined together with the separator string.
    The default value is a comma (",").
 */
QString QxtCheckComboBox::separator() const
{
    return qxt_d().separator;
}

void QxtCheckComboBox::setSeparator(const QString& separator)
{
    if (qxt_d().separator != separator)
    {
        qxt_d().separator = separator;
        qxt_d().updateCheckedItems();
    }
}
