/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkIntensityProfileVisualizationWidget_h
#define QmitkIntensityProfileVisualizationWidget_h

//Qt
#include <QmitkChartWidget.h>

#include <MitkImageStatisticsUIExports.h>

//mitk
#include <mitkIntensityProfile.h>
#include <memory>

namespace Ui
{
  class QmitkIntensityProfileControls;
};

/**
 * \brief Widget for displaying intensity profile line charts along a path.
 *
 * Provides a chart-based visualization of intensity values along a line or path,
 * with controls for toggling a subchart overview and copying the intensity data
 * to the clipboard. The chart displays distance on the X-axis and intensity values
 * on the Y-axis.
 *
 * \sa QmitkChartWidget
 * \sa QmitkHistogramVisualizationWidget
 * \sa mitk::IntensityProfile
 */

class MITKIMAGESTATISTICSUI_EXPORT QmitkIntensityProfileVisualizationWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the intensity profile visualization widget.
   * \param[in] parent Optional parent widget.
   */
  QmitkIntensityProfileVisualizationWidget(QWidget* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkIntensityProfileVisualizationWidget() override;

  /**
   * \brief Displays an intensity profile as a line chart.
   *
   * Converts the intensity profile to a vector of values and adds it to the chart widget.
   * If the profile is nullptr or empty, the call is ignored.
   *
   * \param[in] intensityProfile The intensity profile to visualize. May be nullptr.
   * \param[in] dataLabel A label identifying this data series in the chart.
   */
  void SetIntensityProfile(mitk::IntensityProfile::ConstPointer intensityProfile, const std::string& dataLabel);

  /**
   * \brief Clears the chart and disables all GUI elements.
   */
  void Reset();

  /**
   * \brief Sets the color theme for the chart widget.
   *
   * The theme is applied when the chart page finishes loading.
   *
   * \param[in] style The color theme to apply (dark or light).
   */
  void SetTheme(QmitkChartWidget::ColorTheme style);

private:

  void CreateConnections();

  void SetGUIElementsEnabled(bool enabled);

  std::vector<double> ConvertIntensityProfileToVector(mitk::IntensityProfile::ConstPointer intensityProfile) const;

  /** \brief  Saves the intensity profile to the clipboard. */
  void OnClipboardButtonClicked();
  /** \brief Shows / Hides the subchart. */
  void OnShowSubchartCheckBoxChanged();

  void OnPageSuccessfullyLoaded();

private:

  std::unique_ptr<Ui::QmitkIntensityProfileControls> m_Controls;
  QmitkChartWidget::ColorTheme m_ChartStyle = QmitkChartWidget::ColorTheme::darkstyle;

  std::vector<double> m_IntensityProfileList;
};

#endif
