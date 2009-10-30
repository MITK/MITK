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
#include "qxtheaderview.h"

class QxtHeaderViewPrivate : public QxtPrivate<QxtHeaderView>
{
public:
    QXT_DECLARE_PUBLIC(QxtHeaderView);

    QxtHeaderViewPrivate();

    bool proportional;
    QMap<int, int> factors;
};

QxtHeaderViewPrivate::QxtHeaderViewPrivate() : proportional(false)
{
}

/*!
    \class QxtHeaderView QxtHeaderView
    \ingroup QxtGui
    \brief An extended QHeaderView with optionally proportional section sizes.
 */

/*!
    Constructs a new QxtHeaderView with \a orientation and \a parent.
 */

QxtHeaderView::QxtHeaderView(Qt::Orientation orientation, QWidget* parent)
        : QHeaderView(orientation, parent)
{
    QXT_INIT_PRIVATE(QxtHeaderView);
}

/*!
    \property QxtHeaderView::hasProportionalSectionSizes
    \brief This property holds whether section sizes are proportional.

    The default value is \b true.

    \note Enabling proportional sections sizes sets resize mode
    \b QHeaderView::Fixed, which means that the user cannot resize
    sections.
 */
bool QxtHeaderView::hasProportionalSectionSizes() const
{
    return qxt_d().proportional;
}

void QxtHeaderView::setProportionalSectionSizes(bool enabled)
{
    if (qxt_d().proportional != enabled)
    {
        qxt_d().proportional = enabled;
        if (enabled)
            setResizeMode(QHeaderView::Fixed);
    }
}

/*!
    Returns the stretch factor of the section at \a logicalIndex.

    \sa setSectionStretchFactor()
 */
int QxtHeaderView::sectionStretchFactor(int logicalIndex) const
{
    return qxt_d().factors.value(logicalIndex);
}

/*!
    Sets the stretch \a factor of the section at \a logicalIndex.

    \sa sectionStretchFactor()
 */
void QxtHeaderView::setSectionStretchFactor(int logicalIndex, int factor)
{
    qxt_d().factors.insert(logicalIndex, factor);
}

/*!
    \reimp
 */
void QxtHeaderView::resizeEvent(QResizeEvent* event)
{
    QHeaderView::resizeEvent(event);
    if (qxt_d().proportional)
    {
        int total = 0;
        for (int i = 0; i < count(); ++i)
            total += qxt_d().factors.value(i, 1);

        int totalSize = 0;
        for (int i = 0; i < count() - 1; ++i)
        {
            qreal factor = qxt_d().factors.value(i, 1) / static_cast<qreal>(total);
            int sectionSize = factor * (orientation() == Qt::Horizontal ? width() : height());
            sectionSize = qMax(minimumSectionSize(), sectionSize);
            resizeSection(i, sectionSize);
            totalSize += sectionSize;
        }
        // avoid rounding errors, give rest to the last section
        resizeSection(count() - 1, width() - totalSize);
    }
}
