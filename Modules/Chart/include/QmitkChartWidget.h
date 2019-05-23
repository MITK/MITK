/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef QmitkC3jsWidget_h
#define QmitkC3jsWidget_h

#include <map>
#include <memory>
#include <QWidget>

#include <MitkChartExports.h>

/*!
\brief QmitkChartWidget is a widget to display various charts based on the javascript chart library plotly.
* \details Data is added via AddData1D() or AddData2D().\n
* There can be multiple charts (of different types with different properties) created by calling AddData1D or AddData2D multiple times.\n\n
* The following chart types are supported:
* * line chart
* * bar chart
* * spline chart
* * pie chart
* * scatter chart
* * area chart
* * area spline chart.
*
* Technical details: The javascript code is embedded in a QWebEngineView. The actual js code is implemented in resource\Chart.js.
* \sa https://plot.ly/javascript/ for further information about the used javaScript library.
* \ingroup Modules/Chart
*/
class MITKCHART_EXPORT QmitkChartWidget : public QWidget
{
  Q_OBJECT

public:
  /*!
  * \brief enum of diagram types.
  */
  enum class ChartType {
    bar, /*!< bar chart, see https://plot.ly/javascript/bar-charts/ */
    line, /*!< line chart, see https://plot.ly/javascript/line-charts/ */
    spline, /*!< spline chart (smoothed line chart), see https://plot.ly/~jduelfer/23/spline/#/ */
    pie, /*!< pie chart, see https://plot.ly/javascript/pie-charts/ */
    area, /*!< area chart, see https://plot.ly/javascript/filled-area-plots/ */
    area_spline, /*!< area-spline chart, similar to https://plot.ly/~jduelfer/23/spline/#/ */
    scatter /*!< scatter chart, see https://plot.ly/javascript/line-and-scatter/ */
  };
  /*!
  * \brief enum of chart style (modifies background and line color).
  */
  enum class ColorTheme {
    darkstyle, /*!< background color: dark gray, foreground color: white*/
    lightstyle /*!< background color: white, foreground color: black */
  };
  enum class LineStyle {
    solid,
    dashed
  };
  enum class AxisScale {
    linear,
    log
  };

  /*!
  * \brief enum of legend position.
  * See https://plot.ly/javascript/legend/
  */
  enum class LegendPosition {
    bottomMiddle,
    bottomRight,
    topRight,
    topLeft,
    middleRight
  };

  explicit QmitkChartWidget(QWidget* parent = nullptr);
  //for UnitTests
  explicit QmitkChartWidget(QWidget *parent, bool unitTest);
  ~QmitkChartWidget() override;

  /*!
  * \brief Adds 1D data to the widget
  * \details internally, the list is converted to a map with increasing integers keys starting at 0.
  * \param label the name of the data that is also used as identifier.
  * \param chartType the chart type that should be used for this data entry
  * \note the data can be cleared with ClearDiagram()
  * \note If the label name already exists, the name is replaced with a unique one by concatenating numbers to it.
  * \warning Pie chart is significantly different than the other chart types. Here, the data given by AddData1D is summed. Each entry represents a different category.
  */
  void AddData1D(const std::vector<double>& data1D, const std::string& label, ChartType chartType = ChartType::bar);

   /*!
   * \brief Updates data of an existing label
   * \param data1D the 1D data , \sa AddData1D
   * \param label the (existing) label
   * \note if the label does not exist, nothing happens
   */
  void UpdateData1D(const std::vector<double> &data1D, const std::string &label);

  /*!
   * \sa UpdateData1D
   * \sa AddData2D
   */
  void UpdateData2D(const std::map<double, double> &data2D, const std::string &label);

  /*!
  * \brief Adds 2D data to the widget. Call repeatedly for displaying multiple charts.
  * \details each entry represents a data point: key: value --> x-value: y-value.
  * \param label the name of the data that is also used as identifier.
  * \param chartType the chart type that should be used for this data entry
  * \note the data can be cleared with ClearDiagram()
  * \note If the label name already exists, the name is replaced with a unique one by concatenating numbers to it.
  * \warning Pie chart is significantly different than the other chart types. Here, the data given by AddData1D is summed. Each entry represents a different category.
  */
  void AddData2D(const std::map<double, double> &data2D,
                 const std::string &label,
                 ChartType chartType = ChartType::bar);

  /*!
  * \brief Removes data from the widget, works for 1D and 2D Data
  * \param label the name of the data that is also used as identifier.
  * \note All data can be cleared with ClearDiagram()
  * \throws Invalid Argument Exception when the label cannot be found
  */
  void RemoveData(const std::string& label);

  void UpdateLabel(const std::string& existingLabel, const std::string& newLabel);

  /*!
  * \brief Sets the color of one data entry (identifier is previously assigned label)
  * \details the color name can be "red" or a hex number (#FF0000).
  * \warning Either define all data entries with a color or no data entry. If a mixed approach is used,
  * plotly choses the color of the data entry (that could be the same as a user defined color).
  * \note If an unknown label is given, nothing happens.
  *  \sa https://www.w3schools.com/cssref/css_colors.asp
  */
  void SetColor(const std::string& label, const std::string& colorName);

