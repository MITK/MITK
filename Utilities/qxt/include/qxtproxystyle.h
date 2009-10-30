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
#ifndef QXTPROXYSTYLE_H
#define QXTPROXYSTYLE_H

#include <QStyle>
#include "qxtglobal.h"

class QXT_GUI_EXPORT QxtProxyStyle : public QStyle
{
public:
    explicit QxtProxyStyle(const QString& baseStyle);
    virtual ~QxtProxyStyle();

    virtual void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget = 0) const;
    virtual void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = 0)  const;
    virtual void drawItemPixmap(QPainter* painter, const QRect& rect, int alignment, const QPixmap& pixmap) const;
    virtual void drawItemText(QPainter* painter, const QRect& rect, int alignment, const QPalette& pal, bool enabled, const QString& text, QPalette::ColorRole textRole = QPalette::NoRole) const;
    virtual void drawPrimitive(PrimitiveElement elem, const QStyleOption* option, QPainter* painter, const QWidget* widget = 0) const;
    virtual QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap& pixmap, const QStyleOption* option) const;
    virtual SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex* option, const QPoint& pos, const QWidget* widget = 0) const;
    virtual QRect itemPixmapRect(const QRect& rect, int alignment, const QPixmap& pixmap) const;
    virtual QRect itemTextRect(const QFontMetrics& metrics, const QRect& rect, int alignment, bool enabled, const QString& text) const;
    virtual int pixelMetric(PixelMetric metric, const QStyleOption* option = 0, const QWidget* widget = 0) const;
    virtual void polish(QWidget* widget);
    virtual void polish(QApplication* app);
    virtual void polish(QPalette& pal);
    virtual QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& contentsSize, const QWidget* widget = 0) const;
    virtual QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption* option = 0, const QWidget* widget = 0) const;
    virtual QPalette standardPalette() const;
    virtual QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption* option = 0, const QWidget* widget = 0) const;
    virtual int styleHint(StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const;
    virtual QRect subControlRect(ComplexControl control, const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget = 0) const;
    virtual QRect subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget = 0) const;
    virtual void unpolish(QWidget* widget);
    virtual void unpolish(QApplication* app);

private:
    QStyle* style;
};

#endif // QXTPROXYSTYLE_H
