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
#ifndef QXTITEMEDITORCREATORBASE_H
#define QXTITEMEDITORCREATORBASE_H

#include <QHash>
#include <QWidget>
#include <QVariant>
#include <QByteArray>
#include <QHashIterator>

/*!
    \class QxtItemEditorCreatorBase QxtItemEditorCreatorBase
    \ingroup QxtGui
    \brief Provides means for introducing default property values to initialize an editor with.

    The QxtItemEditorCreator class provides the possibility to set default property values
    which are applied when initializing given editor.
 */

template <class T>
class QXT_GUI_EXPORT QxtItemEditorCreatorBase
{
public:
    /*!
        Returns the default value of \a property.
     */
    QVariant defaultPropertyValue(const QByteArray& property) const
    {
        return properties.value(property);
    }

    /*!
        Sets the default \a value for \a property.
     */
    void setDefaultPropertyValue(const QByteArray& property, const QVariant& value)
    {
        properties.insert(property, value);
    }

    /*!
        Initializes \a editor with introduced default property values.
     */
    inline QWidget* initializeEditor(QWidget* editor) const;

private:
    QHash<QByteArray, QVariant> properties;
};

template <class T>
Q_INLINE_TEMPLATE QWidget* QxtItemEditorCreatorBase<T>::initializeEditor(QWidget* editor) const
{
    QHashIterator<QByteArray, QVariant> i(properties);
    while (i.hasNext())
    {
        i.next();
        editor->setProperty(i.key(), i.value());
    }
    return editor;
}

#endif // QXTITEMEDITORCREATORBASE_H
