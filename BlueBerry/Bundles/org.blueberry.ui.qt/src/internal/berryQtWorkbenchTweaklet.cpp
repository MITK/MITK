
/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryQtWorkbenchTweaklet.h"
#include <internal/berryQtControlWidget.h>

#include <QApplication>
#include <QMainWindow>

#include "berryQtShowViewDialog.h"
#include "berryQtDisplay.h"
#include "berryQtWorkbenchWindow.h"

#include <berryPlatform.h>
#include <berryPlatformUI.h>

#include <internal/berryWorkbenchWindow.h>

namespace berry {

QtWorkbenchTweaklet::QtWorkbenchTweaklet()
{

}

QtWorkbenchTweaklet::QtWorkbenchTweaklet(const QtWorkbenchTweaklet& other)
{

}

Display* QtWorkbenchTweaklet::CreateDisplay()
{
  return new QtDisplay();
}

bool QtWorkbenchTweaklet::IsRunning()
{
  return QApplication::instance() != 0;
}

WorkbenchWindow::Pointer QtWorkbenchTweaklet::CreateWorkbenchWindow(int number)
{
  WorkbenchWindow::Pointer wnd(new QtWorkbenchWindow(number));
  return wnd;
}

IDialog::Pointer
QtWorkbenchTweaklet::CreateStandardDialog(const std::string& dialogid)
{
  if (dialogid == DIALOG_ID_SHOW_VIEW)
    return IDialog::Pointer(new QtShowViewDialog(PlatformUI::GetWorkbench()->GetViewRegistry()));
  else
    return IDialog::Pointer(0);
}


}  // namespace berry
