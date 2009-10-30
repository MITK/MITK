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
#include "qxtletterboxwidget.h"
#include "qxtletterboxwidget_p.h"
#include <QLayout>
#include <QStyle>

QxtLetterBoxWidgetPrivate::QxtLetterBoxWidgetPrivate() : center(0), timer(), margin(0)
{
    timer.setSingleShot(true);
}

/*!
    \class QxtLetterBoxWidget QxtLetterBoxWidget
    \ingroup QxtGui
    \brief A letter box widget.

    QxtLetterBoxWidget preserves the aspect ratio of its content widget.

    Use \b QWidget::sizeIncrement to define the aspect ratio:
    \code
    QxtLetterBoxWidget letterBox;
    widget->setSizeIncrement(16, 9);
    letterBox.setWidget(widget);
    letterBox.show();
    \endcode
 */

/*!
    Constructs a new QxtLetterBoxWidget with \a parent.
 */
QxtLetterBoxWidget::QxtLetterBoxWidget(QWidget* parent) : QFrame(parent)
{
    QXT_INIT_PRIVATE(QxtLetterBoxWidget);
    connect(&qxt_d().timer, SIGNAL(timeout()), this, SLOT(resizeWidget()));
}

/*!
    Destructs the group box.
 */
QxtLetterBoxWidget::~QxtLetterBoxWidget()
{
}

/*!
    \property QxtLetterBoxWidget::backgroundColor
    \brief This property holds the background color

    This property is provided for convenience.
    This property corresponds to \b QPalette::Window.

    \note Setting or clearing the property also sets the property
    \b QWidget::autoFillBackground as \b true or \b false, respectively.
 */
QColor QxtLetterBoxWidget::backgroundColor() const
{
    return palette().color(QPalette::Window);
}

void QxtLetterBoxWidget::setBackgroundColor(const QColor& color)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, color);
    setPalette(pal);
    setAutoFillBackground(true);
}

void QxtLetterBoxWidget::clearBackgroundColor()
{
    setBackgroundColor(QColor());
    setAutoFillBackground(false);
}

/*!
    \property QxtLetterBoxWidget::margin
    \brief This property holds the margin

    The default value is \b 0.
 */
int QxtLetterBoxWidget::margin() const
{
    return qxt_d().margin;
}

void QxtLetterBoxWidget::setMargin(int margin)
{
    if (qxt_d().margin != margin)
    {
        qxt_d().margin = margin;
        resizeWidget();
    }
}

/*!
    Returns the widget for the letter box.
    This function returns zero if the widget has not been set.

    \sa setWidget()
 */
QWidget* QxtLetterBoxWidget::widget() const
{
    return qxt_d().center;
}

/*!
    Sets the given widget to be the letter box's widget.

    \note QxtLetterBox takes ownership of the widget pointer
    and deletes it at the appropriate time.

    \sa widget()
 */
void QxtLetterBoxWidget::setWidget(QWidget* widget)
{
    if (qxt_d().center && qxt_d().center != widget)
        qxt_d().center->deleteLater();
    qxt_d().center = widget;
    if (widget)
    {
        widget->setParent(this);
        resizeWidget();
    }
}

/*!
    \property QxtLetterBoxWidget::resizeDelay
    \brief This property holds the delay of resize

    The default value is \b 0 which means immediate resize.

    Using a short resize delay might be useful if the
    widget is complex and resizing it is expensive.
 */
uint QxtLetterBoxWidget::resizeDelay() const
{
    return qxt_d().timer.interval();
}

void QxtLetterBoxWidget::setResizeDelay(uint delay)
{
    qxt_d().timer.setInterval(delay);
}

/*!
    Resizes the content widget.
 */
void QxtLetterBoxWidget::resizeWidget()
{
    if (qxt_d().center)
    {
        QSize s = qxt_d().center->sizeIncrement();
        s.scale(size(), Qt::KeepAspectRatio);
        s -= QSize(2 * qxt_d().margin, 2 * qxt_d().margin);
        s = QLayout::closestAcceptableSize(qxt_d().center, s);
        QRect r = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, s, rect());
        qxt_d().center->setGeometry(r);
    }
}

/*!
    \reimp
 */
void QxtLetterBoxWidget::resizeEvent(QResizeEvent* event)
{
    QFrame::resizeEvent(event);
    if (resizeDelay() > 0)
        qxt_d().timer.start();
    else
        resizeWidget();
}
