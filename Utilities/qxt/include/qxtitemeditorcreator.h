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
#ifndef QXTITEMEDITORCREATOR_H
#define QXTITEMEDITORCREATOR_H

#include <QItemEditorCreator>
#include "qxtitemeditorcreatorbase.h"
#include "qxtglobal.h"

/*!
    \class QxtItemEditorCreator QxtItemEditorCreator
    \ingroup QxtGui
    \brief An extended QItemEditorCreator with default values for user-chosen properties.

    The QxtItemEditorCreator class provides the possibility to set default values for
    user-chosen properties.

    Usage:
    \code
    QItemEditorFactory *factory = new QItemEditorFactory;

    QxtItemEditorCreator<QSpinBox>* spinBoxCreator = new QxtItemEditorCreator<QSpinBox>("value");
    QxtItemEditorCreator<QLineEdit>* lineEditCreator = new QxtItemEditorCreator<QLineEdit>("text");

    // spin boxes for degrees between -180 and 180
    spinBoxCreator->setDefaultPropertyValue("minimum", -180);
    spinBoxCreator->setDefaultPropertyValue("maximum", 180);

    // line edits for passwords
    lineEditCreator->setDefaultPropertyValue("echoMode", QLineEdit::Password);

    factory->registerEditor(QVariant::Int, spinBoxCreator);
    factory->registerEditor(QVariant::String, lineEditCreator);

    QItemEditorFactory::setDefaultFactory(factory);
    \endcode

    Setting default properties above makes sure that spin boxes have ranges from -180 to 180
    and line editors' echo mode is set to QLineEdit::Password.

    \sa QItemEditorCreator
 */

template <class T>
class QXT_GUI_EXPORT QxtItemEditorCreator : public QxtItemEditorCreatorBase<T>, public QItemEditorCreator<T>
{
public:
    /*!
        Constructs a new QxtItemEditorCreator with \a valuePropertyName.
     */
    inline QxtItemEditorCreator(const QByteArray& valuePropertyName)
            : QItemEditorCreator<T>(valuePropertyName)
    {
    }

    /*!
        \reimp
     */
    inline QWidget* createWidget(QWidget* parent) const
    {
        return initializeEditor(QItemEditorCreator<T>::createWidget(parent));
    }
};

#endif // QXTITEMEDITORCREATOR_H
