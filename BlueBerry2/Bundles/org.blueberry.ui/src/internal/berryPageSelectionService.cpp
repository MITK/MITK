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

#include "berryPageSelectionService.h"
#include "berryPagePartSelectionTracker.h"

#include "../berryIWorkbenchPage.h"

namespace berry
{

void PageSelectionService::SetPage(IWorkbenchPage* page)
{
  this->page = page;
}

PageSelectionService::PageSelectionService(IWorkbenchPage* page)
{
  SetPage(page);
}

IWorkbenchPage* PageSelectionService::GetPage() const
{
  return page;
}

AbstractPartSelectionTracker::Pointer PageSelectionService::CreatePartTracker(
    const std::string& partId) const
{
  AbstractPartSelectionTracker::Pointer tracker(new PagePartSelectionTracker(GetPage(), partId));
  return tracker;
}

}
