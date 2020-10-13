#include <iostream>

#include "QmitkShadowWidget.h"

QmitkShadowWidget::QmitkShadowWidget(QWidget* parent)
  : QWidget(parent)
{
};

void QmitkShadowWidget::resizeEvent(QResizeEvent* e)
{
  emit onResize(e);
}