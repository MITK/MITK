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
#include "qxtpushbutton.h"
#include <QAbstractTextDocumentLayout>
#include <QStyleOptionButton>
#include <QTextDocument>
#include <QStylePainter>
#include <QStyle>
#include <QMenu>

static const int Vertical_Mask = 0x02;

class QxtPushButtonPrivate : public QxtPrivate<QxtPushButton>
{
public:
    QXT_DECLARE_PUBLIC(QxtPushButton);
    QxtPushButtonPrivate();

    bool isRichText() const;
    QStyleOptionButton getStyleOption() const;

    QString text;
    Qxt::Rotation rot;
    Qt::TextFormat format;
    mutable QTextDocument* doc;
};

QxtPushButtonPrivate::QxtPushButtonPrivate() :
        rot(Qxt::NoRotation), format(Qt::PlainText), doc(0)
{}

bool QxtPushButtonPrivate::isRichText() const
{
#if QT_VERSION >= 0x040200
    bool rich = (format == Qt::RichText) || (format == Qt::AutoText && Qt::mightBeRichText(qxt_p().text()));
    if (rich && !doc)
    {
        doc = new QTextDocument(const_cast<QxtPushButton*>(&qxt_p()));
        doc->setUndoRedoEnabled(false);
    }
#else // QT_VERSION < 0x040200
    bool rich = false;
#endif // QT_VERSION
    return rich;
}

QStyleOptionButton QxtPushButtonPrivate::getStyleOption() const
{
    QStyleOptionButton option;
    option.initFrom(&qxt_p());
    if (rot & Vertical_Mask)
    {
        QSize size = option.rect.size();
        size.transpose();
        option.rect.setSize(size);
    }
    option.features = QStyleOptionButton::None;
    if (qxt_p().isFlat())
        option.features |= QStyleOptionButton::Flat;
    if (qxt_p().menu())
        option.features |= QStyleOptionButton::HasMenu;
    if (qxt_p().autoDefault() || qxt_p().isDefault())
        option.features |= QStyleOptionButton::AutoDefaultButton;
    if (qxt_p().isDefault())
        option.features |= QStyleOptionButton::DefaultButton;
    if (qxt_p().isDown() || (qxt_p().menu() && qxt_p().menu()->isVisible()))
        option.state |= QStyle::State_Sunken;
    if (qxt_p().isChecked())
        option.state |= QStyle::State_On;
    if (!qxt_p().isFlat() && !qxt_p().isDown())
        option.state |= QStyle::State_Raised;
    if (!isRichText())
    {
        option.text = qxt_p().text();
        option.icon = qxt_p().icon();
        option.iconSize = qxt_p().iconSize();
    }
    return option;
}

/*!
    \class QxtPushButton QxtPushButton
    \ingroup QxtGui
    \brief An extended QPushButton with rotation and rich text support.

    QxtPushButton is a QPushButton which can be rotated. In addition, QxtPushButton
    provides rich text support.

    \image html qxtpushbutton.png "QxtPushButton in action."
 */

/*!
    Constructs a new QxtPushButton with \a parent.
 */
QxtPushButton::QxtPushButton(QWidget* parent) : QPushButton(parent)
{
    QXT_INIT_PRIVATE(QxtPushButton);
}

/*!
    Constructs a new QxtPushButton with \a text and \a parent.
 */
QxtPushButton::QxtPushButton(const QString& text, QWidget* parent) : QPushButton(text, parent)
{
    QXT_INIT_PRIVATE(QxtPushButton);
}

/*!
    Constructs a new QxtPushButton with \a icon, \a text and \a parent.

    \note An icon is not rendered when using rich text.
 */
QxtPushButton::QxtPushButton(const QIcon& icon, const QString& text, QWidget* parent) : QPushButton(icon, text, parent)
{
    QXT_INIT_PRIVATE(QxtPushButton);
}

/*!
    Constructs a new QxtPushButton with \a rotation, \a text and \a parent.
 */
QxtPushButton::QxtPushButton(Qxt::Rotation rotation, const QString& text, QWidget* parent) : QPushButton(text, parent)
{
    QXT_INIT_PRIVATE(QxtPushButton);
    setRotation(rotation);
}

