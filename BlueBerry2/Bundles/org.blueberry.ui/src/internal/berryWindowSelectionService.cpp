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

#include "berryWindowSelectionService.h"

#include "berryWindowPartSelectionTracker.h"
#include "../berryIWorkbenchWindow.h"

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
