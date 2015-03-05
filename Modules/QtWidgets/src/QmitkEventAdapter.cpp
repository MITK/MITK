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

#include "QmitkEventAdapter.h"
#include <mitkInteractionConst.h>
#include <mitkWheelEvent.h>

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
    state, mitk::Key_none, p, wheelEvent->delta());

  return mitkEvent;
}


mitk::KeyEvent
QmitkEventAdapter::AdaptKeyEvent(mitk::BaseRenderer* sender, QKeyEvent* keyEvent, const QPoint& cp)
{
  int key = keyEvent->key();

  // Those keycodes changed in Qt 4
  if (key >= 0x01000000 && key <= 0x01000060)
    key -= (0x01000000 - 0x1000);
  else if(key >= 0x01001120 && key <= 0x01001262)
    key -= 0x01000000;

  mitk::Point2D p;
  p[0] = cp.x();
  p[1] = cp.y();

  int modifiers = keyEvent->modifiers();
  int state = 0;
  if (modifiers & Qt::ShiftModifier)
    state |= mitk::BS_ShiftButton;
  if (modifiers & Qt::ControlModifier)
    state |= mitk::BS_ControlButton;
  if (modifiers & Qt::AltModifier)
    state |= mitk::BS_AltButton;
  if (modifiers & Qt::MetaModifier)
    state |= mitk::BS_MetaButton;
  //if (modifiers & Qt::KeypadModifier)
  //  state |= mitk::BS_Keypad;

  mitk::KeyEvent mke(sender, keyEvent->type(), mitk::BS_NoButton, state, key, keyEvent->text().toStdString(), p);

  return mke;
}
