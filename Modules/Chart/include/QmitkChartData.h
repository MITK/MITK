/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkChartData_h
#define QmitkChartData_h

#include <QmitkChartWidget.h>
#include <QVariant>
#include <QApplication>
#include <QClipboard>
#include <QUrl>
#include <QFileDialog>
#include <boost/algorithm/string.hpp>
#include <fstream>

/**
 * \brief Holds global chart properties for chart generation via QWebChannel.
 *
 * This class stores chart-wide configuration properties such as axis labels,
 * title, theme, legend settings, subchart visibility, axis scale, data point
 * size, stacked mode, and view range limits. Properties are exposed via
 * Q_PROPERTY so they can be sent to the JavaScript charting library (plotly)
 * through QWebChannel.
 *
 * \note The actual per-series data points are held in QmitkChartxyData.
 *
 * \sa QmitkChartWidget
 * \sa QmitkChartxyData
 */
class QmitkChartData : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QVariant m_xAxisLabel READ GetXAxisLabel WRITE SetXAxisLabel NOTIFY SignalXAxisLabelChanged);
  Q_PROPERTY(QVariant m_yAxisLabel READ GetYAxisLabel WRITE SetYAxisLabel NOTIFY SignalYAxisLabelChanged);
  Q_PROPERTY(QVariant m_chartTitle READ GetTitle WRITE SetTitle NOTIFY SignalTitleChanged);
  Q_PROPERTY(QVariant m_themeName READ GetThemeName WRITE SetThemeName NOTIFY SignalThemeNameChanged);
  Q_PROPERTY(QVariant m_LegendPosition READ GetLegendPosition WRITE SetLegendPosition NOTIFY SignalLegendPositionChanged);
  Q_PROPERTY(QVariant m_ShowLegend READ GetShowLegend WRITE SetShowLegend NOTIFY SignalShowLegendChanged);
  Q_PROPERTY(QVariant m_ShowErrorBars READ GetShowErrorBars WRITE SetShowErrorBars NOTIFY SignalShowErrorBarsChanged);
  Q_PROPERTY(QVariant m_YAxisScale READ GetYAxisScale WRITE SetYAxisScale NOTIFY SignalYAxisScaleChanged);
  Q_PROPERTY(QVariant m_ShowSubchart READ GetShowSubchart WRITE SetShowSubchart NOTIFY SignalShowSubchartChanged);
  Q_PROPERTY(QVariant m_UsePercentageInPieChart READ GetUsePercentageInPieChart WRITE SetUsePercentageInPieChart NOTIFY SignalUsePercentageInPieChartChanged);
  Q_PROPERTY(QVariant m_DataPointSize READ GetDataPointSize WRITE SetDataPointSize NOTIFY SignalDataPointSizeChanged);
  Q_PROPERTY(QVariant m_StackedData READ GetStackedData WRITE SetStackedData NOTIFY SignalStackedDataChanged);
  Q_PROPERTY(QVariant m_MinValueXView READ GetMinValueXView WRITE SetMinValueXView NOTIFY SignalMinValueXViewChanged);
  Q_PROPERTY(QVariant m_MaxValueXView READ GetMaxValueXView WRITE SetMaxValueXView NOTIFY SignalMaxValueXViewChanged);
  Q_PROPERTY(QVariant m_MinValueYView READ GetMinValueYView WRITE SetMinValueYView NOTIFY SignalMinValueYViewChanged);
  Q_PROPERTY(QVariant m_MaxValueYView READ GetMaxValueYView WRITE SetMaxValueYView NOTIFY SignalMaxValueYViewChanged);

