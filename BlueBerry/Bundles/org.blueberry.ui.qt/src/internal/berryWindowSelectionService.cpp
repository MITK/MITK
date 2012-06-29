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
    const std::string& partId) const
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
