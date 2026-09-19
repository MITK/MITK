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
#include <qwt_scale_widget.h>
#include <qwt_text.h>

#include <QBrush>
#include <QPalette>
#include <QPen>
#include <QString>

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

  QwtPlotZoomer* SetupNavigation(QwtPlot* plot)
  {
    auto* zoomer = new QwtPlotZoomer(plot->canvas());
    zoomer->setTrackerMode(QwtPicker::AlwaysOff);
    zoomer->setRubberBandPen(QPen(HIGHLIGHT_COLOR));
    // Free the middle button (default zoom-stack navigation) for panning: move
    // stepwise zoom-out to the right button and zoom-to-base to Ctrl+right.
    zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

    auto* panner = new QwtPlotPanner(plot->canvas());
    panner->setMouseButton(Qt::MiddleButton);

    auto* magnifier = new QwtPlotMagnifier(plot->canvas());
    magnifier->setMouseButton(Qt::NoButton);

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
