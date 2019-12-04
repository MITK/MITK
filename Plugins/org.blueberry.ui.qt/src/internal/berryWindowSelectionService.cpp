/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryWindowSelectionService.h"

#include "berryWindowPartSelectionTracker.h"
#include "berryIWorkbenchWindow.h"

namespace berry
{

void WindowSelectionService::SetWindow(IWorkbenchWindow* window)
{
  this->window = window;
}

IWorkbenchWindow* WindowSelectionService::GetWindow() const
{
  return window;
}

AbstractPartSelectionTracker::Pointer WindowSelectionService::CreatePartTracker(
    const QString& partId) const
{
  AbstractPartSelectionTracker::Pointer tracker(new WindowPartSelectionTracker(
      GetWindow(), partId));
  return tracker;
}

WindowSelectionService::WindowSelectionService(
    IWorkbenchWindow* window)
{
  SetWindow(window);
}

}
