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


#ifndef QMITKHISTOGRAMJSWIDGET_H
#define QMITKHISTOGRAMJSWIDGET_H

#include <QWidget>
#include <QUrl>
#include <QtWebKit/QtWebKit>
#include "QmitkExtExports.h"
#include <QVariant>
#include "mitkImage.h"



class QmitkExt_EXPORT QmitkHistogramJSWidget : public QWebView
{
  Q_OBJECT

// Properties which can be used in JavaScript
  Q_PROPERTY(QList<QVariant> measurement
             READ getMeasurement)
  Q_PROPERTY(QList<QVariant> frequency
             READ getFrequency)
  Q_PROPERTY(bool useLineGraph
             READ getUseLineGraph)

public:
  typedef mitk::Image::HistogramType HistogramType;
  typedef mitk::Image::HistogramType::ConstIterator HistogramConstIteratorType;

  explicit QmitkHistogramJSWidget(QWidget *parent = 0);
  ~QmitkHistogramJSWidget();
  void resizeEvent(QResizeEvent* resizeEvent);
  void ComputeHistogram(HistogramType* histogram);
  void clearHistogram();
  QList<QVariant> getMeasurement();
  QList<QVariant> getFrequency();
  bool getUseLineGraph();

private:
  QList<QVariant> m_Frequency;
  QList<QVariant> m_Measurement;
  bool m_UseLineGraph;
  HistogramType::ConstPointer m_Histogram;

  void clearData();

private slots:
  void addJSObject();

public slots:
  void histogramToBarChart();
  void histogramToLineGraph();
  void resetView();

signals:
  void DataChanged();
};

#endif // QMITKHISTOGRAMJSWIDGET_H
