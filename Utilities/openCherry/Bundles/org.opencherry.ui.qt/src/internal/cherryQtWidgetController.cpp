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

#include "cherryQtWidgetController.h"

#include <cherryShell.h>

namespace cherry {

const char QtWidgetController::PROPERTY_ID[] = "QtWidgetController_property";

QtWidgetController::QtWidgetController(Shell::Pointer shell)
 : shell(shell)
{

}

Shell::Pointer QtWidgetController::GetShell()
{
  return shell;
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
