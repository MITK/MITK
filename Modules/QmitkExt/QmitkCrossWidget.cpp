/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-05-12 19:14:45 +0200 (Tue, 12 May 2009) $
 Version:   $Revision: 17169 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

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
