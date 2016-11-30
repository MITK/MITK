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

/** /brief This class holds the relevant data for the chart generation with C3.
* Documentation: This class holds the relevant data for the chart generation with C3. It forwards the frequencies and the measurements to JavaScript.
* Those are then displayed in the histogram chart.
* It is derived from QObject, because we need Q_PROPERTIES to send Data via QWebChannel to JavaScript.
*/
class QmitkC3Data : public QObject
{
  Q_OBJECT

  typedef mitk::Image::HistogramType HistogramType;
  Q_PROPERTY(QList<QVariant> m_YData READ GetYData WRITE SetYData NOTIFY SignalYDataChanged);
  Q_PROPERTY(QList<QVariant> m_XData READ GetXData WRITE SetXData NOTIFY SignalXDataChanged);
  Q_PROPERTY(QVariant m_UseLineChart READ GetUseLineChart WRITE SetUseLineChart NOTIFY SignalUseLineChartChanged);
  Q_PROPERTY(QVariant m_ShowSubchart READ GetShowSubchart WRITE SetShowSubchart NOTIFY SignalShowSubchartChanged);

public:
  QmitkC3Data();
  QmitkC3Data(HistogramType::ConstPointer histogram);

  void SetAppearance(bool useLineChart, bool showSubChart);
  Q_INVOKABLE QList<QVariant> GetYData() { return m_YData; };
  Q_INVOKABLE void SetYData(QList<QVariant> yData) { m_YData = yData; };

  Q_INVOKABLE QList<QVariant> GetXData() { return m_XData; };
  Q_INVOKABLE void SetXData(QList<QVariant> xData) { m_XData = xData; };

  Q_INVOKABLE QVariant GetUseLineChart() { return m_UseLineChart; };
  Q_INVOKABLE void SetUseLineChart(QVariant useLineChart) { m_UseLineChart = useLineChart; };

  Q_INVOKABLE QVariant GetShowSubchart() { return m_ShowSubchart; };
  Q_INVOKABLE void SetShowSubchart(QVariant showSubchart) { m_ShowSubchart = showSubchart; };
  /**
  * \brief Clears the Data.
  *
  * This function clears the data.
  */
  void ClearData();

  HistogramType::ConstPointer GetHistogram() { return m_Histogram; };
  void SetHistogram(HistogramType::ConstPointer histogram) { m_Histogram = histogram; };

  QList<QVariant>* GetYDataPointer() { return &m_YData; };

  QList<QVariant>* GetXDataPointer() { return &m_XData; };

signals:
  void SignalYDataChanged(const QList<QVariant> yData);
  void SignalXDataChanged(const QList<QVariant> xData);
  void SignalUseLineChartChanged(const QVariant useLineChart);
  void SignalShowSubchartChanged(const QVariant showSubchart);

private:
  /**
  * Holds the current histogram
  */
  HistogramType::ConstPointer m_Histogram;

  /**
  * \brief List of frequencies.
  *
  * A QList which holds the frequencies of the current histogram
  * or holds the intensities of current intensity profile.
  */
  QList<QVariant> m_YData;

  /**
  * \brief List of measurements.
  *
  * A QList which holds the measurements of the current histogram
  * or holds the distances of current intensity profile.
  */
  QList<QVariant> m_XData;

  QVariant m_UseLineChart;
  QVariant m_ShowSubchart;
};

#endif  //QmitkC3Data_h