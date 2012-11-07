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


#include "QmitkHistogramJSWidget.h"

QmitkHistogramJSWidget::QmitkHistogramJSWidget(QWidget *parent) :
  QWebView(parent)
{

  connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJSObject()));
  QUrl myUrl = QUrl("qrc:/qmitk/Histogram.html");
  setUrl(myUrl);

  // set Scrollbars to always disabled
  page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
  page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
}

QmitkHistogramJSWidget::~QmitkHistogramJSWidget()
{

}

// adds an Object of Type QmitkHistogramJSWidget to the JavaScript
void QmitkHistogramJSWidget::addJSObject()
{
  page()->mainFrame()->addToJavaScriptWindowObject(QString("histogramData"), this);
}


// reloads WebView, everytime its size has been changed, so the size of the Histogram fits to the size of the widget
void QmitkHistogramJSWidget::resizeEvent(QResizeEvent* resizeEvent)
{
  QWebView::resizeEvent(resizeEvent);
  this->reload();
}

void QmitkHistogramJSWidget::ComputeHistogram(HistogramType* histogram)
{
  this->clearData();
  m_Histogram = histogram;
  HistogramConstIteratorType startIt = m_Histogram->End();
  HistogramConstIteratorType endIt = m_Histogram->End();
  HistogramConstIteratorType it;
  unsigned int i = 0;
  for (it = m_Histogram->Begin() ; it != m_Histogram->End(); ++it, ++i)
  {
    QVariant frequency = it.GetFrequency();
    QVariant measurement = it.GetMeasurementVector()[0];
    m_Frequency.insert(i, frequency);
    m_Measurement.insert(i, measurement);
  }

  this->reload();
}

void QmitkHistogramJSWidget::clearData()
{
  m_Frequency.clear();
  m_Measurement.clear();
}

void QmitkHistogramJSWidget::clearHistogram()
{
  this->clearData();
  this->reload();
}

QList<QVariant> QmitkHistogramJSWidget::getFrequency()
{
  return m_Frequency;
}

QList<QVariant> QmitkHistogramJSWidget::getMeasurement()
{
  return m_Measurement;
}