public:
  /** \brief Construct a QmitkChartData with default property values. */
  QmitkChartData();

  /**
   * \brief Configure appearance-related properties in a single call.
   *
   * \param[in] showSubChart Whether to display a subchart (default: true).
   * \param[in] usePercentageInPieChart Whether to show percentages in pie charts (default: false).
   */
  void SetAppearance(bool showSubChart = true, bool usePercentageInPieChart = false);

  /**
   * \brief Get the X axis label text.
   * \return The current X axis label as a QVariant.
   */
  Q_INVOKABLE QVariant GetXAxisLabel() const { return m_xAxisLabel; };
  /**
   * \brief Set the X axis label text.
   * \param[in] label The label text to display on the X axis.
   */
  Q_INVOKABLE void SetXAxisLabel(const QVariant& label) { m_xAxisLabel = label; emit SignalXAxisLabelChanged(label); };

  /**
   * \brief Get the Y axis label text.
   * \return The current Y axis label as a QVariant.
   */
  Q_INVOKABLE QVariant GetYAxisLabel() const { return m_yAxisLabel; };
  /**
   * \brief Set the Y axis label text.
   * \param[in] label The label text to display on the Y axis.
   */
  Q_INVOKABLE void SetYAxisLabel(const QVariant& label) { m_yAxisLabel = label; emit SignalYAxisLabelChanged(label); };

  /**
   * \brief Get the chart title.
   * \return The current chart title as a QVariant.
   */
  Q_INVOKABLE QVariant GetTitle() const { return m_chartTitle; };
  /**
   * \brief Set the chart title.
   * \param[in] title The title text to display above the chart.
   */
  Q_INVOKABLE void SetTitle(const QVariant& title) { m_chartTitle = title; emit SignalTitleChanged(title); };

  /**
   * \brief Get the current theme name.
   * \return The theme name (e.g. "dark" or "light") as a QVariant.
   */
  Q_INVOKABLE QVariant GetThemeName() const { return m_themeName; };
  /**
   * \brief Set the chart theme name.
   * \param[in] themeName The theme identifier string (e.g. "dark", "light").
   */
  Q_INVOKABLE void SetThemeName(const QVariant &themeName)
  {
    m_themeName = themeName;
    emit SignalThemeNameChanged(themeName);
  };

  /**
   * \brief Get the legend position.
   * \return The current legend position as a QVariant string.
   */
  Q_INVOKABLE QVariant GetLegendPosition() const { return m_LegendPosition; };
  /**
   * \brief Set the legend position.
   * \param[in] legendPosition The position identifier (e.g. "topRight", "bottomMiddle").
   */
  Q_INVOKABLE void SetLegendPosition(const QVariant& legendPosition) { m_LegendPosition = legendPosition; emit SignalLegendPositionChanged(legendPosition); };

  /**
   * \brief Get whether the legend is shown.
   * \return true if the legend is visible, as a QVariant.
   */
  Q_INVOKABLE QVariant GetShowLegend() const { return m_ShowLegend; };
  /**
   * \brief Set whether the legend is shown.
   * \param[in] show true to show the legend, false to hide it.
   */
  Q_INVOKABLE void SetShowLegend(const QVariant& show) { m_ShowLegend = show; emit SignalShowLegendChanged(show); };

  /**
   * \brief Get whether error bars are shown.
   * \return true if error bars are visible, as a QVariant.
   */
  Q_INVOKABLE QVariant GetShowErrorBars() const { return m_ShowErrorBars; };
  /**
   * \brief Set whether error bars are shown.
   * \param[in] show true to show error bars, false to hide them.
   */
  Q_INVOKABLE void SetShowErrorBars(const QVariant &show)
  {
    m_ShowErrorBars = show;
    emit SignalShowErrorBarsChanged(show);
  };

  /**
   * \brief Get the Y axis scale type.
   * \return The axis scale identifier (e.g. "linear" or "log") as a QVariant.
   */
  Q_INVOKABLE QVariant GetYAxisScale() const { return m_YAxisScale; };
  /**
   * \brief Set the Y axis scale type.
   * \param[in] YAxisScale The axis scale identifier (e.g. "linear" or "log").
   */
  Q_INVOKABLE void SetYAxisScale(const QVariant& YAxisScale) { m_YAxisScale = YAxisScale; emit SignalYAxisScaleChanged(YAxisScale); };

  /**
   * \brief Get whether the subchart (overview navigator) is shown.
   * \return true if the subchart is visible, as a QVariant.
   */
  Q_INVOKABLE QVariant GetShowSubchart() const { return m_ShowSubchart; };
  /**
   * \brief Set whether the subchart (overview navigator) is shown.
   * \param[in] showSubchart true to display the subchart, false to hide it.
   */
  Q_INVOKABLE void SetShowSubchart(const QVariant& showSubchart) { m_ShowSubchart = showSubchart; emit SignalShowSubchartChanged(showSubchart); };

  /**
   * \brief Get whether percentages are shown in pie charts.
   * \return true if percentages are displayed, as a QVariant.
   */
  Q_INVOKABLE QVariant GetUsePercentageInPieChart() const { return m_UsePercentageInPieChart; };
  /**
   * \brief Set whether percentages are shown in pie charts.
   * \param[in] usePercentageInPieChart true to show percentage labels.
   */
  Q_INVOKABLE void SetUsePercentageInPieChart(const QVariant& usePercentageInPieChart) { m_UsePercentageInPieChart = usePercentageInPieChart; emit SignalUsePercentageInPieChartChanged(usePercentageInPieChart); };

  /**
   * \brief Get the data point marker size.
   * \return The data point size (0 means no markers), as a QVariant.
   */
  Q_INVOKABLE QVariant GetDataPointSize() const { return m_DataPointSize; };
  /**
   * \brief Set the data point marker size.
   * \param[in] showDataPoints The marker size; 0 hides data point markers.
   */
  Q_INVOKABLE void SetDataPointSize(const QVariant& showDataPoints) { m_DataPointSize = showDataPoints; emit SignalDataPointSizeChanged(showDataPoints); };

  /**
   * \brief Get whether data is displayed in stacked mode.
   * \return true if stacked mode is active, as a QVariant.
   */
  Q_INVOKABLE QVariant GetStackedData() const { return m_StackedData; };
  /**
   * \brief Set whether data is displayed in stacked mode.
   * \param[in] stackedData true to stack data series.
   */
  Q_INVOKABLE void SetStackedData(const QVariant& stackedData) { m_StackedData = stackedData; emit SignalStackedDataChanged(m_StackedData); };

  /**
   * \brief Get the minimum X axis view value.
   * \return The minimum X view boundary as a QVariant.
   */
   Q_INVOKABLE QVariant GetMinValueXView() const { return m_MinValueXView; };
  /**
   * \brief Set the minimum X axis view value for zooming.
   * \param[in] minValueXView The minimum X value to display.
   */
  Q_INVOKABLE void SetMinValueXView(const QVariant &minValueXView)
  {
    m_MinValueXView = minValueXView;
    emit SignalMinValueXViewChanged(m_MinValueXView);
  };

  /**
   * \brief Get the maximum X axis view value.
   * \return The maximum X view boundary as a QVariant.
   */
     Q_INVOKABLE QVariant GetMaxValueXView() const { return m_MaxValueXView; };
  /**
   * \brief Set the maximum X axis view value for zooming.
   * \param[in] maxValueXView The maximum X value to display.
   */
  Q_INVOKABLE void SetMaxValueXView(const QVariant &maxValueXView)
  {
    m_MaxValueXView = maxValueXView;
    emit SignalMaxValueXViewChanged(m_MaxValueXView);
  };

  /**
   * \brief Get the minimum Y axis view value.
   * \return The minimum Y view boundary as a QVariant.
   */
     Q_INVOKABLE QVariant GetMinValueYView() const { return m_MinValueYView; };
  /**
   * \brief Set the minimum Y axis view value for zooming.
   * \param[in] minValueYView The minimum Y value to display.
   */
  Q_INVOKABLE void SetMinValueYView(const QVariant &minValueYView)
  {
    m_MinValueYView = minValueYView;
    emit SignalMinValueYViewChanged(m_MinValueYView);
  };

  /**
   * \brief Get the maximum Y axis view value.
   * \return The maximum Y view boundary as a QVariant.
   */
     Q_INVOKABLE QVariant GetMaxValueYView() const { return m_MaxValueYView; };
  /**
   * \brief Set the maximum Y axis view value for zooming.
   * \param[in] maxValueYView The maximum Y value to display.
   */
  Q_INVOKABLE void SetMaxValueYView(const QVariant &maxValueYView)
  {
    m_MaxValueYView = maxValueYView;
    emit SignalMaxValueYViewChanged(m_MaxValueYView);
  };

  /**
   * \brief Emit the SignalImageUrl signal to trigger SVG export in JavaScript.
   */
  void EmitSignalImageUrl()
  {
    emit SignalImageUrl();
  };

