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
#include "qxttooltip.h"
#include "qxttooltip_p.h"
#include <QStyleOptionFrame>
#include <QDesktopWidget>
#include <QStylePainter>
#include <QApplication>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QToolTip>
#include <QPalette>
#include <QTimer>
#include <QFrame>
#include <QStyle>
#include <QHash>

static const Qt::WindowFlags FLAGS = Qt::ToolTip;

QxtToolTipPrivate* QxtToolTipPrivate::self = 0;

QxtToolTipPrivate* QxtToolTipPrivate::instance()
{
    if (!self)
        self = new QxtToolTipPrivate();
    return self;
}

QxtToolTipPrivate::QxtToolTipPrivate() : QWidget(qApp->desktop(), FLAGS)
{
    setWindowFlags(FLAGS);
    vbox = new QVBoxLayout(this);
    setPalette(QToolTip::palette());
    setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / 255.0);
    layout()->setMargin(style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0, this));
    qApp->installEventFilter(this);
}

QxtToolTipPrivate::~QxtToolTipPrivate()
{
    qApp->removeEventFilter(this); // not really necessary but rather for completeness :)
    self = 0;
}

void QxtToolTipPrivate::show(const QPoint& pos, QWidget* tooltip, QWidget* parent, const QRect& rect)
{
    Q_ASSERT(tooltip && parent);
    if (!isVisible())
    {
        int scr = 0;
        if (QApplication::desktop()->isVirtualDesktop())
            scr = QApplication::desktop()->screenNumber(pos);
        else
            scr = QApplication::desktop()->screenNumber(this);
        setParent(QApplication::desktop()->screen(scr));
        setWindowFlags(FLAGS);
        setToolTip(tooltip);
        currentParent = parent;
        currentRect = rect;
        move(calculatePos(scr, pos));
        QWidget::show();
    }
}

void QxtToolTipPrivate::setToolTip(QWidget* tooltip)
{
    for (int i = 0; i < vbox->count(); ++i)
    {
        QLayoutItem* item = layout()->takeAt(i);
        if (item->widget())
            item->widget()->hide();
    }
    vbox->addWidget(tooltip);
    tooltip->show();
}

void QxtToolTipPrivate::enterEvent(QEvent* event)
{
    Q_UNUSED(event);
    hideLater();
}

void QxtToolTipPrivate::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QStylePainter painter(this);
    QStyleOptionFrame opt;
    opt.initFrom(this);
    painter.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
}

bool QxtToolTipPrivate::eventFilter(QObject* object, QEvent* event)
{
    switch (event->type())
    {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    {
        // accept only modifiers
        const QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        const int key = keyEvent->key();
        const Qt::KeyboardModifiers mods = keyEvent->modifiers();
        if ((mods & Qt::KeyboardModifierMask) ||
                (key == Qt::Key_Shift || key == Qt::Key_Control ||
                 key == Qt::Key_Alt || key == Qt::Key_Meta))
            break;
    }
    case QEvent::Leave:
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::FocusIn:
    case QEvent::FocusOut:
    case QEvent::Wheel:
        hideLater();
        break;

    case QEvent::MouseMove:
    {
        const QPoint pos = static_cast<QMouseEvent*>(event)->pos();
        if (!currentRect.isNull() && !currentRect.contains(pos))
        {
            hideLater();
        }
        break;
    }

    case QEvent::ToolTip:
    {
        // eat appropriate tooltip events
        QWidget* widget = static_cast<QWidget*>(object);
        if (tooltips.contains(widget))
        {
            QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
            const QRect area = tooltips.value(widget).second;
            if (area.isNull() || area.contains(helpEvent->pos()))
            {
                show(helpEvent->globalPos(), tooltips.value(widget).first, widget, area);
                return true;
            }
        }
    }

    default:
        break;
    }
    return false;
}

void QxtToolTipPrivate::hideLater()
{
    currentRect = QRect();
    if (isVisible())
        QTimer::singleShot(0, this, SLOT(hide()));
}

QPoint QxtToolTipPrivate::calculatePos(int scr, const QPoint& eventPos) const
{
#ifdef Q_WS_MAC
    QRect screen = QApplication::desktop()->availableGeometry(scr);
#else
    QRect screen = QApplication::desktop()->screenGeometry(scr);
#endif

    QPoint p = eventPos;
    p += QPoint(2,
#ifdef Q_WS_WIN
                24
#else
                16
#endif
               );
    QSize s = sizeHint();
    if (p.x() + s.width() > screen.x() + screen.width())
        p.rx() -= 4 + s.width();
    if (p.y() + s.height() > screen.y() + screen.height())
        p.ry() -= 24 + s.height();
    if (p.y() < screen.y())
        p.setY(screen.y());
    if (p.x() + s.width() > screen.x() + screen.width())
        p.setX(screen.x() + screen.width() - s.width());
    if (p.x() < screen.x())
        p.setX(screen.x());
    if (p.y() + s.height() > screen.y() + screen.height())
        p.setY(screen.y() + screen.height() - s.height());
    return p;
}

