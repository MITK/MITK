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

#include "berryWorkbenchConfigurer.h"

#include "../berryPlatformUI.h"
#include "berryWorkbenchWindow.h"
#include "../berryImageDescriptor.h"
#include "berryWorkbench.h"

namespace berry
{

WorkbenchConfigurer::WorkbenchConfigurer() :
  saveAndRestore(false), isEmergencyClosing(false), exitOnLastWindowClose(true)
{

}

IWorkbench* WorkbenchConfigurer::GetWorkbench()
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
  return window.Cast<WorkbenchWindow> ()->GetWindowConfigurer();
}

bool WorkbenchConfigurer::GetSaveAndRestore()
{
  return saveAndRestore;
}

void WorkbenchConfigurer::SetSaveAndRestore(bool enabled)
{
  saveAndRestore = enabled;
}

Object::Pointer WorkbenchConfigurer::GetData(const std::string& key) const
{
  if (key.empty())
  {
    throw Poco::InvalidArgumentException();
  }
  Poco::HashMap<std::string, Object::Pointer>::ConstIterator i =
      extraData.find(key);
  if (i != extraData.end())
    return i->second;
  return Object::Pointer(0);
}

void WorkbenchConfigurer::SetData(const std::string& key, Object::Pointer data)
{
  if (key.empty())
  {
    throw Poco::InvalidArgumentException();
  }
  if (data)
  {
    extraData.insert(std::make_pair(key, data));
  }
  else
  {
    extraData.erase(key);
  }
}

void WorkbenchConfigurer::EmergencyClose()
{
  if (!isEmergencyClosing)
  {
    isEmergencyClosing = true;
    if (Workbench::GetInstance() != 0 && !Workbench::GetInstance()->IsClosing())
    {
      Workbench::GetInstance()->Close(PlatformUI::RETURN_EMERGENCY_CLOSE, true);
    }
  }
}

bool WorkbenchConfigurer::EmergencyClosing()
{
  return isEmergencyClosing;
}

bool WorkbenchConfigurer::RestoreState()
{
  return dynamic_cast<Workbench*> (GetWorkbench())->RestoreState();
}

void WorkbenchConfigurer::OpenFirstTimeWindow()
{
  dynamic_cast<Workbench*> (this->GetWorkbench())->OpenFirstTimeWindow();
}

IWorkbenchWindowConfigurer::Pointer WorkbenchConfigurer::RestoreWorkbenchWindow(
    IMemento::Pointer memento)
{
  return this->GetWindowConfigurer(
      dynamic_cast<Workbench*> (this->GetWorkbench())->RestoreWorkbenchWindow(
          memento));
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
