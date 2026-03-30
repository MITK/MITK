/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkChartWidget_h
#define QmitkChartWidget_h

#include <map>
#include <memory>
#include <QWidget>

#include <MitkChartExports.h>

class QmitkChartxyData;

/**
 * \brief Qt widget for displaying interactive charts based on the plotly.js library.
 *
 * QmitkChartWidget provides a high-level C++ API for creating interactive charts
 * within MITK applications. Data is added via AddData1D() or AddData2D(), and
 * multiple data series with different chart types and properties can be displayed
 * simultaneously.
 *
 * Supported chart types:
 * - Line chart
 * - Bar chart
 * - Spline chart (smoothed line)
 * - Pie chart
 * - Scatter chart
 * - Area chart
 * - Area spline chart
 *
 * Features include configurable colors, line styles, marker symbols, error bars,
 * legend positioning, axis labels and scales, themes, and data point visibility.
 *
 * Internally, chart rendering is handled by JavaScript (plotly.js) embedded in a
 * QWebEngineView. Data and configuration are passed to JavaScript via QWebChannel.
 *
 * \sa https://plot.ly/javascript/ for the underlying JavaScript charting library.
 * \sa QmitkChartData
 * \sa QmitkChartxyData
 * \ingroup Modules/Chart
 */
class MITKCHART_EXPORT QmitkChartWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Enumeration of supported chart types.
   */
  enum class ChartType {
    bar,         ///< Bar chart. \sa https://plot.ly/javascript/bar-charts/
    line,        ///< Line chart. \sa https://plot.ly/javascript/line-charts/
    spline,      ///< Spline chart (smoothed line). \sa https://plot.ly/~jduelfer/23/spline/
    pie,         ///< Pie chart. \sa https://plot.ly/javascript/pie-charts/
    area,        ///< Filled area chart. \sa https://plot.ly/javascript/filled-area-plots/
    area_spline, ///< Area chart with spline interpolation.
    scatter      ///< Scatter chart. \sa https://plot.ly/javascript/line-and-scatter/
  };

  /**
   * \brief Enumeration of chart color themes.
   */
  enum class ColorTheme {
    darkstyle,  ///< Dark gray background, white foreground.
    lightstyle  ///< White background, black foreground.
  };

  /**
   * \brief Enumeration of line drawing styles.
   */
  enum class LineStyle {
    solid,  ///< Solid line.
    dashed  ///< Dashed line.
  };

  /**
   * \brief Enumeration of data point marker symbols.
   */
  enum class MarkerSymbol {
    circle,             ///< Circle marker.
    diamond,            ///< Diamond marker.
    cross,              ///< Cross (+) marker.
    square,             ///< Square marker.
    pentagon,           ///< Pentagon marker.
    star,               ///< Star marker.
    x,                  ///< X marker.
    diamond_tall,       ///< Tall diamond marker.
    star_diamond,       ///< Star-diamond marker.
    star_triangle_up,   ///< Upward star-triangle marker.
    star_triangle_down, ///< Downward star-triangle marker.
    asterisk,           ///< Asterisk marker.
    cross_thin,         ///< Thin cross marker.
    x_thin              ///< Thin X marker.
  };

  /**
   * \brief Enumeration of named chart colors.
   */
  enum class ChartColor {
    red,        ///< Red
    orange,     ///< Orange
    yellow,     ///< Yellow
    green,      ///< Green
    blue,       ///< Blue
    purple,     ///< Purple
    brown,      ///< Brown
    magenta,    ///< Magenta
    tan,        ///< Tan
    cyan,       ///< Cyan
    olive,      ///< Olive
    maroon,     ///< Maroon
    navy,       ///< Navy
    aquamarine, ///< Aquamarine
    turqouise,  ///< Turquoise
    silver,     ///< Silver
    lime,       ///< Lime
    teal,       ///< Teal
    indigo,     ///< Indigo
    violet,     ///< Violet
    pink,       ///< Pink
    black,      ///< Black
    white,      ///< White
    grey        ///< Grey
  };

  /**
   * \brief Enumeration of axis scale types.
   */
  enum class AxisScale {
    linear, ///< Linear axis scale (default).
    log     ///< Logarithmic axis scale.
  };

  /**
   * \brief Enumeration of legend positions within the chart.
   * \sa https://plot.ly/javascript/legend/
   */
  enum class LegendPosition {
    bottomMiddle, ///< Bottom center of the chart.
    bottomRight,  ///< Bottom right corner.
    topRight,     ///< Top right corner (default).
    topLeft,      ///< Top left corner.
    middleRight   ///< Middle right side.
  };

  /**
   * \brief Construct a chart widget.
   * \param[in] parent The parent QWidget (default: nullptr).
   */
  explicit QmitkChartWidget(QWidget* parent = nullptr);

  /**
   * \brief Construct a chart widget for unit testing.
   * \param[in] parent The parent QWidget.
   * \param[in] unitTest If true, the widget is constructed in unit test mode.
   */
  explicit QmitkChartWidget(QWidget *parent, bool unitTest);

  /** \brief Destructor. */
  ~QmitkChartWidget() override;

  /**
   * \brief Add a 1D data series to the chart.
   *
   * The values are automatically assigned integer x-values starting at 0.
   *
   * \param[in] data1D The y-values of the data series.
   * \param[in] label A unique name for this data series, also used as the display label.
   * \param[in] chartType The chart type to use for this series (default: bar).
   *
   * \note If the label already exists, a unique suffix is appended.
   * \note Data can be cleared with Clear().
   * \warning For pie charts, each element represents a separate category and values are summed.
   */
  void AddData1D(const std::vector<double>& data1D, const std::string& label, ChartType chartType = ChartType::bar);

  /**
   * \brief Update the data of an existing 1D data series.
   *
   * \param[in] data1D The new y-values.
   * \param[in] label The label of the existing data series.
   *
   * \note If the label does not exist, nothing happens.
   *
   * \sa AddData1D
   */
  void UpdateData1D(const std::vector<double> &data1D, const std::string &label);

  /**
   * \brief Update the data of an existing 2D data series.
   *
   * \param[in] data2D The new (x, y) value pairs.
   * \param[in] label The label of the existing data series.
   *
   * \note If the label does not exist, nothing happens.
   *
   * \sa UpdateData1D
   * \sa AddData2D
   */
  void UpdateData2D(const std::vector< std::pair<double, double> > &data2D, const std::string &label);

  /**
   * \brief Update chart example data with full property specification.
   *
   * \param[in] data2D The (x, y) data pairs.
   * \param[in] label The data series label.
   * \param[in] type The chart type as a string.
   * \param[in] color The color as a string.
   * \param[in] lineStyle The line style as a string.
   * \param[in] pieLabelsData Optional pie chart labels (default: empty).
   */
  void UpdateChartExampleData(const std::vector< std::pair<double, double> >& data2D,
                              const std::string& label,
                              const std::string& type,
                              const std::string& color,
                              const std::string& lineStyle,
                              const std::string& pieLabelsData = 0);

  /**
   * \brief Add a 2D data series to the chart.
   *
   * Each entry is an (x, y) pair. Call repeatedly to display multiple series.
   *
   * \param[in] data2D The (x-value, y-value) data pairs.
   * \param[in] label A unique name for this data series.
   * \param[in] chartType The chart type to use (default: bar).
   *
   * \note If the label already exists, a unique suffix is appended.
   * \note Data can be cleared with Clear().
   * \warning For pie charts, each element represents a separate category.
   */
  void AddData2D(const std::vector< std::pair<double, double> > &data2D,
                 const std::string &label,
                 ChartType chartType = ChartType::bar);

  /**
   * \brief Add chart example data with full property specification.
   *
   * Convenience method for the chart example plugin that accepts chart
   * properties as strings rather than enum values.
   *
   * \param[in] data2D The (x, y) data pairs.
   * \param[in] label The data series label.
   * \param[in] type The chart type as a string.
   * \param[in] color The color as a string.
   * \param[in] style The line style as a string.
   * \param[in] pieLabelsData Optional pie chart labels (default: empty).
   */
  void AddChartExampleData(const std::vector< std::pair<double, double> >& data2D,
                           const std::string& label,
                           const std::string& type,
                           const std::string& color,
                           const std::string& style,
                           const std::string& pieLabelsData = 0);

  /**
   * \brief Remove a data series from the chart.
   *
   * Works for both 1D and 2D data.
   *
   * \param[in] label The label identifier of the data series to remove.
   *
   * \throw std::invalid_argument If the label is not found.
   *
   * \note Use Clear() to remove all data at once.
   */
  void RemoveData(const std::string& label);

  /**
   * \brief Rename an existing data series.
   *
   * \param[in] existingLabel The current label of the data series.
   * \param[in] newLabel The new label to assign.
   */
  void UpdateLabel(const std::string& existingLabel, const std::string& newLabel);

  /**
   * \brief Retrieve the data element for a given label.
   *
   * \param[in] label The label identifier of the data series.
   * \return A pointer to the QmitkChartxyData, or nullptr if not found.
   */
  QmitkChartxyData *GetDataElementByLabel(const std::string& label) const;

  /**
   * \brief Set the color of a data series.
   *
   * \param[in] label The label identifier of the data series.
   * \param[in] colorName The color as a CSS color name or hex code (e.g. "red" or "#FF0000").
   *
   * \note If an unknown label is given, nothing happens.
   * \warning Either set colors for all series or none. Mixed approaches may
   *          cause plotly to assign conflicting colors.
   *
   * \sa https://www.w3schools.com/cssref/css_colors.asp
   */
  void SetColor(const std::string& label, const std::string& colorName);

  /**
   * \brief Set the line style of a data series.
   *
   * \param[in] label The label identifier of the data series.
   * \param[in] style The line style (solid or dashed).
   *
   * \note If an unknown label is given, nothing happens.
   * \warning Only takes visual effect when the chart type is ChartType::line.
   *          The style is retained if the chart type changes later.
   */
  void SetLineStyle(const std::string& label, LineStyle style);

  /**
   * \brief Set the marker symbol for a data series.
   *
   * \param[in] label The label identifier of the data series.
   * \param[in] symbol The marker symbol to use.
   *
   * \note If an unknown label is given, nothing happens.
   */
  void SetMarkerSymbol(const std::string &label, MarkerSymbol symbol);

  /**
   * \brief Set the Y axis scale type.
   *
   * \param[in] scale The axis scale (linear or logarithmic).
   *
   * \sa https://plot.ly/javascript/log-plot/
   */
  void SetYAxisScale(AxisScale scale);

  /**
   * \brief Set the X axis label text.
   * \param[in] label The label text.
   */
  void SetXAxisLabel(const std::string& label);

  /**
   * \brief Set the Y axis label text.
   * \param[in] label The label text.
   */
  void SetYAxisLabel(const std::string& label);

  /**
   * \brief Set category labels for pie chart segments.
   *
   * \param[in] pieLabels The category labels, one per data element.
   * \param[in] label The label of the pie chart data series.
   *
   * \note The series label (from AddData1D) still serves as the unique identifier.
   */
  void SetPieLabels(const std::vector<std::string> &pieLabels, const std::string &label);

  /**
   * \brief Set the chart title.
   * \param[in] title The title text displayed above the chart.
   */
  void SetTitle(const std::string &title);

  /**
   * \brief Change the chart type of an existing data series.
   *
   * \param[in] label The label identifier of the data series.
   * \param[in] type The new chart type.
   *
   * \note If an unknown label is given, nothing happens.
   * \warning Pie charts differ significantly from other types.
   *
   * \sa ChartType
   */
  void SetChartType(const std::string& label, ChartType type);

  /**
   * \brief Set error bars in the X direction for a data series.
   *
   * \param[in] label The label identifier of the data series.
   * \param[in] errorPlus The error values in the positive X direction.
   * \param[in] errorMinus The error values in the negative X direction.
   *                       If empty, symmetric error bars (same as errorPlus) are used.
   */
  void SetXErrorBars(const std::string &label, const std::vector<double> &errorPlus, const std::vector<double>& errorMinus = std::vector<double>());

  /**
   * \brief Set error bars in the Y direction for a data series.
   *
   * \param[in] label The label identifier of the data series.
   * \param[in] errorPlus The error values in the positive Y direction.
   * \param[in] errorMinus The error values in the negative Y direction.
   *                       If empty, symmetric error bars (same as errorPlus) are used.
   *
   * \sa SetXErrorBars
   */
  void SetYErrorBars(const std::string &label, const std::vector<double> &errorPlus, const std::vector<double> &errorMinus = std::vector<double>());

  /**
   * \brief Set the legend position.
   *
   * \param[in] position The desired legend position.
   *
   * \sa LegendPosition
   */
  void SetLegendPosition(LegendPosition position);

  /**
   * \brief Set whether the chart legend is visible.
   * \param[in] show true to show the legend, false to hide it.
   */
  void SetShowLegend(bool show);

  /**
   * \brief Set whether data series are displayed in stacked mode.
   * \param[in] stacked true to stack data series.
   */
  void SetStackedData(bool stacked);

  /**
   * \brief Render and display the chart in the widget.
   *
   * \param[in] showSubChart Whether to show a subchart (overview navigator) below the main chart (default: false).
   *
   * \note If no data has been added, an empty chart is displayed.
   *
   * \sa AddData1D
   * \sa AddData2D
   */
  void Show(bool showSubChart=false);

  /**
   * \brief Set whether data point markers are visible.
   *
   * \param[in] showDataPoints true to display data point markers.
   *
   * \sa https://plot.ly/javascript/line-charts/#styling-line-plot
   * \sa https://plot.ly/javascript/pointcloud/
   */
  void SetShowDataPoints(bool showDataPoints);

  /**
   * \brief Clear all data series and reset the widget.
   */
  void Clear();

  /**
   * \brief Set the chart color theme.
   *
   * \param[in] themeEnabled The color theme to apply.
   *
   * \warning Must be called before Show() or Reload() to take effect.
   */
  void SetTheme(ColorTheme themeEnabled);

  /**
   * \brief Set whether the subchart (overview navigator) is shown.
   *
   * \param[in] showSubChart true to show the subchart.
   *
   * \note Requires Reload() to display the change.
   */
  void SetShowSubchart(bool showSubChart);

  /**
   * \brief Set whether error bars are shown.
   *
   * \param[in] showErrorBars true to display error bars.
   *
   * \note Requires Reload() to display the change.
   */
  void SetShowErrorBars(bool showErrorBars);

  /**
   * \brief Set the visible X axis range (zoom).
   *
   * \param[in] minValueX The minimum X value to display.
   * \param[in] maxValueX The maximum X value to display.
   */
  void SetMinMaxValueXView(double minValueX,double maxValueX);

  /**
   * \brief Set the visible Y axis range (zoom).
   *
   * \param[in] minValueY The minimum Y value to display.
   * \param[in] maxValueY The maximum Y value to display.
   */
  void SetMinMaxValueYView(double minValueY, double maxValueY);

  /**
   * \brief Reload the chart to reflect property or data changes.
   *
   * Call this after modifying chart properties or adding data to
   * an already-displayed chart.
   */
  void Reload();

  /**
   * \brief Return the recommended size for the widget.
   * \return The size hint as a QSize.
   */
  QSize sizeHint() const override;

  /**
   * \brief Save the current chart as an SVG image file.
   *
   * Triggers the JavaScript side to generate the SVG, which is then
   * saved via a file dialog.
   */
  void SavePlotAsImage();

public slots:
  /**
   * \brief Slot called when the web page finishes loading.
   * \param[in] isLoadSuccessful true if the page loaded successfully.
   */
  void OnLoadFinished(bool isLoadSuccessful);

  /**
   * \brief Slot called when the chart page has been successfully loaded and is ready.
   */
  void OnPageSuccessfullyLoaded();

signals:
  /**
   * \brief Emitted when the chart page has been successfully loaded and is ready for use.
   */
  void PageSuccessfullyLoaded();

private:
  /*! source: https://stackoverflow.com/questions/29383/converting-bool-to-text-in-c*/
  std::string convertBooleanValue(bool value) const;

  class Impl;
  std::unique_ptr<Impl> m_Impl;
};

#endif
