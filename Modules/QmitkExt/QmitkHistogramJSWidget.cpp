#include "QmitkHistogramJSWidget.h"

QmitkHistogramJSWidget::QmitkHistogramJSWidget(QWidget *parent) :
  QWebView(parent)
{
  for (int i = 0; i<50; i++)
  {
    int blubb = i*i*i;
    Dataset.insert(i,QVariant(blubb));
  }

  for ( int j = -25; j<50; j++)
  {
    m_Dataset.insert(j, j*j*j*j);
  }

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
  emit sendData(Dataset);
}

// reloads WebView, everytime its size has been changed, so the size of the Histogram fits to the size of the widget
void QmitkHistogramJSWidget::resizeEvent(QResizeEvent* resizeEvent)
{
  QWebView::resizeEvent(resizeEvent);
  this->reload();
}