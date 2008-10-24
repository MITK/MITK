/*=========================================================================

 Program:   openCherry Platform
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

#include "cherryQtControlWidget.h"

#include <algorithm>

namespace cherry {

QtControlWidget::QtControlWidget(QWidget* parent, Qt::WindowFlags f)
 : QFrame(parent, f)
{
  this->setFrameStyle(QFrame::Box | QFrame::Plain);
}

void QtControlWidget::AddControlListener(GuiTk::IControlListener::Pointer listener)
{
  ListenersListType::iterator iter = std::find(controlListeners.begin(), controlListeners.end(), listener);
  if (iter != controlListeners.end()) return;

  controlListeners.push_back(listener);
}

void QtControlWidget::RemoveControlListener(GuiTk::IControlListener::Pointer listener)
{
  controlListeners.remove(listener);
}

void QtControlWidget::moveEvent(QMoveEvent* event)
{
  if (controlListeners.empty()) return;

  GuiTk::ControlEvent::Pointer controlEvent = new GuiTk::ControlEvent();
  for (ListenersListType::iterator listener = controlListeners.begin();
       listener != controlListeners.end(); ++listener)
  {
    (*listener)->ControlMoved(controlEvent);
  }
}

void QtControlWidget::resizeEvent(QResizeEvent* event)
{
  if (controlListeners.empty()) return;

  std::cout << "RESIZE EVENT in QtControlWidget " << qPrintable(this->objectName()) << std::endl;
  GuiTk::ControlEvent::Pointer controlEvent = new GuiTk::ControlEvent();
  for (ListenersListType::iterator listener = controlListeners.begin();
       listener != controlListeners.end(); ++listener)
  {
    (*listener)->ControlResized(controlEvent);
  }
}

void QtControlWidget::showEvent(QShowEvent* event)
{
  if (controlListeners.empty()) return;

  GuiTk::ControlEvent::Pointer controlEvent = new GuiTk::ControlEvent();
  for (ListenersListType::iterator listener = controlListeners.begin();
       listener != controlListeners.end(); ++listener)
  {
    (*listener)->ControlActivated(controlEvent);
  }
}

}
