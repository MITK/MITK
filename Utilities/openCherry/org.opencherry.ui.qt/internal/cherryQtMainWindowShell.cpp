/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY { } without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryQtMainWindowShell.h"

#include "cherryQtWidgetsTweaklet.h"

#include <cherryConstants.h>

#include <QEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QWindowStateChangeEvent>

namespace cherry
{

QtMainWindowShell::QtMainWindowShell(QWidget* parent, Qt::WindowFlags flags) :
  QMainWindow(parent, flags)
{

}

void QtMainWindowShell::SetBounds(const Rectangle& bounds)
{
  this->setGeometry(bounds.x, bounds.y, bounds.width, bounds.height);
}

Rectangle QtMainWindowShell::GetBounds() const
{
  const QRect& qRect = this->geometry();
  Rectangle rect(qRect.x(), qRect.y(), qRect.width(), qRect.height());
  return rect;
}

void QtMainWindowShell::SetLocation(int x, int y)
{
  this->move(x, y);
}

Point QtMainWindowShell::ComputeSize(int wHint, int hHint, bool changed)
{
  if (changed) this->updateGeometry();
  QSize size(this->size());
  Point point(size.width(), size.height());
  return point;
}

void QtMainWindowShell::SetText(const std::string& text)
{
  QString title(QString::fromStdString(text));
  this->setWindowTitle(title);
  this->setObjectName(title);
}

bool QtMainWindowShell::IsVisible()
{
  return this->isVisible();
}

void QtMainWindowShell::SetVisible(bool visible)
{
  this->setVisible(visible);
}

void* QtMainWindowShell::GetControl()
{
  return this;
}

void QtMainWindowShell::SetImages(const std::vector<void*>& images)
{
}

bool QtMainWindowShell::GetMaximized()
{
  return this->isMaximized();
}

bool QtMainWindowShell::GetMinimized()
{
  return this->isMinimized();
}

void QtMainWindowShell::SetMaximized(bool maximized)
{
  maximized ? this->showMaximized() : this->showNormal();
}

void QtMainWindowShell::SetMinimized(bool minimized)
{
  minimized ? this->showMinimized() : this->showNormal();
}

void QtMainWindowShell::AddShellListener(IShellListener::Pointer listener)
{
  if (std::find(shellListeners.begin(), shellListeners.end(), listener) != shellListeners.end())
    return;

  shellListeners.push_back(listener);
}

void QtMainWindowShell::RemoveShellListener(IShellListener::Pointer listener)
{
  shellListeners.remove(listener);
}

void QtMainWindowShell::Open(bool block)
{
  this->setWindowModality(block ? Qt::WindowModal : Qt::NonModal);
  this->show();
}

void QtMainWindowShell::Close()
{
  this->close();
}

std::vector<Shell::Pointer> QtMainWindowShell::GetShells()
{
  return std::vector<Shell::Pointer>(QtWidgetsTweaklet::shellList.begin(),
                                     QtWidgetsTweaklet::shellList.end());
}

int QtMainWindowShell::GetStyle()
{
  Qt::WindowFlags qtFlags = this->windowFlags();

  int cherryFlags = 0;
  if (!(qtFlags & Qt::FramelessWindowHint))
    cherryFlags |= Constants::BORDER;
  if (qtFlags & Qt::WindowTitleHint)
    cherryFlags |= Constants::TITLE;
  if (qtFlags & Qt::WindowSystemMenuHint)
    cherryFlags |= Constants::CLOSE;
  if (qtFlags & Qt::WindowMinimizeButtonHint)
    cherryFlags |= Constants::MIN;
  if (qtFlags & Qt::WindowMaximizeButtonHint)
    cherryFlags |= Constants::MAX;

  return cherryFlags;
}

void QtMainWindowShell::changeEvent(QEvent* event)
{
  Shell::Pointer shell = this;
  ShellEvent::Pointer shellEvent = new ShellEvent(shell);
  switch (event->type())
  {
  case QEvent::WindowActivate:
  {
    for (std::list<IShellListener::Pointer>::iterator listener = shellListeners.begin();
         listener != shellListeners.end(); ++listener)
    {
      (*listener)->ShellActivated(shellEvent);
      if (!shellEvent->doit) return;
    }
  }
    break;
  case QEvent::WindowDeactivate:
  {
    for (std::list<IShellListener::Pointer>::iterator listener = shellListeners.begin();
         listener != shellListeners.end(); ++listener)
    {
      (*listener)->ShellDeactivated(shellEvent);
      if (!shellEvent->doit) return;
    }
  }
    break;
  case QEvent::WindowStateChange:
  {
    QWindowStateChangeEvent* stateEvent = dynamic_cast<QWindowStateChangeEvent*>(event);
    Qt::WindowStates oldState = stateEvent->oldState();
    if (this->isMinimized() && !(oldState & Qt::WindowMinimized))
    {
      for (std::list<IShellListener::Pointer>::iterator listener = shellListeners.begin();
         listener != shellListeners.end(); ++listener)
      {
        (*listener)->ShellIconified(shellEvent);
        if (!shellEvent->doit) return;
      }
    }
    else if (oldState & Qt::WindowMinimized && !this->isMinimized())
    {
      for (std::list<IShellListener::Pointer>::iterator listener = shellListeners.begin();
         listener != shellListeners.end(); ++listener)
      {
        (*listener)->ShellDeiconified(shellEvent);
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

void QtMainWindowShell::closeEvent(QCloseEvent* event)
{
  Shell::Pointer shell = this;
  ShellEvent::Pointer shellEvent = new ShellEvent(shell);
  for (std::list<IShellListener::Pointer>::iterator listener = shellListeners.begin();
       listener != shellListeners.end(); ++listener)
  {
    (*listener)->ShellClosed(shellEvent);
    if (!shellEvent->doit) {
      event->accept();
      return;
    }
  }

  QMainWindow::closeEvent(event);
}

void QtMainWindowShell::AddControlListener(GuiTk::IControlListener::Pointer listener)
{
  ControlListenersList::iterator iter = std::find(controlListeners.begin(), controlListeners.end(), listener);
  if (iter != controlListeners.end()) return;

  controlListeners.push_back(listener);
}

void QtMainWindowShell::RemoveControlListener(GuiTk::IControlListener::Pointer listener)
{
  controlListeners.remove(listener);
}

void QtMainWindowShell::moveEvent(QMoveEvent* event)
{
  if (controlListeners.empty()) return;

  GuiTk::ControlEvent::Pointer controlEvent = new GuiTk::ControlEvent(this, event->pos().x(), event->pos().y());
  for (ControlListenersList::iterator listener = controlListeners.begin();
       listener != controlListeners.end(); ++listener)
  {
    (*listener)->ControlMoved(controlEvent);
  }
}

void QtMainWindowShell::resizeEvent(QResizeEvent* event)
{
  if (controlListeners.empty()) return;

  GuiTk::ControlEvent::Pointer controlEvent = new GuiTk::ControlEvent(this, 0, 0, event->size().width(), event->size().height());
  for (ControlListenersList::iterator listener = controlListeners.begin();
       listener != controlListeners.end(); ++listener)
  {
    (*listener)->ControlResized(controlEvent);
  }
}

void QtMainWindowShell::inFocusEvent(QFocusEvent* /*event*/)
{
  if (controlListeners.empty()) return;

  GuiTk::ControlEvent::Pointer controlEvent = new GuiTk::ControlEvent(this);
  for (ControlListenersList::iterator listener = controlListeners.begin();
       listener != controlListeners.end(); ++listener)
  {
    (*listener)->ControlActivated(controlEvent);
  }
}

}
