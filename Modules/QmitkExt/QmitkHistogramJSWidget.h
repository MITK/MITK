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
#include "mitkPlanarFigure.h"
#include <itkPolyLineParametricPath.h>



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
  typedef itk::PolyLineParametricPath< 3 > ParametricPathType;
  typedef itk::ParametricPath< 3 >::Superclass PathType;

  explicit QmitkHistogramJSWidget(QWidget *parent = 0);
  ~QmitkHistogramJSWidget();
  void resizeEvent(QResizeEvent* resizeEvent);
  void ComputeHistogram(HistogramType* histogram);
  void clearHistogram();
  QList<QVariant> getMeasurement();
  QList<QVariant> getFrequency();
  bool getUseLineGraph();
  void setImage(mitk::Image* image);
  void setPlanarFigure(const mitk::PlanarFigure* planarFigure);
  void ComputeHistogramOfPlanarFigure();

private:
  QList<QVariant> m_Frequency;
  QList<QVariant> m_Measurement;
  mitk::Image::Pointer m_Image;
  mitk::PlanarFigure::ConstPointer m_PlanarFigure;
  bool m_UseLineGraph;
  HistogramType::ConstPointer m_Histogram;
  PathType::ConstPointer m_DerivedPath;
  ParametricPathType::Pointer m_ParametricPath;

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
