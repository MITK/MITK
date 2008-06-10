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

#include "cherryWorkbenchConfigurer.h"

#include "../cherryPlatformUI.h"
#include "../cherryWorkbenchWindow.h"
#include "../cherryWorkbench.h"

namespace cherry
{

WorkbenchConfigurer::WorkbenchConfigurer() :
  saveAndRestore(false), isEmergencyClosing(false), exitOnLastWindowClose(true)
{

}

IWorkbench::Pointer WorkbenchConfigurer::GetWorkbench()
{
  return PlatformUI::GetWorkbench();
}

IWorkbenchWindowConfigurer::Pointer WorkbenchConfigurer::GetWindowConfigurer(
    IWorkbenchWindow::Pointer window)
{
  if (window.IsNull())
  {
    throw Poco::InvalidArgumentException();
  }
  return window.Cast<WorkbenchWindow>()->GetWindowConfigurer();
}

bool WorkbenchConfigurer::GetSaveAndRestore()
{
  return saveAndRestore;
}

void WorkbenchConfigurer::SetSaveAndRestore(bool enabled)
{
  saveAndRestore = enabled;
}

void WorkbenchConfigurer::EmergencyClose()
{
  //        if (!isEmergencyClosing) {
  //            isEmergencyClosing = true;
  //            if (Workbench::GetInstance() != 0
  //                    && !Workbench::GetInstance()->IsClosing()) {
  //                Workbench::GetInstance()->Close(
  //                        PlatformUI::RETURN_EMERGENCY_CLOSE, true);
  //            }
  //        }
}

bool WorkbenchConfigurer::EmergencyClosing()
{
  return isEmergencyClosing;
}

bool WorkbenchConfigurer::RestoreState()
{
  //return ((Workbench) getWorkbench()).restoreState();
  return false;
}

void WorkbenchConfigurer::OpenFirstTimeWindow()
{
  this->GetWorkbench().Cast<Workbench>()->OpenFirstTimeWindow();
}

IWorkbenchWindowConfigurer::Pointer WorkbenchConfigurer::RestoreWorkbenchWindow(
    IMemento::Pointer memento)
{
  return this->GetWindowConfigurer(this->GetWorkbench().Cast<Workbench>()->RestoreWorkbenchWindow(memento));
}

bool WorkbenchConfigurer::GetExitOnLastWindowClose()
{
  return exitOnLastWindowClose;
}

void WorkbenchConfigurer::SetExitOnLastWindowClose(bool enabled)
{
  exitOnLastWindowClose = enabled;
}

}
