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
#include "qxtgroupbox.h"

#include <QChildEvent>

class QxtGroupBoxPrivate : public QxtPrivate<QxtGroupBox>
{
public:
    QXT_DECLARE_PUBLIC(QxtGroupBox);

    QxtGroupBoxPrivate();
    bool collapsive;
    bool flat; // so we can restore it when expanding
};

QxtGroupBoxPrivate::QxtGroupBoxPrivate() : collapsive(true), flat(false)
{}


/*!
    \class QxtGroupBox QxtGroupBox
    \ingroup QxtGui
    \brief A collapsive and checkable QGroupBox.

    QxtGroupBox is a checkable group box automatically expanding/collapsing
    its content according to the check state. QxtGroupBox shows its children
    when checked and hides its children when unchecked.

    \image html qxtgroupbox.png "Two QxtGroupBoxes - an expanded and a collapsed - on top of each other."
 */

/*!
    Constructs a new QxtGroupBox with \a parent.
 */
QxtGroupBox::QxtGroupBox(QWidget* parent)
        : QGroupBox(parent)
{
    QXT_INIT_PRIVATE(QxtGroupBox);
    setCheckable(true);
    setChecked(true);
    connect(this, SIGNAL(toggled(bool)), this, SLOT(setExpanded(bool)));

}

/*!
    Constructs a new QxtGroupBox with \a title and \a parent.
 */
QxtGroupBox::QxtGroupBox(const QString& title, QWidget* parent)
        : QGroupBox(title, parent)
{
    QXT_INIT_PRIVATE(QxtGroupBox);
    setCheckable(true);
    setChecked(true);
    connect(this, SIGNAL(toggled(bool)), this, SLOT(setExpanded(bool)));
}

/*!
    Destructs the group box.
 */
QxtGroupBox::~QxtGroupBox()
{}

/*!
    \property QxtGroupBox::collapsive
    \brief This property holds whether the group box is collapsive
 */
bool QxtGroupBox::isCollapsive() const
{
    return qxt_d().collapsive;
}

void QxtGroupBox::setCollapsive(bool enable)
{
    if (qxt_d().collapsive != enable)
    {
        qxt_d().collapsive = enable;
        if (!enable)
            setExpanded(true);
        else if (!isChecked())
            setExpanded(false);
    }
}

/*!
    Sets the group box \a collapsed.

    A collapsed group box hides its children.

    \sa setExpanded(), QGroupBox::toggled()
 */
void QxtGroupBox::setCollapsed(bool collapsed)
{
    setExpanded(!collapsed);
}

/*!
    Sets the group box \a expanded.

    An expanded group box shows its children.

    \sa setCollapsed(), QGroupBox::toggled()
 */
void QxtGroupBox::setExpanded(bool expanded)
{
    if (qxt_d().collapsive || expanded)
    {
        // show/hide direct children
        foreach(QObject* child, children())
        {
            if (child->isWidgetType())
                static_cast<QWidget*>(child)->setVisible(expanded);
        }
        if (expanded) {
          setFlat(qxt_d().flat);
        } else {
          qxt_d().flat = isFlat();
          setFlat(true);
        }
    }
}

/*!
    \reimp
 */
void QxtGroupBox::childEvent(QChildEvent* event)
{
    QObject* child = event->child();
    if (event->added() && child->isWidgetType())
    {
        QWidget* widget = static_cast<QWidget*>(child);
        if (qxt_d().collapsive && !isChecked())
            widget->hide();
    }
}
