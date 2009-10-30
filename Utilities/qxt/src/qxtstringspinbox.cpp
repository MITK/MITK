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
#include "qxtstringspinbox.h"

class QxtStringSpinBoxPrivate : public QxtPrivate<QxtStringSpinBox>
{
public:
    QXT_DECLARE_PUBLIC(QxtStringSpinBox);
    int startsWith(const QString& start, QString& string) const;
    QStringList strings;
};

int QxtStringSpinBoxPrivate::startsWith(const QString& start, QString& string) const
{
    const int size = strings.size();
    for (int i = 0; i < size; ++i)
    {
        if (strings.at(i).startsWith(start, Qt::CaseInsensitive))
        {
            // found
            string = strings.at(i);
            return i;
        }
    }
    // not found
    return -1;
}

/*!
    \class QxtStringSpinBox QxtStringSpinBox
    \ingroup QxtGui
    \brief A spin box with string items.

    QxtStringSpinBox is spin box that takes strings. QxtStringSpinBox allows
    the user to choose a value by clicking the up and down buttons or by
    pressing Up or Down on the keyboard to increase or decrease the value
    currently displayed. The user can also type the value in manually.

    \image html qxtstringspinbox.png "QxtStringSpinBox in Cleanlooks style."
 */

/*!
    Constructs a new QxtStringSpinBox with \a parent.
 */
QxtStringSpinBox::QxtStringSpinBox(QWidget* pParent) : QSpinBox(pParent)
{
    setRange(0, 0);
}

/*!
    Destructs the spin box.
 */
QxtStringSpinBox::~QxtStringSpinBox()
{}

/*!
    \property QxtStringSpinBox::strings
    \brief This property holds the string items of the spin box.
 */
const QStringList& QxtStringSpinBox::strings() const
{
    return qxt_d().strings;
}

void QxtStringSpinBox::setStrings(const QStringList& strings)
{
    qxt_d().strings = strings;
    setRange(0, strings.size() - 1);
    if (!strings.isEmpty())
        setValue(0);
}

/*!
    \reimp
 */
void QxtStringSpinBox::fixup(QString& input) const
{
    // just attempt to change the input string to be valid according to the string list
    // no need to result in a valid string, callers of this function are responsible to
    // re-test the validity afterwards

    // try finding a string from the list which starts with input
    input = input.simplified();
    if (!input.isEmpty())
    {
        qxt_d().startsWith(input, input);
    }
}

/*!
    \reimp
 */
QValidator::State QxtStringSpinBox::validate(QString& input, int& pos) const
{
    // Invalid:  input is invalid according to the string list
    // Intermediate: it is likely that a little more editing will make the input acceptable
    //   (e.g. the user types "A" and stringlist contains "ABC")
    // Acceptable:  the input is valid.
    Q_UNUSED(pos);
    QString temp;
    QValidator::State state = QValidator::Invalid;
    if (qxt_d().strings.contains(input))
    {
        // exact match
        state = QValidator::Acceptable;
    }
    else if (input.isEmpty() || qxt_d().startsWith(input, temp) != -1)
    {
        // still empty or some string in the list starts with input
        state = QValidator::Intermediate;
    }
    // else invalid
    return state;
}

/*!
    \reimp
 */
QString QxtStringSpinBox::textFromValue(int value) const
{
    Q_ASSERT(qxt_d().strings.isEmpty() || (value >= 0 && value < qxt_d().strings.size()));
    return qxt_d().strings.isEmpty() ? QLatin1String("") : qxt_d().strings.at(value);
}

/*!
    \reimp
 */
int QxtStringSpinBox::valueFromText(const QString& text) const
{
    return qxt_d().strings.indexOf(text);
}
