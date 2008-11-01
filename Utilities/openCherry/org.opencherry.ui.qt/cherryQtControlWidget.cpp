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
  controlEvents.AddListener(listener);
}

void QtControlWidget::RemoveControlListener(GuiTk::IControlListener::Pointer listener)
{
  controlEvents.RemoveListener(listener);
}

void QtControlWidget::moveEvent(QMoveEvent* event)
{
  GuiTk::ControlEvent::Pointer controlEvent = new GuiTk::ControlEvent();
  controlEvents.movedEvent(controlEvent);
}

void QtControlWidget::resizeEvent(QResizeEvent* event)
{
  GuiTk::ControlEvent::Pointer controlEvent = new GuiTk::ControlEvent();
  controlEvents.resizedEvent(controlEvent);
}

void QtControlWidget::showEvent(QShowEvent* event)
{
  GuiTk::ControlEvent::Pointer controlEvent = new GuiTk::ControlEvent();
  controlEvents.activatedEvent(controlEvent);
}

}
