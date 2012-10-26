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

// emits Data to the JavaScript
void QmitkHistogramJSWidget::emitData()
{
  emit sendFrequency(m_Frequency);
  emit sendMeasurement(m_Measurement);
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
  for (it = m_Histogram->Begin() ; it != m_Histogram->End(); it++, i++)
  {
    double frequency = it.GetFrequency();
    double measurement = it.GetMeasurementVector()[0];
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
