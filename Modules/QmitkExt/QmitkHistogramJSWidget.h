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
  Q_PROPERTY(QList<QVariant> measurement
             READ getMeasurement)
  Q_PROPERTY(QList<QVariant> frequency
             READ getFrequency)

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

private:
  QList<QVariant> m_Frequency;
  QList<QVariant> m_Measurement;
  HistogramType::ConstPointer m_Histogram;

  void clearData();

private slots:
  void addJSObject();

signals:
  void DataChanged();
  void sizeChanged();
};

#endif // QMITKHISTOGRAMJSWIDGET_H
