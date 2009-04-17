#include <qpainter.h>
#include <qwt_plot.h>
#include <qwt_painter.h>
#include <qwt_scale_map.h>

#include "QmitkHistogram.h"

class QmitkHistogram::HistogramData
{
public:
  QwtIntervalData data;
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

void QmitkHistogram::setData(const QwtIntervalData &data)
{
  m_Data->data = data;
  itemChanged();
}

const QwtIntervalData &QmitkHistogram::data() const
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

QwtDoubleRect QmitkHistogram::boundingRect() const
{
  QwtDoubleRect rect = m_Data->data.boundingRect();
  if ( !rect.isValid() ) 
    return rect;

  if ( rect.bottom() < m_Data->reference ) 
    rect.setBottom( m_Data->reference );
  else if ( rect.top() > m_Data->reference ) 
    rect.setTop( m_Data->reference );

  return rect;
}

void QmitkHistogram::draw(QPainter *painter, const QwtScaleMap &xMap, 
                          const QwtScaleMap &yMap, const QRect &) const
{
  const QwtIntervalData &iData = m_Data->data;

  painter->setPen(QPen(m_Data->color));

  const int x0 = xMap.transform(baseline());
  const int y0 = yMap.transform(baseline());

  for ( int i = 0; i < (int)iData.size(); i++ )
  {
    const int y2 = yMap.transform(iData.value(i));
    if ( y2 == y0 )
      continue;

    int x1 = xMap.transform(iData.interval(i).minValue());
    int x2 = xMap.transform(iData.interval(i).maxValue());
    if ( x1 > x2 )
      qSwap(x1, x2);

    if ( i < (int)iData.size() - 2 )
    {
      const int xx1 = xMap.transform(iData.interval(i+1).minValue());
      const int xx2 = xMap.transform(iData.interval(i+1).maxValue());

      if ( x2 == qwtMin(xx1, xx2) )
      {
        const int yy2 = yMap.transform(iData.value(i+1));
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
#if QT_VERSION >= 0x040000
  const QRect r = rect.normalized();
#else
  const QRect r = rect.normalize();
#endif

  const int factor = 125;
  const QColor light(color.light(factor));
  const QColor dark(color.dark(factor));

  painter->setBrush(color);
  painter->setPen(Qt::NoPen);
  QwtPainter::drawRect(painter, r.x() + 1, r.y() + 1,
    r.width() - 2, r.height() - 2);
  painter->setBrush(Qt::NoBrush);

  painter->setPen(QPen(light, 2));
#if QT_VERSION >= 0x040000
  QwtPainter::drawLine(painter,
    r.left() + 1, r.top() + 2, r.right() + 1, r.top() + 2);
#else
  QwtPainter::drawLine(painter,
    r.left(), r.top() + 2, r.right() + 1, r.top() + 2);
#endif

  painter->setPen(QPen(dark, 2));
#if QT_VERSION >= 0x040000
  QwtPainter::drawLine(painter, 
    r.left() + 1, r.bottom(), r.right() + 1, r.bottom());
#else
  QwtPainter::drawLine(painter, 
    r.left(), r.bottom(), r.right() + 1, r.bottom());
#endif

  painter->setPen(QPen(light, 1));

#if QT_VERSION >= 0x040000
  QwtPainter::drawLine(painter, 
    r.left(), r.top() + 1, r.left(), r.bottom());
  QwtPainter::drawLine(painter,
    r.left() + 1, r.top() + 2, r.left() + 1, r.bottom() - 1);
#else
  QwtPainter::drawLine(painter, 
    r.left(), r.top() + 1, r.left(), r.bottom() + 1);
  QwtPainter::drawLine(painter,
    r.left() + 1, r.top() + 2, r.left() + 1, r.bottom());
#endif

  painter->setPen(QPen(dark, 1));

#if QT_VERSION >= 0x040000
  QwtPainter::drawLine(painter, 
    r.right() + 1, r.top() + 1, r.right() + 1, r.bottom());
  QwtPainter::drawLine(painter, 
    r.right(), r.top() + 2, r.right(), r.bottom() - 1);
#else
  QwtPainter::drawLine(painter, 
    r.right() + 1, r.top() + 1, r.right() + 1, r.bottom() + 1);
  QwtPainter::drawLine(painter, 
    r.right(), r.top() + 2, r.right(), r.bottom());
#endif

  painter->restore();
}
