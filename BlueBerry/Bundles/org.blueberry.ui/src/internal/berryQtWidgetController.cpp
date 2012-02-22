/*=========================================================================

 Program:   BlueBerry Platform
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

#include "berryQtWidgetController.h"

#include <berryShell.h>

namespace berry {

const char QtWidgetController::PROPERTY_ID[] = "QtWidgetController_property";

QtWidgetController::QtWidgetController(Shell* shell)
 : shell(shell)
{
  if (shell)
  {
    // Normally, a Shell outlives a QtWidgetController, but
    // in QtWorkbenchWindow::~QtWorkbenchWindow the main widget is
    // deleted via widget->deleteLater() and hence an instance of this
    // class could still be alive when the Shell is deleted
    shell->AddDestroyListener(MessageDelegate<QtWidgetController>(this, &QtWidgetController::ShellDestroyed));
  }
}

QtWidgetController::~QtWidgetController()
{
  if (shell)
  {
    shell->RemoveDestroyListener(MessageDelegate<QtWidgetController>(this, &QtWidgetController::ShellDestroyed));
  }
}

Shell::Pointer QtWidgetController::GetShell()
{
  return Shell::Pointer(shell);
}

void QtWidgetController::ShellDestroyed()
{
  shell = 0;
}

void QtWidgetController::AddControlListener(GuiTk::IControlListener::Pointer listener)
{
  controlEvents.AddListener(listener);
}

void QtWidgetController::RemoveControlListener(GuiTk::IControlListener::Pointer listener)
{
  controlEvents.RemoveListener(listener);
}

void QtWidgetController::AddShellListener(IShellListener::Pointer listener)
{
  shellEvents.AddListener(listener);
}

void QtWidgetController::RemoveShellListener(IShellListener::Pointer listener)
{
  shellEvents.RemoveListener(listener);
}

}
