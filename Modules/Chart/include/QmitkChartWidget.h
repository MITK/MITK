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
#include <QWidget>

#include <MitkChartExports.h>


/*!
\brief QmitkChartWidget is a widget to display various charts based on the javascript chart library C3js. Currently, bar charts, line charts and pie charts are supported.
* \details Data is added via AddData1D() or AddData2D().\n
* There can be multiple charts (of the same type) created by calling AddData1D or AddData2D multiple times.\n\n
* The following chart types are supported:
* * line chart: http://c3js.org/samples/simple_multiple.html
* * bar chart: http://c3js.org/samples/chart_bar.html
* * spline chart: http://c3js.org/samples/chart_spline.html
* * pie chart: http://c3js.org/samples/chart_pie.html
* \n Technical details: The javascript code is embedded in a QWebEngineView. The actual js code is implemented in resource\Chart.js.
* \sa http://c3js.org for further information about the used javaScript library.
* \warning Pie is significantly different than the other types. Here, the data given by AddData1D is summed. Each entry represents a different category.
* \ingroup Modules/Chart
*/
class MITKCHART_EXPORT QmitkChartWidget : public QWidget
{
  Q_OBJECT

public:
  /*!
  * \brief enum of diagram types. Supported are bar, line, spline (a smoothed line) and pie. 
  */
	enum class ChartType { 
    bar, /*!< bar chart, see http://c3js.org/samples/chart_bar.html */
    line, /*!< line chart, see http://c3js.org/samples/simple_multiple.html */
    spline, /*!< spline chart (smoothed line chart), see http://c3js.org/samples/chart_spline.html */
    pie /*!< pie chart, see http://c3js.org/samples/chart_pie.html*/
  };
  enum class ChartStyle {
    defaultstyle,
    darkstyle
  };

  explicit QmitkChartWidget(ChartType type=ChartType::bar, QWidget* parent = nullptr);
  virtual ~QmitkChartWidget();

  /*!
  * \brief Adds 1D data to the widget
  * \details internally, the list is converted to a map with increasing integers keys starting at 0.
  * \note the data can be cleared with ClearDiagram()
  */
  void AddData1D(const std::vector<double>& data1D, const std::string& label);

  /*!
  * \brief Adds 2D data to the widget. Call repeatedly for displaying multiple charts.
  * \details each entry represents a data point: key: value --> x-value: y-value.
  * \note the data can be cleared with ClearDiagram() 
  */
  void AddData2D(const std::map<double, double>& data2D, const std::string& label);

  std::vector<std::string> GetDataLabels() const;

  void SetXAxisLabel(const std::string& label);
  std::string GetXAxisLabel() const;

  void SetYAxisLabel(const std::string& label);
  std::string GetYAxisLabel() const;

  /*!
  * \brief sets the diagram type
  *  \note to also display the changes, call ChangeDiagramTypeAndReload()
  *  \sa DiagramType for available types
  */
  void SetChartType(ChartType type);
  ChartType GetChartType() const;

  /*!
  * \brief Reloads the chart and changes the chart type
  */
  void SetChartTypeAndReload(ChartType type);

  /*!
  * \brief Displays the diagram in the widget
  * \param showSubChart if a subchart is displayed inside the widget or not. 
  * \details see http://c3js.org/samples/options_subchart.html
  */
  void Show(bool showSubChart=false);

  /*!
  * \brief Clears all data inside and resets the widget.
  */
  void Clear();

  /*!
  * \brief Changes the theme of the widget.
  */
  void SetTheme(ChartStyle themeEnabled);

  /*!
  * \brief Reloads the chart in the widget
  * \details reloading may be needed to display added data in an existing chart
  * \param showSubChart if a subchart is displayed inside the widget or not.
  */
  void Reload(bool showSubChart);

private:
  class Impl;
  Impl* m_Impl;

public slots:
  void OnLoadFinished(bool isLoadSuccessfull);

signals:
  void PageSuccessfullyLoaded();
};

#endif