/*!
    Destructs the button.
 */
QxtPushButton::~QxtPushButton()
{}

/*!
    \property QxtPushButton::rotation
    \brief This property holds the rotation of the button

    The button is rotated according to this property.
    The default value is \b Qxt::NoRotation.

    \sa Qxt::Rotation
 */
Qxt::Rotation QxtPushButton::rotation() const
{
    return qxt_d().rot;
}

void QxtPushButton::setRotation(Qxt::Rotation rotation)
{
    if (qxt_d().rot != rotation)
    {
        qxt_d().rot = rotation;
        switch (rotation)
        {
        case Qxt::NoRotation:
        case Qxt::UpsideDown:
            setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
            break;

        case Qxt::Clockwise:
        case Qxt::CounterClockwise:
            setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
            break;

        default:
            // nothing to do
            break;
        }
        updateGeometry();
        update();
    }
}

/*!
    \property QxtPushButton::textFormat
    \brief This property holds the text format of the button

    Supported formats are \b Qt::PlainText, \b Qt::RichText and \b Qt::AutoText.

    The default format is \b Qt::PlainText.

    \note Rich text requires Qt 4.2 or newer.

    \sa Qt::TextFormat
 */
Qt::TextFormat QxtPushButton::textFormat() const
{
    return qxt_d().format;
}

void QxtPushButton::setTextFormat(Qt::TextFormat format)
{
    if (qxt_d().format != format)
    {
        qxt_d().format = format;
        if (!qxt_d().isRichText())
        {
            delete qxt_d().doc;
            qxt_d().doc = 0;
        }
        qxt_d().text.clear();
        update();
        updateGeometry();
    }
}

/*!
    \reimp
 */
QSize QxtPushButton::sizeHint() const
{
    QSize size;
    if (qxt_d().isRichText())
        size = qxt_d().doc->documentLayout()->documentSize().toSize();
    else
        size = QPushButton::sizeHint();

    if (qxt_d().rot & Vertical_Mask)
        size.transpose();
    return size;
}

/*!
    \reimp
 */
QSize QxtPushButton::minimumSizeHint() const
{
    return sizeHint();
}

/*!
    \reimp
 */
void QxtPushButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    // the only reliable way to detect text changes
    if (text() != qxt_d().text)
    {
        qxt_d().text = text();
        if (qxt_d().isRichText())
            qxt_d().doc->setHtml(qxt_d().text);
        updateGeometry();
    }

    QStylePainter painter(this);
    painter.rotate(qxt_d().rot);
    switch (qxt_d().rot)
    {
    case Qxt::UpsideDown:
        painter.translate(-width(), -height());
        break;

    case Qxt::Clockwise:
        painter.translate(0, -width());
        break;

    case Qxt::CounterClockwise:
        painter.translate(-height(), 0);
        break;

    default:
        // nothing to do
        break;
    }

    const QStyleOptionButton option = qxt_d().getStyleOption();
    painter.drawControl(QStyle::CE_PushButton, option);

#if QT_VERSION >= 0x040200
    if (qxt_d().isRichText())
    {
        int dx = 0;
        int dy = 0;
        if (option.state & (QStyle::State_On | QStyle::State_Sunken))
        {
            dx += style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &option, this);
            dy += style()->pixelMetric(QStyle::PM_ButtonShiftVertical, &option, this);
        }

        QRect area = rect();
        const QSizeF docSize = qxt_d().doc->documentLayout()->documentSize();
        if (qxt_d().rot & Vertical_Mask)
        {
            dx += static_cast<int>((height() - docSize.width())  / 2);
            dy += static_cast<int>((width()  - docSize.height()) / 2);
            painter.translate(dx, dy);

            QSize size = area.size();
            size.transpose();
            area.setSize(size);
        }
        else
        {
            dx += static_cast<int>((width()  - docSize.width())  / 2);
            dy += static_cast<int>((height() - docSize.height()) / 2);
            painter.translate(dx, dy);
        }
        qxt_d().doc->drawContents(&painter, area);
    }
#endif // QT_VERSION
}