/*!
    \class QxtToolTip QxtToolTip
    \ingroup QxtGui
    \brief Show any arbitrary widget as a tooltip.

    QxtToolTip provides means for showing any arbitrary widget as a tooltip.

    \note The rich text support of QToolTip already makes it possible to
    show heavily customized tooltips with lists, tables, embedded images
    and such. However, for example dynamically created images like
    thumbnails cause problems. Basically the only way is to dump the
    thumbnail to a temporary file to be able to embed it into HTML. This
    is where QxtToolTip steps in. A generated thumbnail may simply be set
    on a QLabel which is then shown as a tooltip. Yet another use case
    is a tooltip with dynamically changing content.

    \image html qxttooltip.png "QxtToolTip in action."

    \warning Added tooltip widgets remain in the memory for the lifetime
    of the application or until they are removed/deleted. Do NOT flood your
    application up with lots of complex tooltip widgets or it will end up
    being a resource hog. QToolTip is sufficient for most of the cases!
 */

/*!
    \reimp
 */
QxtToolTip::QxtToolTip()
{
}

/*!
    Shows the \a tooltip at \a pos for \a parent at \a rect.

    \sa hide()
*/
void QxtToolTip::show(const QPoint& pos, QWidget* tooltip, QWidget* parent, const QRect& rect)
{
    QxtToolTipPrivate::instance()->show(pos, tooltip, parent, rect);
}

/*!
    Hides the tooltip.

    \sa show()
*/
void QxtToolTip::hide()
{
    QxtToolTipPrivate::instance()->hide();
}

/*!
    Returns the tooltip for \a parent.

    \sa setToolTip()
*/
QWidget* QxtToolTip::toolTip(QWidget* parent)
{
    Q_ASSERT(parent);
    QWidget* tooltip = 0;
    if (!QxtToolTipPrivate::instance()->tooltips.contains(parent))
        qWarning("QxtToolTip::toolTip: Unknown parent");
    else
        tooltip = QxtToolTipPrivate::instance()->tooltips.value(parent).first;
    return tooltip;
}

/*!
    Sets the \a tooltip to be shown for \a parent.
    An optional \a rect may also be passed.

    \sa toolTip()
*/
void QxtToolTip::setToolTip(QWidget* parent, QWidget* tooltip, const QRect& rect)
{
    Q_ASSERT(parent);
    if (tooltip)
    {
        // set
        tooltip->hide();
        QxtToolTipPrivate::instance()->tooltips[parent] = qMakePair(QPointer<QWidget>(tooltip), rect);
    }
    else
    {
        // remove
        if (!QxtToolTipPrivate::instance()->tooltips.contains(parent))
            qWarning("QxtToolTip::setToolTip: Unknown parent");
        else
            QxtToolTipPrivate::instance()->tooltips.remove(parent);
    }
}

/*!
    Returns the rect on which tooltip is shown for \a parent.

    \sa setToolTipRect()
*/
QRect QxtToolTip::toolTipRect(QWidget* parent)
{
    Q_ASSERT(parent);
    QRect rect;
    if (!QxtToolTipPrivate::instance()->tooltips.contains(parent))
        qWarning("QxtToolTip::toolTipRect: Unknown parent");
    else
        rect = QxtToolTipPrivate::instance()->tooltips.value(parent).second;
    return rect;
}

/*!
    Sets the \a rect on which tooltip is shown for \a parent.

    \sa toolTipRect()
*/
void QxtToolTip::setToolTipRect(QWidget* parent, const QRect& rect)
{
    Q_ASSERT(parent);
    if (!QxtToolTipPrivate::instance()->tooltips.contains(parent))
        qWarning("QxtToolTip::setToolTipRect: Unknown parent");
    else
        QxtToolTipPrivate::instance()->tooltips[parent].second = rect;
}

/*!
    Returns the margin of the tooltip.

    \sa setMargin()
*/
int QxtToolTip::margin()
{
    return QxtToolTipPrivate::instance()->layout()->margin();
}

/*!
    Sets the margin of the tooltip.

    The default value is \b QStyle::PM_ToolTipLabelFrameWidth.

    \sa margin()
*/
void QxtToolTip::setMargin(int margin)
{
    QxtToolTipPrivate::instance()->layout()->setMargin(margin);
}

/*!
    Returns the opacity level of the tooltip.

    \sa QWidget::windowOpacity()
*/
qreal QxtToolTip::opacity()
{
    return QxtToolTipPrivate::instance()->windowOpacity();
}

/*!
    Sets the opacity level of the tooltip.

    The default value is \b QStyle::SH_ToolTipLabel_Opacity.

    \sa QWidget::setWindowOpacity()
*/
void QxtToolTip::setOpacity(qreal level)
{
    QxtToolTipPrivate::instance()->setWindowOpacity(level);
}
