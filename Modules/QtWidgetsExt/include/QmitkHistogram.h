/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkHistogram_h
#define QmitkHistogram_h

#include <QColor>

#include <qwt_plot_item.h>
#include <qwt_series_data.h>
#include <qwt_text.h>

/**
 * \brief A QwtPlotItem that renders a histogram as vertical bars in a QwtPlot.
 *
 * Each bar corresponds to an interval sample. Bars are drawn with a 3D shading
 * effect using lighter/darker pen colors. The baseline reference value can be
 * adjusted.
 *
 * \sa QmitkPlotWidget, QmitkPlotDialog
 */
class QmitkHistogram : public QwtPlotItem
{
public:
  /**
   * \brief Construct a histogram with a plain text title.
   * \param[in] title The title string for the histogram item.
   */
  explicit QmitkHistogram(const QString &title = QString());

  /**
   * \brief Construct a histogram with a formatted QwtText title.
   * \param[in] title The QwtText title.
   */
  explicit QmitkHistogram(const QwtText &title);

  /** \brief Destructor. */
  ~QmitkHistogram() override;

  /**
   * \brief Set the interval series data for the histogram.
   * \param[in] data The interval series data containing bin intervals and values.
   */
  void setData(const QwtIntervalSeriesData &data);

  /**
   * \brief Get the current interval series data.
   * \return Const reference to the data.
   */
  const QwtIntervalSeriesData &data() const;

  /**
   * \brief Set the fill color for the histogram bars.
   * \param[in] color The bar color.
   */
  void setColor(const QColor &color);

  /**
   * \brief Get the current bar color.
   * \return The color.
   */
  QColor color() const;

  /**
   * \brief Calculate the bounding rectangle of the histogram data.
   * \return The bounding rectangle including the baseline.
   */
  QRectF boundingRect() const override;

  /**
   * \brief Draw the histogram bars onto the given painter.
   * \param[in] painter The QPainter to draw with.
   * \param[in] xMap Scale map for the x axis.
   * \param[in] yMap Scale map for the y axis.
   * \param[in] canvasRect The canvas rectangle (unused).
   */
  void draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect) const override;

  /**
   * \brief Set the baseline reference value for the histogram.
   * \param[in] reference The y value of the baseline.
   */
  void setBaseline(double reference);

  /**
   * \brief Get the current baseline reference value.
   * \return The baseline value.
   */
  double baseline() const;

protected:
  /**
   * \brief Draw a single histogram bar with 3D shading.
   * \param[in] painter The QPainter.
   * \param[in] o The bar orientation.
   * \param[in] rect The rectangle defining the bar area.
   */
  virtual void drawBar(QPainter *painter, Qt::Orientation o, const QRect &rect) const;

private:
  void init();

  class HistogramData;
  HistogramData *m_Data;
};

#endif
