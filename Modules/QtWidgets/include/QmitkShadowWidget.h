#pragma once

#include "MitkQtWidgetsExports.h"

#include <QWidget>

class MITKQTWIDGETS_EXPORT QmitkShadowWidget: public QWidget
{
  Q_OBJECT

public:
  explicit QmitkShadowWidget(QWidget* parent);

signals:
  void onResize(QResizeEvent* e);

protected:
  virtual void resizeEvent(QResizeEvent* e) override;
};