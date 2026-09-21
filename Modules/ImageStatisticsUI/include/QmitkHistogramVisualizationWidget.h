/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkHistogramVisualizationWidget_h
#define QmitkHistogramVisualizationWidget_h

#include <MitkImageStatisticsUIExports.h>

#include <QmitkPlotStyle.h>

// itk
#include <itkHistogram.h>

#include <QColor>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QWidget>

#include <memory>
#include <vector>

class QEvent;
class QwtPlot;
class QwtPlotHistogram;
class QwtPlotZoomer;

namespace Ui
{
  class QmitkHistogramVisualizationControls;
}

/**
 * \brief Widget for displaying and interacting with histogram visualizations.
 *
 * Provides a Qwt-based histogram display with controls for adjusting the number
 * of bins, setting a custom min/max value zoom range, and copying histogram data
 * to the clipboard. Several histograms (e.g. one per label of a segmentation) can
 * be shown at once, each in its own color.
 *
 * \sa QmitkImageStatisticsWidget
 */

class MITKIMAGESTATISTICSUI_EXPORT QmitkHistogramVisualizationWidget : public QWidget
{
  Q_OBJECT

public:
  using HistogramType = itk::Statistics::Histogram<double>;

  /** \brief One histogram to show, with the name and color it is shown with. */
  struct HistogramSeries
  {
    /** Shown in the tooltip (if several series are shown) and used in the clipboard export. */
    QString name;
    /** An invalid color selects the default bar color. */
    QColor color;
    HistogramType::ConstPointer histogram;
  };

  /**
   * \brief Constructs the histogram visualization widget.
   * \param[in] parent Optional parent widget.
   */
  QmitkHistogramVisualizationWidget(QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkHistogramVisualizationWidget() override;

  /**
   * \brief Replaces the shown histograms and enables the GUI controls.
   *
   * A zoomed-in view survives the call as long as it overlaps the new value range.
   * Series without a histogram are skipped. An empty vector leaves an empty plot
   * with disabled controls.
   *
   * \param[in] series The histograms to show, in drawing order.
   */
  void SetHistograms(const std::vector<HistogramSeries>& series);

  /**
   * \brief Clears all histogram data including the zoom and disables all GUI elements.
   */
  void Reset();

  /**
   * \brief Sets the color theme for the plot.
   * \param[in] style The plot style to apply (dark or light).
   */
  void SetTheme(QmitkPlotStyle style);

  /**
   * \brief Resets all controls to their default settings.
   *
   * Enables the default number of bins checkbox and restores the default bin count.
   */
  void ResetDefault();

  /**
   * \brief Returns the current number of histogram bins configured in the spin box.
   * \return The number of bins.
   */
  int GetBins();

signals:
  /**
   * \brief Emitted when the user changes the number of bins.
   *
   * Listeners (e.g., a statistics calculator) should connect to this signal and
   * recalculate histogram data with the updated bin count.
   *
   * \param[in] nBins The new number of bins requested by the user.
   */
  void RequestHistogramUpdate(unsigned int nBins);

protected:
  /** \brief Clears the bar highlight when the cursor leaves the plot canvas. */
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  struct SeriesItem
  {
    HistogramSeries series;
    QwtPlotHistogram* item = nullptr;
  };

  void CreateConnections();
  void SetGUIElementsEnabled(bool enabled);
  /** \brief Applies m_Style (canvas background and axis colors) to the plot. */
  void ApplyTheme();
  /** \brief Detaches and deletes all series items and clears the highlight (does not replot). */
  void RemoveSeries();
  /** \brief Highlights the histogram bar under the cursor, clearing it off any bar. */
  void OnHover(const QPointF& pos);
  /** \brief Removes the current bar highlight (does not replot). */
  void ClearHighlight();

  //slots
  /** \brief Saves the histogram to the clipboard. */
  void OnClipboardButtonClicked();
  /** \brief Enables / Disables SpinBox to change the number of bins. */
  void OnDefaultNBinsCheckBoxChanged();
  /** \brief Emits the signal RequestHistogramUpdate(unsigned int nBins) with the updated value. */
  void OnNBinsSpinBoxValueChanged();
  /** \brief Zooms the plot to the x range given by the min and max spin boxes. */
  void OnZoomRangeEdited();
  /** \brief Mirrors the current zoom rectangle in the min and max spin boxes. */
  void OnZoomed(const QRectF& rect);

private:
  std::unique_ptr<Ui::QmitkHistogramVisualizationControls> m_Controls;
  const unsigned int m_DefaultNBins = 100;
  const unsigned int m_MinNBins = 10;
  const unsigned int m_MaxNBins = 10000;

  QwtPlot* m_Plot;
  QwtPlotHistogram* m_HighlightItem;
  QwtPlotZoomer* m_Zoomer;
  std::vector<SeriesItem> m_Series;
  QmitkPlotStyle m_Style = QmitkPlotStyle::Dark;
};

#endif
