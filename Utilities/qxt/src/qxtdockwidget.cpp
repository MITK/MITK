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
#include "qxtdockwidget.h"
#include <QStyle>

class QxtDockWidgetPrivate : public QxtPrivate<QxtDockWidget>
{
public:
    QXT_DECLARE_PUBLIC(QxtDockWidget);

    QSize contentsSize() const;
    QSize prev;
};

QSize QxtDockWidgetPrivate::contentsSize() const
{
    QSize contents = qxt_p().size();
    int fw = qxt_p().style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth);
    QSize frame(2 * fw, fw);
#ifdef Q_WS_WIN
    frame -= QSize(0, 3);
#endif
    contents -= frame;
    return contents;
}

/*!
    \class QxtDockWidget QxtDockWidget
    \ingroup QxtGui
    \brief An extended QDockWidget that remembers its size.

    QxtDockWidget fills in the gap in QDockWidget and makes the dock
    widget to remember its size while toggling visibility off and on.

    \note 146752 - QDockWidget should remember its size when hidden/shown.<br>
    http://www.trolltech.com/developer/task-tracker/index_html?method=entry&amp;id=146752
 */

/*!
    Constructs a new QxtDockWidget with \a title, \a parent and \a flags.
 */
QxtDockWidget::QxtDockWidget(const QString& title, QWidget* parent, Qt::WindowFlags flags)
        : QDockWidget(title, parent, flags)
{
    QXT_INIT_PRIVATE(QxtDockWidget);
}

/*!
    Constructs a new QxtDockWidget with \a title, \a parent and \a flags.
 */
QxtDockWidget::QxtDockWidget(QWidget* parent, Qt::WindowFlags flags)
        : QDockWidget(parent, flags)
{
    QXT_INIT_PRIVATE(QxtDockWidget);
}

/*!
    Destructs the dock widget.
 */
QxtDockWidget::~QxtDockWidget()
{}

/*!
    \reimp
 */
QSize QxtDockWidget::sizeHint() const
{
    QSize size;
    if (qxt_d().prev.isValid() && !isFloating())
        size = qxt_d().prev;
    else
        size = QDockWidget::sizeHint();
    return size;
}

/*!
    \reimp
 */
void QxtDockWidget::setVisible(bool visible)
{
    if (!visible && !isFloating())
        qxt_d().prev = qxt_d().contentsSize();
    QDockWidget::setVisible(visible);
}
