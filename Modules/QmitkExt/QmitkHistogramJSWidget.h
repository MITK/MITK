#ifndef QMITKHISTOGRAMJSWIDGET_H
#define QMITKHISTOGRAMJSWIDGET_H

#include <QWidget>
#include <QUrl>
#include <QtWebKit/QtWebKit>
#include "QmitkExtExports.h"
#include <QVariant>
#include "itkHistogram.h"

class QmitkExt_EXPORT QmitkHistogramJSWidget : public QWebView
{
  Q_OBJECT

private:
  QList<QVariant> Dataset;
  QMap<int, QVariant> m_Dataset;

public:
  explicit QmitkHistogramJSWidget(QWidget *parent = 0);
  ~QmitkHistogramJSWidget();
  void resizeEvent(QResizeEvent* resizeEvent);

private slots:
  void addJSObject();

signals:
  void sendData(QList<QVariant> data);

public slots:
  void emitData();
};

#endif // QMITKHISTOGRAMJSWIDGET_H
