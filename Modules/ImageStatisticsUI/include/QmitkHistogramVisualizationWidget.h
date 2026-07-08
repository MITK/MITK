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

#include <QPointF>
#include <QWidget>

#include <map>
#include <memory>
#include <string>

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
 * to the clipboard. Multiple histograms can be displayed simultaneously, each
 * identified by a data label.
 *
 * \sa QmitkImageStatisticsWidget
 */

class MITKIMAGESTATISTICSUI_EXPORT QmitkHistogramVisualizationWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the histogram visualization widget.
   * \param[in] parent Optional parent widget.
   */
  QmitkHistogramVisualizationWidget(QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkHistogramVisualizationWidget() override;

  /**
   * \brief Displays a histogram in the chart and enables the GUI controls.
   *
   * If a histogram with the given label already exists, it is updated. Otherwise, a new
   * histogram series is added to the chart.
   *
   * \param[in] histogram The ITK histogram to display. If nullptr, the call is ignored.
   * \param[in] dataLabel A unique label identifying this histogram series.
   */
  void SetHistogram(itk::Statistics::Histogram<double>::ConstPointer histogram, const std::string &dataLabel);

  /**
   * \brief Clears all histogram data and disables all GUI elements.
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
   * Enables the default number of bins checkbox and sets bins to 100.
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
  void CreateConnections();
  void SetGUIElementsEnabled(bool enabled);
  /** \brief Applies m_Style (canvas background and axis colors) to the plot. */
  void ApplyTheme();
  /** \brief Highlights the histogram bar under the cursor, clearing it off any bar. */
  void OnHover(const QPointF& pos);
  /** \brief Removes the current bar highlight (does not replot). */
  void ClearHighlight();

//slots
	/** \brief  Saves the histogram to the clipboard. */
	void OnClipboardButtonClicked();
	/** \brief Enables / Disables SpinBox to change the number of bins. */
	void OnDefaultNBinsCheckBoxChanged();
	/** \brief Emits the signal RequestHistogramUpdate(unsigned int nBins) with the updated value. */
	void OnNBinsSpinBoxValueChanged();
	/** \brief Enables / Disables SpinBoxes to set custom min and max values */
	void OnViewMinMaxCheckBoxChanged();
	/**\brief */
	void OnMaxValueSpinBoxValueChanged();
	/** \brief */
	void OnMinValueSpinBoxValueChanged();

private:
  std::unique_ptr<Ui::QmitkHistogramVisualizationControls> m_Controls;
  const unsigned int m_DefaultNBins = 100;
  const unsigned int m_MinNBins = 10;
  const unsigned int m_MaxNBins = 10000;

  QwtPlot* m_Plot;
  QwtPlotHistogram* m_HighlightItem;
  QwtPlotZoomer* m_Zoomer;
  std::map<std::string, QwtPlotHistogram*> m_HistogramItems;
  std::map<std::string, itk::Statistics::Histogram<double>::ConstPointer> m_Histograms;
  QmitkPlotStyle m_Style = QmitkPlotStyle::Dark;
};

#endif
