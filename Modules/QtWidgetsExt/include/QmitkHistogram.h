/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkHistogram_h
#define QmitkHistogram_h

#include <qcolor.h>
#include <qglobal.h>

#include <qwt_plot_item.h>
#include <qwt_series_data.h>
#include <qwt_text.h>

/**
\brief Used to create a histogram that can be shown in a Qwt Plot.
See QmitkHistogramWidget for an example of its usage.
*/

class QmitkHistogram : public QwtPlotItem
{
public:
  explicit QmitkHistogram(const QString &title = QString::null);
  explicit QmitkHistogram(const QwtText &title);
  ~QmitkHistogram() override;

  void setData(const QwtIntervalSeriesData &data);
  const QwtIntervalSeriesData &data() const;

  void setColor(const QColor &);
  QColor color() const;

  QRectF boundingRect() const override;

  void draw(QPainter *, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &) const override;

  void setBaseline(double reference);
  double baseline() const;

protected:
  virtual void drawBar(QPainter *, Qt::Orientation o, const QRect &) const;

private:
  void init();

  class HistogramData;
  HistogramData *m_Data;
};

#endif
