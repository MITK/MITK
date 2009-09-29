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

#include "cherryWindowSelectionService.h"

#include "cherryWindowPartSelectionTracker.h"
#include "../cherryIWorkbenchWindow.h"

namespace cherry
{

void WindowSelectionService::SetWindow(SmartPointer<IWorkbenchWindow> window)
{
  this->window = window;
}

SmartPointer<IWorkbenchWindow> WindowSelectionService::GetWindow() const
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
    SmartPointer<IWorkbenchWindow> window)
{
  SetWindow(window);
}

}
