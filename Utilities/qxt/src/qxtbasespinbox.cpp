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
#include "qxtbasespinbox.h"

class QxtBaseSpinBoxPrivate : public QxtPrivate<QxtBaseSpinBox>
{
public:
    QXT_DECLARE_PUBLIC(QxtBaseSpinBox);
    QxtBaseSpinBoxPrivate();

    int base;
    bool upper;
};

QxtBaseSpinBoxPrivate::QxtBaseSpinBoxPrivate() : base(10), upper(false)
{
}

/*!
    \class QxtBaseSpinBox QxtBaseSpinBox
    \ingroup QxtGui
    \brief A spin box with support for numbers in base between 2 and 36.

    Example spinbox for hexadecimal input:
    \code
    QxtBaseSpinBox* spinBox = new QxtBaseSpinBox(16, this);
    spinBox->setPrefix("0x");
    spinBox->setValue(0xbabe);
    spinBox->setUpperCase(true);
    \endcode

    \image html qxtbasespinbox.png "QxtBaseSpinBox in action."

    \note Notice that QxtBaseSpinBox is not locale-aware.
 */

/*!
    \fn QxtBaseSpinBox::baseChanged(int base)

    This signal is emitted whenever the number base has changed.
 */

/*!
    Constructs a new QxtBaseSpinBox with \a parent. Base defaults to \b 10.
 */
QxtBaseSpinBox::QxtBaseSpinBox(QWidget* parent) : QSpinBox(parent)
{
    QXT_INIT_PRIVATE(QxtBaseSpinBox);
}

/*!
    Constructs a new QxtBaseSpinBox with \a base and \a parent.
 */
QxtBaseSpinBox::QxtBaseSpinBox(int base, QWidget* parent) : QSpinBox(parent)
{
    QXT_INIT_PRIVATE(QxtBaseSpinBox);
    qxt_d().base = base;
}

/*!
    Destructs the spin box.
 */
QxtBaseSpinBox::~QxtBaseSpinBox()
{
}

/*!
    \reimp
 */
void QxtBaseSpinBox::fixup(QString& input) const
{
    QString inputWithoutPrefix = input.mid(prefix().length());
    inputWithoutPrefix = qxt_d().upper ? inputWithoutPrefix.toUpper() : inputWithoutPrefix.toLower();
    input = prefix() + inputWithoutPrefix;
}

/*!
    \reimp
 */
QValidator::State QxtBaseSpinBox::validate(QString& input, int& pos) const
{
    // quick rejects
    const QString prefix = QSpinBox::prefix();
    const QString inputWithoutPrefix = input.mid(prefix.length());
    if (pos < prefix.length())
    {
        // do not let modify prefix
        return QValidator::Invalid;
    }
    else if (inputWithoutPrefix.isEmpty())
    {
        // allow empty input => intermediate
        return QValidator::Intermediate;
    }

    // Invalid:      input is invalid according to the string list
    // Intermediate: it is likely that a little more editing will make the input acceptable
    // Acceptable:   the input is valid.
    Q_UNUSED(pos);

    bool ok = false;
    const int min = minimum();
    const int max = maximum();
    const int number = inputWithoutPrefix.toInt(&ok, qxt_d().base);

    QValidator::State state = QValidator::Invalid;
    if (!ok)
    {
        // cannot convert => invalid
        state = QValidator::Invalid;
    }
    else if (number >= min && number <= max)
    {
        // converts ok, between boundaries => acceptable if case matches
        if (qxt_d().upper)
            return (input == prefix + inputWithoutPrefix.toUpper() ? QValidator::Acceptable : QValidator::Intermediate);
        else
            return (input == prefix + inputWithoutPrefix.toLower() ? QValidator::Acceptable : QValidator::Intermediate);
    }
    else
    {
        // converts ok, outside boundaries => intermediate
        state = QValidator::Intermediate;
    }
    return state;
}

/*!
    \property QxtBaseSpinBox::base
    \brief This property holds the number base.

    The base must be between 2 and 36.

    The default value is \b 10.
 */
int QxtBaseSpinBox::base() const
{
    return qxt_d().base;
}

void QxtBaseSpinBox::setBase(int base)
{
    if (base < 2 || base > 36)
        qWarning("QxtBaseSpinBox: base must be between 2 and 36");

    base = qBound(2, base, 36);
    if (qxt_d().base != base)
    {
        qxt_d().base = base;
        emit baseChanged(base);
        setValue(value());
    }
}

/*!
    \property QxtBaseSpinBox::upperCase
    \brief This property holds whether letters are shown in upper case.

    Naturally, this applies to only bases which can contain letters.

    The default value is \b false.
 */
bool QxtBaseSpinBox::isUpperCase() const
{
    return qxt_d().upper;
}

void QxtBaseSpinBox::setUpperCase(bool upperCase)
{
    if (qxt_d().upper != upperCase)
    {
        qxt_d().upper = upperCase;
        setValue(value());
    }
}

/*!
    \reimp
 */
QString QxtBaseSpinBox::textFromValue(int value) const
{
    QString text = QString::number(value, qxt_d().base);
    if (qxt_d().upper)
        return text.toUpper();
    return text;
}

/*!
    \reimp
 */
int QxtBaseSpinBox::valueFromText(const QString& text) const
{
    return text.toInt(0, qxt_d().base);
}
