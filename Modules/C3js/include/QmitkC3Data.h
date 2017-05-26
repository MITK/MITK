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

#include <QmitkC3jsWidget.h>
#include <QVariant>

/** \brief This class holds the relevant properties for the chart generation with C3 such as labels and diagram type.
* It is derived from QObject, because we need Q_PROPERTIES to send Data via QWebChannel to JavaScript.
* \sa The actual data for the chart generation is in QmitkC3xyData!
*/
class QmitkC3Data : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QList<QVariant> m_dataLabels READ GetDataLabels WRITE SetDataLabels NOTIFY SignalDataLabelsChanged);
  Q_PROPERTY(QVariant m_xAxisLabel READ GetXAxisLabel WRITE SetXAxisLabel NOTIFY SignalXAxisLabelChanged);
  Q_PROPERTY(QVariant m_yAxisLabel READ GetYAxisLabel WRITE SetYAxisLabel NOTIFY SignalYAxisLabelChanged);
  Q_PROPERTY(QVariant m_DiagramTypeName READ GetDiagramType WRITE SetDiagramType NOTIFY SignalDiagramTypeChanged);
  Q_PROPERTY(QVariant m_ShowSubchart READ GetShowSubchart WRITE SetShowSubchart NOTIFY SignalShowSubchartChanged);
  Q_PROPERTY(QVariant m_UsePercentageInPieChart READ GetUsePercentageInPieChart WRITE SetUsePercentageInPieChart NOTIFY SignalUsePercentageInPieChartChanged);

public:
  QmitkC3Data();

  void SetAppearance(QVariant diagramTypeName, bool showSubChart = true, bool usePercentageInPieChart = false);

  Q_INVOKABLE QList<QVariant> GetDataLabels() const { return m_dataLabels; };
  Q_INVOKABLE void SetDataLabels(QList<QVariant> dataLabels) { m_dataLabels = dataLabels; };

  Q_INVOKABLE QVariant GetXAxisLabel() { return m_xAxisLabel; };
  Q_INVOKABLE void SetXAxisLabel(QVariant label) { m_xAxisLabel = label; };

  Q_INVOKABLE QVariant GetYAxisLabel() { return m_yAxisLabel; };
  Q_INVOKABLE void SetYAxisLabel(QVariant label) { m_yAxisLabel = label; };

  Q_INVOKABLE QVariant GetDiagramType() { return m_DiagramTypeName; };
  Q_INVOKABLE void SetDiagramType(QVariant diagramTypeName) { m_DiagramTypeName = diagramTypeName; /*emit SignalDiagramTypeChanged(m_DiagramTypeName);*/ };

  Q_INVOKABLE QVariant GetShowSubchart() { return m_ShowSubchart; };
  Q_INVOKABLE void SetShowSubchart(QVariant showSubchart) { m_ShowSubchart = showSubchart; };

  Q_INVOKABLE QVariant GetUsePercentageInPieChart() { return m_UsePercentageInPieChart; };
  Q_INVOKABLE void SetUsePercentageInPieChart(QVariant usePercentageInPieChart) { m_UsePercentageInPieChart = usePercentageInPieChart; };

signals:
  void SignalDataLabelsChanged(const QList<QVariant> dataLabels);
  void SignalYAxisLabelChanged(const QVariant label);
  void SignalXAxisLabelChanged(const QVariant label);
  void SignalDiagramTypeChanged(const QVariant chartType);
  void SignalShowSubchartChanged(const QVariant showSubchart);
  void SignalUsePercentageInPieChartChanged(const QVariant usePercentageInPieChart);

private:

  QList<QVariant> m_dataLabels;
  QVariant m_xAxisLabel;
  QVariant m_yAxisLabel;
  QVariant m_diagramTitle;

  QVariant m_DiagramTypeName;
  QVariant m_ShowSubchart;
  QVariant m_UsePercentageInPieChart;
  QVariant m_numberDatasets;
};

#endif  //QmitkC3Data_h