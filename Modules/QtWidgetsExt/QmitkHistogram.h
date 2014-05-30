/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKHISTOGRAM_H
#define QMITKHISTOGRAM_H

#include <qglobal.h>
#include <qcolor.h>

#include <qwt_plot_item.h>
#include <qwt_series_data.h>

/**
\brief Used to create a histogram that can be shown in a Qwt Plot.
See QmitkHistogramWidget for an example of its usage.
*/

class QmitkHistogram: public QwtPlotItem
{
public:
    explicit QmitkHistogram(const QString &title = QString::null);
    explicit QmitkHistogram(const QwtText &title);
    virtual ~QmitkHistogram();

    void setData(const QwtIntervalSeriesData &data);
    const QwtIntervalSeriesData &data() const;

    void setColor(const QColor &);
    QColor color() const;

    virtual QRectF boundingRect() const;

    virtual void draw(QPainter *, const QwtScaleMap &xMap,
        const QwtScaleMap &yMap, const QRectF &) const;

    void setBaseline(double reference);
    double baseline() const;

protected:
    virtual void drawBar(QPainter *,
        Qt::Orientation o, const QRect &) const;

private:
    void init();

    class HistogramData;
    HistogramData *m_Data;
};

#endif

