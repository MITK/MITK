/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkImageStatisticsPlotHelper.h"

#include <qwt_plot.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_div.h>
#include <qwt_scale_map.h>
#include <qwt_scale_widget.h>
#include <qwt_text.h>

#include <QBrush>
#include <QPalette>
#include <QPen>
#include <QRectF>
#include <QString>
#include <QWheelEvent>

#include <cmath>
#include <optional>

namespace
{
  /** Shifts the y axis up if it reaches below the minimum, keeping its extent. */
  void ClampYAxis(QwtPlot* plot, double minimumY)
  {
    const QwtScaleDiv& scaleDiv = plot->axisScaleDiv(QwtPlot::yLeft);
    const double lower = scaleDiv.lowerBound();

    if (lower >= minimumY)
      return;

    plot->setAxisScale(QwtPlot::yLeft, minimumY, scaleDiv.upperBound() + (minimumY - lower));
    plot->replot();
  }

  class BoundedZoomer : public QwtPlotZoomer
  {
  public:
    BoundedZoomer(QWidget* canvas, double minimumY)
      : QwtPlotZoomer(canvas),
        m_MinimumY(minimumY)
    {
    }

    void zoom(const QRectF& rect) override
    {
      QRectF bounded = rect.normalized();

      // Entirely below the minimum: nothing to zoom to.
      if (bounded.bottom() <= m_MinimumY)
        return;

      if (bounded.top() < m_MinimumY)
        bounded.setTop(m_MinimumY);

      QwtPlotZoomer::zoom(bounded);
    }

    void zoom(int offset) override
    {
      QwtPlotZoomer::zoom(offset);

      // Panning and wheel zoom move the axes without touching the zoom stack, so
      // stepping through the stack has to reach its rectangle even if the index
      // did not change (e.g. "zoom out" while already at the base).
      if (this->scaleRect() != this->zoomRect())
      {
        this->rescale();
        Q_EMIT zoomed(this->zoomRect());
      }
    }

  private:
    double m_MinimumY;
  };

  class BoundedPanner : public QwtPlotPanner
  {
  public:
    BoundedPanner(QWidget* canvas, double minimumY)
      : QwtPlotPanner(canvas),
        m_MinimumY(minimumY)
    {
    }

  protected:
    void moveCanvas(int dx, int dy) override
    {
      QwtPlotPanner::moveCanvas(dx, dy);
      ClampYAxis(this->plot(), m_MinimumY);
    }

  private:
    double m_MinimumY;
  };

  /** Wheel zoom along the x axis only, centered on the cursor, so the y range is
  never changed by the wheel and the hovered value stays under the cursor. */
  class HorizontalMagnifier : public QwtPlotMagnifier
  {
  public:
    explicit HorizontalMagnifier(QWidget* canvas)
      : QwtPlotMagnifier(canvas)
    {
    }

  protected:
    void widgetWheelEvent(QWheelEvent* event) override
    {
      m_WheelPosition = event->position();
      QwtPlotMagnifier::widgetWheelEvent(event);
      m_WheelPosition.reset();
    }

    void rescale(double factor) override
    {
      QwtPlot* plot = this->plot();
      factor = std::abs(factor);

      if (nullptr == plot || factor == 1.0 || factor == 0.0)
        return;

      const QwtScaleMap map = plot->canvasMap(QwtPlot::xBottom);
      const double lower = map.s1();
      const double upper = map.s2();
      // Keyboard zoom has no cursor position and scales around the center.
      const double center = m_WheelPosition ? map.invTransform(m_WheelPosition->x()) : 0.5 * (lower + upper);

      plot->setAxisScale(QwtPlot::xBottom, center - (center - lower) * factor, center + (upper - center) * factor);
      plot->replot();
    }

  private:
    std::optional<QPointF> m_WheelPosition;
  };
}

namespace QmitkImageStatisticsPlot
{
  const QColor HIGHLIGHT_COLOR(0x9e, 0xce, 0xf5);

  QwtText MakeTooltip(const QString& text)
  {
    QwtText tooltip(text);
    tooltip.setColor(Qt::white);
    tooltip.setBackgroundBrush(QBrush(QColor(0, 0, 0, 180)));
    tooltip.setRenderFlags(Qt::AlignLeft | Qt::AlignVCenter);
    return tooltip;
  }

  QwtPlotZoomer* SetupNavigation(QwtPlot* plot, double minimumY)
  {
    auto* zoomer = new BoundedZoomer(plot->canvas(), minimumY);
    zoomer->setTrackerMode(QwtPicker::AlwaysOff);
    zoomer->setRubberBandPen(QPen(HIGHLIGHT_COLOR));
    // Free the middle button (default zoom-stack navigation) for panning: move
    // stepwise zoom-out to the right button and zoom-to-base to Ctrl+right.
    zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

    auto* panner = new BoundedPanner(plot->canvas(), minimumY);
    panner->setMouseButton(Qt::MiddleButton);

    auto* magnifier = new HorizontalMagnifier(plot->canvas());
    magnifier->setMouseButton(Qt::NoButton);
    // Qwt zooms out when scrolling up; inverting the factor makes scrolling up zoom in.
    magnifier->setWheelFactor(1.0 / magnifier->wheelFactor());

    return zoomer;
  }

  void ApplyTheme(QwtPlot* plot, QmitkPlotStyle style)
  {
    const bool dark = style == QmitkPlotStyle::Dark;
    const QColor background = dark ? QColor(0x2d, 0x2d, 0x30) : QColor(Qt::white);
    const QColor foreground = dark ? QColor(0xf1, 0xf1, 0xf1) : QColor(Qt::black);

    plot->setCanvasBackground(background);

    QPalette palette = plot->palette();
    palette.setColor(QPalette::WindowText, foreground);
    palette.setColor(QPalette::Text, foreground);
    plot->setPalette(palette);
    plot->axisWidget(QwtPlot::xBottom)->setPalette(palette);
    plot->axisWidget(QwtPlot::yLeft)->setPalette(palette);

    plot->replot();
  }
}
