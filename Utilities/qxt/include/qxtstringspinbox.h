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
#ifndef QXTSTRINGSPINBOX_H
#define QXTSTRINGSPINBOX_H

#include <QSpinBox>
#include "qxtglobal.h"
#include "qxtpimpl.h"

class QxtStringSpinBoxPrivate;

class QXT_GUI_EXPORT QxtStringSpinBox : public QSpinBox
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(QxtStringSpinBox);
    Q_PROPERTY(QStringList strings READ strings WRITE setStrings)

public:
    explicit QxtStringSpinBox(QWidget* pParent = 0);
    virtual ~QxtStringSpinBox();

    const QStringList& strings() const;
    void setStrings(const QStringList& strings);

    virtual void fixup(QString& input) const;
    virtual QValidator::State validate(QString& input, int& pos) const;

protected:
    virtual QString textFromValue(int value) const;
    virtual int valueFromText(const QString& text) const;
};

#endif // QXTSTRINGSPINBOX_H
