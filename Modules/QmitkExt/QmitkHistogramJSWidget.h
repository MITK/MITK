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

public:
  typedef mitk::Image::HistogramType HistogramType;
  typedef mitk::Image::HistogramType::ConstIterator HistogramConstIteratorType;

  explicit QmitkHistogramJSWidget(QWidget *parent = 0);
  ~QmitkHistogramJSWidget();
  void resizeEvent(QResizeEvent* resizeEvent);
  void ComputeHistogram(HistogramType* histogram);
  void clearHistogram();

private:
  QList<QVariant> m_Frequency;
  QList<QVariant> m_Measurement;
  HistogramType::ConstPointer m_Histogram;

  void clearData();

private slots:
  void addJSObject();

signals:
  void sendFrequency(QList<QVariant> frequency);
  void sendMeasurement(QList<QVariant> measurement);

public slots:
  void emitData();
};

#endif // QMITKHISTOGRAMJSWIDGET_H
