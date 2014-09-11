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

#include "QmitkCrossWidget.h"

#include <QMouseEvent>
#include <QCursor>

QmitkCrossWidget::QmitkCrossWidget(QWidget * parent, Qt::WindowFlags f) :
  QLabel(parent, f)
{
  setEnabled(true);
  setVisible(true);
  setFocusPolicy(Qt::ClickFocus);
}

void QmitkCrossWidget::mousePressEvent(QMouseEvent * )
{
  QPoint p = QCursor::pos();

  lastX = p.x();
  lastY = p.y();

  emit SignalDeltaMove( 0 , 0 );
}

void QmitkCrossWidget::mouseMoveEvent(QMouseEvent * )
{
  QPoint p = QCursor::pos();

  int newX = p.x();
  int newY = p.y();

  int deltaX = newX-lastX;
  int deltaY = newY-lastY;

  this->ResetMousePosition( lastX,lastY);

  emit SignalDeltaMove( deltaX , deltaY );
}

void QmitkCrossWidget::mouseReleaseEvent(QMouseEvent*)
{
}

