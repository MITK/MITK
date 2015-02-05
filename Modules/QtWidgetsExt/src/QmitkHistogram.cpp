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

#include <qpainter.h>
#include <qwt_plot.h>
#include <qwt_painter.h>
#include <qwt_scale_map.h>

#include "QmitkHistogram.h"

class QmitkHistogram::HistogramData
{
public:
  QwtIntervalSeriesData data;
  QColor color;
  double reference;
};

QmitkHistogram::QmitkHistogram(const QwtText &title):
QwtPlotItem(title)
{
  init();
}

QmitkHistogram::QmitkHistogram(const QString &title):
QwtPlotItem(QwtText(title))
{
  init();
}

QmitkHistogram::~QmitkHistogram()
{
  delete m_Data;
}

void QmitkHistogram::init()
{
  m_Data = new HistogramData();
  m_Data->reference = 0.0;

  setItemAttribute(QwtPlotItem::AutoScale, true);
  setItemAttribute(QwtPlotItem::Legend, true);

  setZ(20.0);
}

void QmitkHistogram::setBaseline(double reference)
{
  if ( m_Data->reference != reference )
  {
    m_Data->reference = reference;
    itemChanged();
  }
}

double QmitkHistogram::baseline() const
{
  return m_Data->reference;
}

void QmitkHistogram::setData(const QwtIntervalSeriesData &data)
{
  m_Data->data.setSamples(data.samples());
  itemChanged();
}

const QwtIntervalSeriesData &QmitkHistogram::data() const
{
  return m_Data->data;
}

void QmitkHistogram::setColor(const QColor &color)
{
  if ( m_Data->color != color )
  {
    m_Data->color = color;
    itemChanged();
  }
}

QColor QmitkHistogram::color() const
{
  return m_Data->color;
}

QRectF QmitkHistogram::boundingRect() const
{
  QRectF rect = m_Data->data.boundingRect();
  if ( !rect.isValid() )
    return rect;

  if ( rect.bottom() < m_Data->reference )
    rect.setBottom( m_Data->reference );
  else if ( rect.top() > m_Data->reference )
    rect.setTop( m_Data->reference );

  return rect;
}

void QmitkHistogram::draw(QPainter *painter, const QwtScaleMap &xMap,
                          const QwtScaleMap &yMap, const QRectF &) const
{
  const QwtIntervalSeriesData &iData = m_Data->data;

  painter->setPen(QPen(m_Data->color));

  const int y0 = yMap.transform(baseline());

  for ( int i = 0; i < (int)iData.size(); i++ )
  {
    const int y2 = yMap.transform(iData.sample(i).value);
    if ( y2 == y0 )
      continue;

    int x1 = xMap.transform(iData.sample(i).interval.minValue());
    int x2 = xMap.transform(iData.sample(i).interval.maxValue());
    if ( x1 > x2 )
      qSwap(x1, x2);

    if ( i < (int)iData.size() - 2 )
    {
      const int xx1 = xMap.transform(iData.sample(i+1).interval.minValue());
      const int xx2 = xMap.transform(iData.sample(i+1).interval.maxValue());

      if ( x2 == qMin(xx1, xx2) )
      {
        const int yy2 = yMap.transform(iData.sample(i+1).value);
        if ( yy2 != y0 && ( (yy2 < y0 && y2 < y0) ||
          (yy2 > y0 && y2 > y0) ) )
        {
          // One pixel distance between neighbored bars
          x2--;
        }
      }
    }
    drawBar(painter, Qt::Vertical,
      QRect(x1, y0, x2 - x1, y2 - y0) );
  }
}

void QmitkHistogram::drawBar(QPainter *painter,
                             Qt::Orientation, const QRect& rect) const
{
  painter->save();

  const QColor color(painter->pen().color());
  const QRect r = rect.normalized();
  const int factor = 125;
  const QColor light(color.light(factor));
  const QColor dark(color.dark(factor));

  painter->setBrush(color);
  painter->setPen(Qt::NoPen);
  QwtPainter::drawRect(painter, r.x() + 1, r.y() + 1,
    r.width() - 2, r.height() - 2);
  painter->setBrush(Qt::NoBrush);

  painter->setPen(QPen(light, 2));

  QwtPainter::drawLine(painter,
    r.left() + 1, r.top() + 2, r.right() + 1, r.top() + 2);

  painter->setPen(QPen(dark, 2));

  QwtPainter::drawLine(painter,
    r.left() + 1, r.bottom(), r.right() + 1, r.bottom());

  painter->setPen(QPen(light, 1));

  QwtPainter::drawLine(painter,
    r.left(), r.top() + 1, r.left(), r.bottom());
  QwtPainter::drawLine(painter,
    r.left() + 1, r.top() + 2, r.left() + 1, r.bottom() - 1);

  painter->setPen(QPen(dark, 1));

  QwtPainter::drawLine(painter,
    r.right() + 1, r.top() + 1, r.right() + 1, r.bottom());
  QwtPainter::drawLine(painter,
    r.right(), r.top() + 2, r.right(), r.bottom() - 1);

  painter->restore();
}

