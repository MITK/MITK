/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkIntensityProfileVisualizationWidget_h
#define QmitkIntensityProfileVisualizationWidget_h

#include <MitkImageStatisticsUIExports.h>

#include <QmitkPlotStyle.h>

//mitk
#include <mitkIntensityProfile.h>

#include <QWidget>

#include <memory>
#include <string>
#include <vector>

class QmitkPlotWidget;
class QwtPlotZoomer;

namespace Ui
{
  class QmitkIntensityProfileControls;
}

/**
 * \brief Widget for displaying intensity profile line charts along a path.
 *
 * Provides a Qwt-based visualization of intensity values along a line or path,
 * with a control for copying the intensity data to the clipboard. The plot
 * displays distance on the X-axis and intensity values on the Y-axis.
 *
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
   * Converts the intensity profile to a vector of values and adds it to the plot.
   * If the profile is nullptr or empty, the call is ignored.
   *
   * \param[in] intensityProfile The intensity profile to visualize. May be nullptr.
   * \param[in] dataLabel A label identifying this data series in the plot.
   */
  void SetIntensityProfile(mitk::IntensityProfile::ConstPointer intensityProfile, const std::string& dataLabel);

  /**
   * \brief Clears the plot and disables all GUI elements.
   */
  void Reset();

  /**
   * \brief Sets the color theme for the plot.
   * \param[in] style The plot style to apply (dark or light).
   */
  void SetTheme(QmitkPlotStyle style);

private:

  void CreateConnections();

  void SetGUIElementsEnabled(bool enabled);

  /** \brief Applies m_Style (canvas background and axis colors) to the plot. */
  void ApplyTheme();

  /** \brief  Saves the intensity profile to the clipboard. */
  void OnClipboardButtonClicked();

  std::unique_ptr<Ui::QmitkIntensityProfileControls> m_Controls;
  QmitkPlotWidget* m_PlotWidget;
  QwtPlotZoomer* m_Zoomer;
  QmitkPlotStyle m_Style = QmitkPlotStyle::Dark;

  std::vector<double> m_IntensityProfileList;
};

#endif
