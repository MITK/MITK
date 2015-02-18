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

#include "berryWorkbenchConfigurer.h"

#include "berryPlatformUI.h"
#include "berryWorkbenchWindow.h"
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

Object::Pointer WorkbenchConfigurer::GetData(const QString& key) const
{
  if (key.isEmpty())
  {
    throw Poco::InvalidArgumentException();
  }
  QHash<QString, Object::Pointer>::ConstIterator i = extraData.find(key);
  if (i != extraData.end())
    return i.value();
  return Object::Pointer(0);
}

void WorkbenchConfigurer::SetData(const QString& key, Object::Pointer data)
{
  if (key.isEmpty())
  {
    throw ctkInvalidArgumentException("Key must not be empty");
  }
  if (data)
  {
    extraData.insert(key, data);
  }
  else
  {
    extraData.remove(key);
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
