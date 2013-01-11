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
  // set histogram to barchart in first instance
  m_UseLineGraph = false;

  // prepare html for use
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

// method to expose data to JavaScript by using properties
void QmitkHistogramJSWidget::ComputeHistogram(HistogramType* histogram)
{
  m_Histogram = histogram;
  HistogramConstIteratorType startIt = m_Histogram->End();
  HistogramConstIteratorType endIt = m_Histogram->End();
  HistogramConstIteratorType it;
  clearData();
  unsigned int i = 0;
  bool firstValue = false;
  for (it = m_Histogram->Begin() ; it != m_Histogram->End(); ++it, ++i)
  {
    if (i<1) MITK_INFO << "Measurement: " << it.GetMeasurementVector()[0] << " Frequency: " <<  it.GetFrequency();
    // filter frequencies of 0 to guarantee a better view while using a mask
    if (it.GetFrequency() != 0 && !firstValue)
    {
      firstValue = true;
      i=0;
    }
    if (firstValue)
    {
      QVariant frequency = it.GetFrequency();
      QVariant measurement = it.GetMeasurementVector()[0];
      m_Frequency.insert(i, frequency);
      m_Measurement.insert(i, measurement);
    }
  }

  this->DataChanged();
}

void QmitkHistogramJSWidget::clearData()
{
  m_Frequency.clear();
  m_Measurement.clear();
}

void QmitkHistogramJSWidget::clearHistogram()
{
  this->clearData();
  this->DataChanged();
}

QList<QVariant> QmitkHistogramJSWidget::getFrequency()
{
  return m_Frequency;
}

QList<QVariant> QmitkHistogramJSWidget::getMeasurement()
{
  return m_Measurement;
}

bool QmitkHistogramJSWidget::getUseLineGraph()
{
  return m_UseLineGraph;
}

// slots for radio- and pushbuttons
void QmitkHistogramJSWidget::histogramToBarChart()
{
  m_UseLineGraph = false;
  this->DataChanged();
}

void QmitkHistogramJSWidget::histogramToLineGraph()
{
  m_UseLineGraph = true;
  this->DataChanged();
}

void QmitkHistogramJSWidget::resetView()
{
  this->reload();
}
