/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkEventAdapter.h"
#include <mitkInteractionConst.h>

#include <QPoint>
#include <QCursor>

mitk::MouseEvent 
QmitkEventAdapter::AdaptMouseEvent(mitk::BaseRenderer* sender, QMouseEvent* mouseEvent)
{
  mitk::Point2D p;
  p[0] = mouseEvent->x(); 
  p[1] = mouseEvent->y();
  
  int modifiers = mouseEvent->modifiers();
  int state = 0;
  
  switch (mouseEvent->type())
  {
  case QEvent::MouseButtonRelease:
    state |= mouseEvent->button();
    break;
  case QEvent::MouseMove:
    state |= mouseEvent->buttons();
    break;
  default:
    break;
  }
  
  if (modifiers & Qt::ShiftModifier)
    state |= mitk::BS_ShiftButton;
  if (modifiers & Qt::ControlModifier)
    state |= mitk::BS_ControlButton;
  if (modifiers & Qt::AltModifier)
    state |= mitk::BS_AltButton;
  if (modifiers & Qt::MetaModifier)
    state |= mitk::BS_MetaButton;
  if (modifiers & Qt::KeypadModifier)
    state |= mitk::BS_Keypad;
  
  mitk::MouseEvent mitkEvent(sender, mouseEvent->type(), mouseEvent->button(), 
                             state, mitk::Key_none, p);
  
  return mitkEvent;
}

mitk::WheelEvent 
QmitkEventAdapter::AdaptWheelEvent(mitk::BaseRenderer* sender, QWheelEvent* wheelEvent)
{
  mitk::Point2D p;
  p[0] = wheelEvent->x(); 
  p[1] = wheelEvent->y();

  int modifiers = wheelEvent->modifiers();
  int state = 0;

  state = wheelEvent->buttons();

  if (modifiers & Qt::ShiftModifier)
    state |= mitk::BS_ShiftButton;
  if (modifiers & Qt::ControlModifier)
    state |= mitk::BS_ControlButton;
  if (modifiers & Qt::AltModifier)
    state |= mitk::BS_AltButton;
  if (modifiers & Qt::MetaModifier)
    state |= mitk::BS_MetaButton;
  if (modifiers & Qt::KeypadModifier)
    state |= mitk::BS_Keypad;

  mitk::WheelEvent mitkEvent(sender, wheelEvent->type(), wheelEvent->buttons(), 
    state, mitk::Key_none, p);

  return mitkEvent;
}


mitk::KeyEvent 
QmitkEventAdapter::AdaptKeyEvent(QKeyEvent* keyEvent, const QPoint& cp)
{
  int key = keyEvent->key();
  
  // Those keycodes changed in Qt 4
  if (key >= 0x01000000 && key <= 0x01000060)
    key -= (0x01000000 - 0x1000);
  else if(key >= 0x01001120 && key <= 0x01001262)
    key -= 0x01000000;
  
  mitk::KeyEvent mke(keyEvent->type(), key, keyEvent->modifiers(),
      keyEvent->text().toStdString(), keyEvent->isAutoRepeat(),
      keyEvent->count(), cp.x(), cp.y(), QCursor::pos().x(), QCursor::pos().y());
  
  return mke;
}