signals:
  void SignalYAxisLabelChanged(const QVariant label);     ///< Emitted when the Y axis label changes.
  void SignalXAxisLabelChanged(const QVariant label);     ///< Emitted when the X axis label changes.
  void SignalLegendPositionChanged(const QVariant legendPosition); ///< Emitted when the legend position changes.
  void SignalShowLegendChanged(const QVariant show);      ///< Emitted when legend visibility changes.
  void SignalShowErrorBarsChanged(const QVariant show);   ///< Emitted when error bar visibility changes.
  void SignalYAxisScaleChanged(const QVariant YAxisScale); ///< Emitted when the Y axis scale changes.
  void SignalTitleChanged(const QVariant title);          ///< Emitted when the chart title changes.
  void SignalThemeNameChanged(const QVariant themeName);  ///< Emitted when the theme changes.
  void SignalShowSubchartChanged(const QVariant showSubchart); ///< Emitted when subchart visibility changes.
  void SignalUsePercentageInPieChartChanged(const QVariant usePercentageInPieChart); ///< Emitted when pie chart percentage mode changes.
  void SignalDataPointSizeChanged(const QVariant showDataPoints); ///< Emitted when data point size changes.
  void SignalStackedDataChanged(const QVariant stackedData); ///< Emitted when stacked data mode changes.
  void SignalMinValueXViewChanged(const QVariant minValueXView); ///< Emitted when minimum X view value changes.
  void SignalMaxValueXViewChanged(const QVariant maxValueXView); ///< Emitted when maximum X view value changes.
  void SignalMinValueYViewChanged(const QVariant minValueYView); ///< Emitted when minimum Y view value changes.
  void SignalMaxValueYViewChanged(const QVariant maxValueYView); ///< Emitted when maximum Y view value changes.
  void SignalImageUrl(); ///< Emitted to request the chart image URL from JavaScript for SVG export.

