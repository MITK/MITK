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
#include "qxtproxystyle.h"
#include <QStyleFactory>

/*!
    \class QxtProxyStyle QxtProxyStyle
    \ingroup QxtGui
    \brief A proxy style.

    A technique called "proxy style" is a common way for creating
    cross-platform custom styles. Developers often want to do slight
    adjustments in some specific values returned by QStyle. A proxy
    style is the solution to avoid subclassing any platform specific
    style (eg. QPlastiqueStyle, QWindowsXPStyle, or QMacStyle) and
    to retain the native look on all supported platforms.

    The subject has been discussed in Qt Quarterly 9:
    http://doc.trolltech.com/qq/qq09-q-and-a.html#style (just notice
    that there are a few noteworthy spelling mistakes in the article).

    Proxy styles are becoming obsolete thanks to style sheets
    introduced in Qt 4.2. However, style sheets still is a new
    concept and only a portion of features are supported yet. Both -
    style sheets and proxy styles - have their pros and cons.

    \section usage Usage

    Implement the custom behaviour in a subclass of QxtProxyStyle:
    \code
    class MyCustomStyle : public QxtProxyStyle
    {
       public:
          MyCustomStyle(const QString& baseStyle) : QxtProxyStyle(baseStyle)
          {
          }

          int pixelMetric(PixelMetric metric, const QStyleOption* option = 0, const QWidget* widget = 0) const
          {
             if (metric == QStyle::PM_ButtonMargin)
                return 6;
             return QxtProxyStyle::pixelMetric(metric, option, widget);
          }
    };
    \endcode

    Using the custom style for the whole application:
    \code
    QString defaultStyle = QApplication::style()->objectName();
    QApplication::setStyle(new MyCustomStyle(defaultStyle));
    \endcode

    Using the custom style for a single widget:
    \code
    QString defaultStyle = widget->style()->objectName();
    widget->setStyle(new MyCustomStyle(defaultStyle));
    \endcode
 */

/*!
    Constructs a new QxtProxyStyle for \a style.
    See QStyleFactory::keys() for supported styles.

    \sa QStyleFactory::keys()
 */
QxtProxyStyle::QxtProxyStyle(const QString& baseStyle)
        : QStyle(), style(QStyleFactory::create(baseStyle))
{
    setObjectName(QLatin1String("proxy"));
}

/*!
    Destructs the proxy style.
 */
QxtProxyStyle::~QxtProxyStyle()
{
    delete style;
}

/*!
    \reimp
 */
void QxtProxyStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const
{
    style->drawComplexControl(control, option, painter, widget);
}

/*!
    \reimp
 */
void QxtProxyStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget)  const
{
    style->drawControl(element, option, painter, widget);
}

/*!
    \reimp
 */
void QxtProxyStyle::drawItemPixmap(QPainter* painter, const QRect& rect, int alignment, const QPixmap& pixmap) const
{
    style->drawItemPixmap(painter, rect, alignment, pixmap);
}

/*!
    \reimp
 */
void QxtProxyStyle::drawItemText(QPainter* painter, const QRect& rect, int alignment, const QPalette& pal, bool enabled, const QString& text, QPalette::ColorRole textRole) const
{
    style->drawItemText(painter, rect, alignment, pal, enabled, text, textRole);
}

/*!
    \reimp
 */
void QxtProxyStyle::drawPrimitive(PrimitiveElement elem, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    style->drawPrimitive(elem, option, painter, widget);
}

/*!
    \reimp
 */
QPixmap QxtProxyStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap& pixmap, const QStyleOption* option) const
{
    return style->generatedIconPixmap(iconMode, pixmap, option);
}

/*!
    \reimp
 */
QStyle::SubControl QxtProxyStyle::hitTestComplexControl(ComplexControl control, const QStyleOptionComplex* option, const QPoint& pos, const QWidget* widget) const
{
    return style->hitTestComplexControl(control, option, pos, widget);
}

/*!
    \reimp
 */
QRect QxtProxyStyle::itemPixmapRect(const QRect& rect, int alignment, const QPixmap& pixmap) const
{
    return style->itemPixmapRect(rect, alignment, pixmap);
}

/*!
    \reimp
 */
QRect QxtProxyStyle::itemTextRect(const QFontMetrics& metrics, const QRect& rect, int alignment, bool enabled, const QString& text) const
{
    return style->itemTextRect(metrics, rect, alignment, enabled, text);
}

/*!
    \reimp
 */
int QxtProxyStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    return style->pixelMetric(metric, option, widget);
}

/*!
    \reimp
 */
void QxtProxyStyle::polish(QWidget* widget)
{
    style->polish(widget);
}

/*!
    \reimp
 */
void QxtProxyStyle::polish(QApplication* app)
{
    style->polish(app);
}

/*!
    \reimp
 */
void QxtProxyStyle::polish(QPalette& pal)
{
    style->polish(pal);
}

/*!
    \reimp
 */
QSize QxtProxyStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& contentsSize, const QWidget* widget) const
{
    return style->sizeFromContents(type, option, contentsSize, widget);
}

/*!
    \reimp
 */
QIcon QxtProxyStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption* option, const QWidget* widget) const
{
    return style->standardIcon(standardIcon, option, widget);
}

/*!
    \reimp
 */
QPalette QxtProxyStyle::standardPalette() const
{
    return style->standardPalette();
}

/*!
    \reimp
 */
QPixmap QxtProxyStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption* option, const QWidget* widget) const
{
    return style->standardPixmap(standardPixmap, option, widget);
}

/*!
    \reimp
 */
int QxtProxyStyle::styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
{
    return style->styleHint(hint, option, widget, returnData);
}

/*!
    \reimp
 */
QRect QxtProxyStyle::subControlRect(ComplexControl control, const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget) const
{
    return style->subControlRect(control, option, subControl, widget);
}

/*!
    \reimp
 */
QRect QxtProxyStyle::subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget) const
{
    return style->subElementRect(element, option, widget);
}

/*!
    \reimp
 */
void QxtProxyStyle::unpolish(QWidget* widget)
{
    style->unpolish(widget);
}

/*!
    \reimp
 */
void QxtProxyStyle::unpolish(QApplication* app)
{
    style->unpolish(app);
}
