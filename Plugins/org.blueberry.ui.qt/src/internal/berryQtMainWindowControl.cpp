/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtMainWindowControl.h"

#include <berryShell.h>

#include <QEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QWindowStateChangeEvent>

namespace berry {

QtMainWindowControl::QtMainWindowControl(Shell* shell, QWidget* parent, Qt::WindowFlags flags)
 : QMainWindow(parent, flags)
{
  controller = new QtWidgetController(shell);

  // TODO WeakPointer: QVariant should hold a weak pointer
  QVariant variant(QVariant::UserType);
  variant.setValue(controller);
  this->setProperty(QtWidgetController::PROPERTY_ID, variant);
}

void QtMainWindowControl::changeEvent(QEvent* event)
{
  if (!controller->shell)
    return QMainWindow::changeEvent(event);

  typedef IShellListener::Events::ShellEventType::ListenerList ListenerList;
  ShellEvent::Pointer shellEvent(new ShellEvent(Shell::Pointer(controller->shell)));
  switch (event->type())
  {
  case QEvent::ActivationChange:
  {
    if (isActiveWindow())
    {
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
    else
    {
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
  }
    break;
  case QEvent::WindowStateChange:
  {
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

  QMainWindow::changeEvent(event);
}

void QtMainWindowControl::closeEvent(QCloseEvent* event)
{
  if (!controller->shell)
    return QMainWindow::changeEvent(event);

  typedef IShellListener::Events::ShellEventType::ListenerList ListenerList;

  ShellEvent::Pointer shellEvent(new ShellEvent(Shell::Pointer(controller->shell)));

  ListenerList closedListeners(controller->shellEvents.shellClosed.GetListeners());
  for (auto listener = closedListeners.begin();
       listener != closedListeners.end(); ++listener)
  {
    (*listener)->Execute(shellEvent);
    if (!shellEvent->doit) {
      event->ignore();
      return;
    }
  }

  QMainWindow::closeEvent(event);
}

void QtMainWindowControl::moveEvent(QMoveEvent* event)
{
  GuiTk::ControlEvent::Pointer controlEvent(new GuiTk::ControlEvent(this, event->pos().x(), event->pos().y()));
  controller->controlEvents.movedEvent(controlEvent);
}

void QtMainWindowControl::resizeEvent(QResizeEvent* event)
{
  GuiTk::ControlEvent::Pointer controlEvent(new GuiTk::ControlEvent(this, 0, 0, event->size().width(), event->size().height()));
  controller->controlEvents.resizedEvent(controlEvent);
}

void QtMainWindowControl::inFocusEvent(QFocusEvent* /*event*/)
{
  GuiTk::ControlEvent::Pointer controlEvent(new GuiTk::ControlEvent(this));
  controller->controlEvents.activatedEvent(controlEvent);
}

}
