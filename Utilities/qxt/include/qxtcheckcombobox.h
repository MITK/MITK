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
#ifndef QXTCHECKCOMBOBOX_H
#define QXTCHECKCOMBOBOX_H

#include <QComboBox>
#include "qxtnamespace.h"
#include "qxtglobal.h"
#include "qxtpimpl.h"

class QxtCheckComboBoxPrivate;

class QXT_GUI_EXPORT QxtCheckComboBox : public QComboBox
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(QxtCheckComboBox);
    Q_PROPERTY(QString separator READ separator WRITE setSeparator)
    Q_PROPERTY(QString defaultText READ defaultText WRITE setDefaultText)
    Q_PROPERTY(QStringList checkedItems READ checkedItems WRITE setCheckedItems)

public:
    explicit QxtCheckComboBox(QWidget* parent = 0);
    virtual ~QxtCheckComboBox();

    virtual void hidePopup();

    QString defaultText() const;
    void setDefaultText(const QString& text);

    Qt::CheckState itemCheckState(int index) const;
    void setItemCheckState(int index, Qt::CheckState state);

    QString separator() const;
    void setSeparator(const QString& separator);

    QStringList checkedItems() const;

public Q_SLOTS:
    void setCheckedItems(const QStringList& items);

Q_SIGNALS:
    void checkedItemsChanged(const QStringList& items);
};

#endif // QXTCHECKCOMBOBOX_H
