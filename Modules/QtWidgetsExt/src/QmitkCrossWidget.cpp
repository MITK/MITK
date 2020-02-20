/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCrossWidget.h"

#include <QCursor>
#include <QMouseEvent>

QmitkCrossWidget::QmitkCrossWidget(QWidget *parent, Qt::WindowFlags f) : QLabel(parent, f)
{
  setEnabled(true);
  setVisible(true);
  setFocusPolicy(Qt::ClickFocus);
}

void QmitkCrossWidget::mousePressEvent(QMouseEvent *)
{
  QPoint p = QCursor::pos();

  lastX = p.x();
  lastY = p.y();

  emit SignalDeltaMove(0, 0);
}

void QmitkCrossWidget::mouseMoveEvent(QMouseEvent *)
{
  QPoint p = QCursor::pos();

  int newX = p.x();
  int newY = p.y();

  int deltaX = newX - lastX;
  int deltaY = newY - lastY;

  this->ResetMousePosition(lastX, lastY);

  emit SignalDeltaMove(deltaX, deltaY);
}

void QmitkCrossWidget::mouseReleaseEvent(QMouseEvent *)
{
}
