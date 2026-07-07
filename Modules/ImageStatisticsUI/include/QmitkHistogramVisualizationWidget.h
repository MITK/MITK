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

// itk
#include <itkHistogram.h>

#include <QmitkChartWidget.h>
#include <memory>

namespace Ui
{
  class QmitkHistogramVisualizationControls;
}

/**
 * \brief Widget for displaying and interacting with histogram visualizations.
 *
 * Provides a chart-based histogram display with controls for adjusting the number of bins,
 * toggling a subchart view, setting custom min/max value ranges, and copying histogram data
 * to the clipboard. Multiple histograms can be displayed simultaneously, each identified by
 * a data label.
 *
 * \sa QmitkChartWidget
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
   * \brief Sets the color theme for the chart widget.
   * \param[in] style The color theme to apply (dark or light).
   */
  void SetTheme(QmitkChartWidget::ColorTheme style);

  /**
   * \brief Resets all controls to their default settings.
   *
   * Enables the default number of bins checkbox, sets bins to 100, and disables the subchart.
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

private:
  void CreateConnections();
  void SetGUIElementsEnabled(bool enabled);
  /** \brief Helper function to convert the histogram in order to forward it to the ChartWidget. */
  std::vector<std::pair<double, double> > ConvertHistogramToPairList(itk::Statistics::Histogram<double>::ConstPointer histogram) const;

//slots
	/** \brief  Saves the histogram to the clipboard. */
	void OnClipboardButtonClicked();
	/** \brief Enables / Disables SpinBox to change the number of bins. */
	void OnDefaultNBinsCheckBoxChanged();
	/** \brief Emits the signal RequestHistogramUpdate(unsigned int nBins) with the updated value. */
	void OnNBinsSpinBoxValueChanged();
	/** \brief Shows / Hides the subchart. */
	void OnShowSubchartCheckBoxChanged();
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

  std::map < std::string, itk::Statistics::Histogram<double>::ConstPointer> m_Histograms;
};

#endif
