/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKHISTOGRAM_H
#define QMITKHISTOGRAM_H

#include <qglobal.h>
#include <qcolor.h>

#include <qwt_plot_item.h> 

class QwtIntervalData;

/** 
\brief Used to create a histogram that can be shown in a Qwt Plot.
See QmitkHistogramWidget for an example of its usage.
*/

class QmitkHistogram: public QwtPlotItem
{

// Q_OBJECT

public:
    explicit QmitkHistogram(const QString &title = QString::null);
    explicit QmitkHistogram(const QwtText &title);
    virtual ~QmitkHistogram();

    void setData(const QwtIntervalData &data);
    const QwtIntervalData &data() const;

    void setColor(const QColor &);
    QColor color() const;

    virtual QwtDoubleRect boundingRect() const;

    virtual void draw(QPainter *, const QwtScaleMap &xMap, 
        const QwtScaleMap &yMap, const QRect &) const;

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
