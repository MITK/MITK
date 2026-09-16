/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkImageStatisticsPlotHelper_h
#define QmitkImageStatisticsPlotHelper_h

#include <QmitkPlotStyle.h>

#include <QColor>

class QString;
class QwtPlot;
class QwtPlotZoomer;
class QwtText;

/**
 * \brief Shared Qwt plot decoration for the image-statistics widgets.
 *
 * The histogram and intensity-profile widgets configure their plots
 * identically; these helpers keep that styling in one place.
 */
namespace QmitkImageStatisticsPlot
{
  /** Accent colour for highlighted bins and the zoom rubber band. */
  extern const QColor HIGHLIGHT_COLOR;

  /** Dark-on-light tracker tooltip shared by the plot pickers. */
  QwtText MakeTooltip(const QString& text);

  /** Adds interactive navigation: left-drag box zoom (right-click to zoom out),
      middle-drag pan, and mouse-wheel zoom. Returns the zoomer so its base can
      be re-synced to the data range. */
  QwtPlotZoomer* SetupNavigation(QwtPlot* plot);

  /** Applies the light/dark canvas background and axis palette to the plot. */
  void ApplyTheme(QwtPlot* plot, QmitkPlotStyle style);
}

#endif