public slots:
  /**
   * \brief Slot that receives an SVG image URL from JavaScript and saves it to a file.
   *
   * Decodes the percent-encoded SVG data, prompts the user with a file
   * dialog, and writes the SVG content to the selected file.
   *
   * \param[in] datafromjs The percent-encoded SVG data URL from JavaScript.
   */
  void slotImageUrl(const QString &datafromjs)
  {
    QString ds = QUrl::fromPercentEncoding(datafromjs.toLatin1());

    QString filename = QFileDialog::getSaveFileName(
          0,
          tr("Save Plot"),
          "my_plot.svg",
          tr("Scalable Vector Graphics (*.svg)") );
    if (filename.isEmpty())
      return;

    std::string out_image = ds.toStdString();
    boost::algorithm::replace_first(out_image, "data:image/svg+xml,", "");
    std::ofstream outfile(filename.toStdString());
    outfile.write(out_image.c_str(), out_image.size());
    outfile.close();
  }

private:
  QVariant m_xAxisLabel;
  QVariant m_yAxisLabel;
  QVariant m_chartTitle;
  QVariant m_themeName = "dark";

  QVariant m_ShowLegend = true;
  QVariant m_ShowErrorBars;
  QVariant m_LegendPosition = "topRight";
  QVariant m_ShowSubchart;
  QVariant m_YAxisScale;
  QVariant m_UsePercentageInPieChart;
  QVariant m_numberDatasets;
  QVariant m_DataPointSize = 0;
  QVariant m_StackedData;
  QVariant m_MinValueXView;
  QVariant m_MaxValueXView;
  QVariant m_MinValueYView;
  QVariant m_MaxValueYView;
};

#endif
