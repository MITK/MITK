
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

#include "cherryQtWorkbenchTweaklet.h"
#include "cherryQtControlWidget.h"

#include <QApplication>
#include <QMainWindow>

#include "cherryQtShowViewDialog.h"
#include "cherryQtDisplay.h"
#include "cherryQtWorkbenchWindow.h"

#include <cherryPlatform.h>
#include <cherryPlatformUI.h>
#include <cherryWorkbenchWindow.h>

namespace cherry {

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


}  // namespace cherry
