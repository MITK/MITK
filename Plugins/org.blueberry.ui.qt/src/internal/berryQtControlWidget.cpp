/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryLog.h"

#include "berryQtControlWidget.h"

#include <berryShell.h>

#include <QMoveEvent>
#include <QResizeEvent>

#include <algorithm>

namespace berry {

QtControlWidget::QtControlWidget(QWidget* parent, Shell* shell, Qt::WindowFlags f)
 : QFrame(parent, f)
{
  controller = new QtWidgetController(shell);

  this->setFrameStyle(QFrame::NoFrame);

  // TODO WeakPointer: QVariant should hold a weak pointer
  auto variant = QVariant::fromValue(controller);
  this->setProperty(QtWidgetController::PROPERTY_ID, variant);
}

QtControlWidget::~QtControlWidget()
{
  GuiTk::ControlEvent::Pointer controlEvent(new GuiTk::ControlEvent(static_cast<QWidget*>(this)));
  controller->controlEvents.destroyedEvent(controlEvent);
}

void QtControlWidget::changeEvent(QEvent* event)
{
  typedef IShellListener::Events::ShellEventType::ListenerList ListenerList;

  switch (event->type())
  {
  case QEvent::WindowActivate:
  {
      ShellEvent::Pointer shellEvent(new ShellEvent(Shell::Pointer(controller->shell)));
    ListenerList activatedListeners(controller->shellEvents.shellActivated.GetListeners());
    for (auto listener = activatedListeners.begin();
         listener != activatedListeners.end(); ++listener)
    {
      (*listener)->Execute(shellEvent);
      if (!shellEvent->doit) {
        event->accept();
        return;
      }
    }
  }
    break;
  case QEvent::WindowDeactivate:
  {
      ShellEvent::Pointer shellEvent(new ShellEvent(Shell::Pointer(controller->shell)));
    ListenerList deactivatedListeners(controller->shellEvents.shellDeactivated.GetListeners());
    for (auto listener = deactivatedListeners.begin();
         listener != deactivatedListeners.end(); ++listener)
    {
      (*listener)->Execute(shellEvent);
      if (!shellEvent->doit) {
        event->accept();
        return;
      }
    }
  }
    break;
  case QEvent::WindowStateChange:
  {
      ShellEvent::Pointer shellEvent(new ShellEvent(Shell::Pointer(controller->shell)));
    QWindowStateChangeEvent* stateEvent = dynamic_cast<QWindowStateChangeEvent*>(event);
    Qt::WindowStates oldState = stateEvent->oldState();
    if (this->isMinimized() && !(oldState & Qt::WindowMinimized))
    {
      ListenerList iconifiedListeners(controller->shellEvents.shellIconified.GetListeners());
      for (auto listener = iconifiedListeners.begin();
         listener != iconifiedListeners.end(); ++listener)
      {
        (*listener)->Execute(shellEvent);
        if (!shellEvent->doit) {
          event->accept();
          return;
        }
      }
    }
    else if (oldState & Qt::WindowMinimized && !this->isMinimized())
    {
      ListenerList deiconifiedListeners(controller->shellEvents.shellDeiconified.GetListeners());
      for (auto listener = deiconifiedListeners.begin();
         listener != deiconifiedListeners.end(); ++listener)
      {
        (*listener)->Execute(shellEvent);
        if (!shellEvent->doit) return;
      }
    }
  }
    break;
  default:
    break;
  }

  QFrame::changeEvent(event);
}

void QtControlWidget::closeEvent(QCloseEvent* event)
{
  typedef IShellListener::Events::ShellEventType::ListenerList ListenerList;

  ShellEvent::Pointer shellEvent(new ShellEvent(Shell::Pointer(controller->shell)));

  ListenerList closedListeners(controller->shellEvents.shellClosed.GetListeners());
  for (auto listener = closedListeners.begin();
       listener != closedListeners.end(); ++listener)
  {
    (*listener)->Execute(shellEvent);
    if (!shellEvent->doit) {
      //event->accept();
      return;
    }
  }

  event->accept();
}

void QtControlWidget::moveEvent(QMoveEvent* event)
{
  GuiTk::ControlEvent::Pointer controlEvent(new GuiTk::ControlEvent(static_cast<QWidget*>(this), event->pos().x(), event->pos().y()));
  controller->controlEvents.movedEvent(controlEvent);
}

void QtControlWidget::resizeEvent(QResizeEvent* event)
{
  GuiTk::ControlEvent::Pointer controlEvent(new GuiTk::ControlEvent(static_cast<QWidget*>(this), 0, 0, event->size().width(), event->size().height()));
  controller->controlEvents.resizedEvent(controlEvent);
}

void QtControlWidget::FireActivateEvent()
{
  GuiTk::ControlEvent::Pointer controlEvent(new GuiTk::ControlEvent(static_cast<QWidget*>(this)));
  controller->controlEvents.activatedEvent(controlEvent);
}


}
