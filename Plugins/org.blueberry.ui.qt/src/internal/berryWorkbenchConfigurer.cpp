/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  return Object::Pointer(nullptr);
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
    if (Workbench::GetInstance() != nullptr && !Workbench::GetInstance()->IsClosing())
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