  /*!
  * \brief Sets the line style of one data entry (identifier is previously assigned label)
  * \details two line styles are possible: LineStyle::solid and LineStyle::dashed.
  * The default line style is solid.
  * \note If an unknown label is given, nothing happens.
  *  \warning only sets the line style if the current chart type is ChartType::line.
  *  However, the line style remains also if the chart changes (e.g. new chart type)
  */
  void SetLineStyle(const std::string& label, LineStyle style);

  /*!
  * \brief Sets the axis scale to either linear (default) or logarithmic.
  * \sa https://plot.ly/javascript/log-plot/
  */
  void SetYAxisScale(AxisScale scale);

  void SetXAxisLabel(const std::string& label);

  void SetYAxisLabel(const std::string& label);

   /*!
   * \brief Sets labels for pie chart data.
   * \note in AddData1D, the label still has to be given that acts as a unique id. However, the label is omitted then.
   */
  void SetPieLabels(const std::vector<std::string> &pieLabels, const std::string &label);
  /*!
  * \brief Sets a title for the chart.
  */
  void SetTitle(const std::string &title);

  /*!
  * \brief Sets the chart type for a data entry
  * \details for available types, see ChartType
  * \note If an unknown label is given, nothing happens.
  * \warning Pie chart is significantly different than the other chart types. Here, the data given by AddData1D is summed. Each entry represents a different category.
  * \sa DiagramType for available types
  */
  void SetChartType(const std::string& label, ChartType type);
  /*!
   * \brief Sets error bars for data in x direction
   * \note If only error plus is provided, the error bars are symmetrical
   * \param label the name of the data that is also used as identifier.
   * \param errorPlus the error in positive direction
   * \param errorMinus the error in negative direction. Same as error plus if omitted 
   */
  void SetXErrorBars(const std::string &label, const std::vector<double> &errorPlus, const std::vector<double>& errorMinus = std::vector<double>());

   /*!
  * \brief Sets error bars for data in y direction
  * \details for parameters, see SetXErrorBars
  * \note If only error plus is provided, the error bars are symmetrical
  */
  void SetYErrorBars(const std::string &label, const std::vector<double> &errorPlus, const std::vector<double> &errorMinus = std::vector<double>());

  /*!
  * \brief Sets the legend position.
  * \details Default position is bottom.
  * \sa LegendPosition for available types
  */
  void SetLegendPosition(LegendPosition position);

  void SetShowLegend(bool show);

  void SetStackedData(bool stacked);

  /*!
  * \brief Displays the chart in the widget
  * \param showSubChart if a subchart is displayed inside the widget or not.
  * \note if no data has been provided, (\sa AddData1D AddData2D), an empty chart is displayed.
  */
  void Show(bool showSubChart=false);

  /*!
  * \brief Either displays the dataPoints or not
  * \param showDataPoints if dataPoints are displayed inside the widget or not.
  * \details: example for not showing points: https://plot.ly/javascript/line-charts/#styling-line-plot
  * example for showing the points: https://plot.ly/javascript/pointcloud/
  */
  void SetShowDataPoints(bool showDataPoints);

  /*!
  * \brief Clears all data inside and resets the widget.
  */
  void Clear();

  /*!
  * \brief Sets the theme of the widget.
  * \details default is dark theme as in MITK.
  * \warning has to be called before Show() or Reload() to work
  */
  void SetTheme(ColorTheme themeEnabled);

  /*!
  * \brief Sets whether the subchart shall be shown.
  * \details Changes the state of the current chart object. 
  * \note Needs to be reloaded with Reload() to display changes.
  */
  void SetShowSubchart(bool showSubChart);

  /*!
   * \brief Sets whether the error bars shall be shown.
   * \details Changes the state of the current chart object. 
   * \note Needs to be reloaded with Reload() to display changes.
   * \param showErrorBars if error bars are displayed or not.
   */
  void SetShowErrorBars(bool showErrorBars);

  /*!
   * \brief Sets the min and max x values of the chart
   * \details Zooms in to view the values between minValue and maxValue in x direction
   */
  void SetMinMaxValueXView(double minValueX,double maxValueX);
  /*!
   * \brief Sets the min and max y values of the chart
   * \details Zooms in to view the values between minValue and maxValue in y direction
   */
  void SetMinMaxValueYView(double minValueY, double maxValueY);

  /*!
   * \brief Reloads the chart in the widget
   * \details reloading may be needed to display added data in an existing chart
   */
  void Reload();

  QSize sizeHint() const override;

public slots:
  void OnLoadFinished(bool isLoadSuccessful);
  void OnPageSuccessfullyLoaded();

signals:
  void PageSuccessfullyLoaded();

private:
  /*! source: https://stackoverflow.com/questions/29383/converting-bool-to-text-in-c*/
  std::string convertBooleanValue(bool value) const;

  class Impl;
  std::unique_ptr<Impl> m_Impl;
};

#endif
