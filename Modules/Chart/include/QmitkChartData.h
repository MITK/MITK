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

#ifndef QmitkC3Data_h
#define QmitkC3Data_h

#include <QmitkChartWidget.h>
#include <QVariant>

/** \brief This class holds the relevant properties for the chart generation with C3 such as labels and diagram type.
* It is derived from QObject, because we need Q_PROPERTIES to send Data via QWebChannel to JavaScript.
* \sa The actual data for the chart generation is in QmitkC3xyData!
*/
class QmitkChartData : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QVariant m_xAxisLabel READ GetXAxisLabel WRITE SetXAxisLabel NOTIFY SignalXAxisLabelChanged);
  Q_PROPERTY(QVariant m_yAxisLabel READ GetYAxisLabel WRITE SetYAxisLabel NOTIFY SignalYAxisLabelChanged);
  Q_PROPERTY(QVariant m_chartTitle READ GetTitle WRITE SetTitle NOTIFY SignalTitleChanged);
  Q_PROPERTY(QVariant m_LegendPosition READ GetLegendPosition WRITE SetLegendPosition NOTIFY SignalLegendPositionChanged);
  Q_PROPERTY(QVariant m_ShowLegend READ GetShowLegend WRITE SetShowLegend NOTIFY SignalShowLegendChanged);
  Q_PROPERTY(QVariant m_YAxisScale READ GetYAxisScale WRITE SetYAxisScale NOTIFY SignalYAxisScaleChanged);
  Q_PROPERTY(QVariant m_ShowSubchart READ GetShowSubchart WRITE SetShowSubchart NOTIFY SignalShowSubchartChanged);
  Q_PROPERTY(QVariant m_UsePercentageInPieChart READ GetUsePercentageInPieChart WRITE SetUsePercentageInPieChart NOTIFY SignalUsePercentageInPieChartChanged);
  Q_PROPERTY(QVariant m_DataPointSize READ GetDataPointSize WRITE SetDataPointSize NOTIFY SignalDataPointSizeChanged);

public:
  QmitkChartData();

  void SetAppearance(bool showSubChart = true, bool usePercentageInPieChart = false);

  Q_INVOKABLE QVariant GetXAxisLabel() const { return m_xAxisLabel; };
  Q_INVOKABLE void SetXAxisLabel(const QVariant& label) { m_xAxisLabel = label; emit SignalXAxisLabelChanged(label); };

  Q_INVOKABLE QVariant GetYAxisLabel() const { return m_yAxisLabel; };
  Q_INVOKABLE void SetYAxisLabel(const QVariant& label) { m_yAxisLabel = label; emit SignalYAxisLabelChanged(label); };

  Q_INVOKABLE QVariant GetTitle() const { return m_chartTitle; };
  Q_INVOKABLE void SetTitle(const QVariant& title) { m_chartTitle = title; emit SignalTitleChanged(title); };

  Q_INVOKABLE QVariant GetLegendPosition() const { return m_LegendPosition; };
  Q_INVOKABLE void SetLegendPosition(const QVariant& legendPosition) { m_LegendPosition = legendPosition; emit SignalLegendPositionChanged(legendPosition); };

  Q_INVOKABLE QVariant GetShowLegend() const { return m_ShowLegend; };
  Q_INVOKABLE void SetShowLegend(const QVariant& show) { m_ShowLegend = show; emit SignalShowLegendChanged(show); };

  Q_INVOKABLE QVariant GetYAxisScale() const { return m_YAxisScale; };
  Q_INVOKABLE void SetYAxisScale(const QVariant& YAxisScale) { m_YAxisScale = YAxisScale; emit SignalYAxisScaleChanged(YAxisScale); };

  Q_INVOKABLE QVariant GetShowSubchart() const { return m_ShowSubchart; };
  Q_INVOKABLE void SetShowSubchart(const QVariant& showSubchart) { m_ShowSubchart = showSubchart; emit SignalShowSubchartChanged(showSubchart); };

  Q_INVOKABLE QVariant GetUsePercentageInPieChart() const { return m_UsePercentageInPieChart; };
  Q_INVOKABLE void SetUsePercentageInPieChart(const QVariant& usePercentageInPieChart) { m_UsePercentageInPieChart = usePercentageInPieChart; emit SignalUsePercentageInPieChartChanged(usePercentageInPieChart); };

  Q_INVOKABLE QVariant GetDataPointSize() const { return m_DataPointSize; };
  Q_INVOKABLE void SetDataPointSize(const QVariant& showDataPoints) {if (showDataPoints > 0 ) { m_DataPointSize = 3; } else { m_DataPointSize = 0; } emit SignalDataPointSizeChanged(showDataPoints); };

signals:
  void SignalYAxisLabelChanged(const QVariant label);
  void SignalXAxisLabelChanged(const QVariant label);
  void SignalLegendPositionChanged(const QVariant legendPosition);
  void SignalShowLegendChanged(const QVariant show);
  void SignalYAxisScaleChanged(const QVariant YAxisScale);
  void SignalTitleChanged(const QVariant title);
  void SignalShowSubchartChanged(const QVariant showSubchart);
  void SignalUsePercentageInPieChartChanged(const QVariant usePercentageInPieChart);
  void SignalDataPointSizeChanged(const QVariant showDataPoints);

private:
  QVariant m_xAxisLabel;
  QVariant m_yAxisLabel;
  QVariant m_chartTitle;

  QVariant m_ShowLegend = true;
  QVariant m_LegendPosition;
  QVariant m_ShowSubchart;
  QVariant m_YAxisScale;
  QVariant m_UsePercentageInPieChart;
  QVariant m_numberDatasets;
  QVariant m_DataPointSize = 0;
};

#endif  //QmitkC3Data_h
