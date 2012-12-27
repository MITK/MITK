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

#include "berryPageSelectionService.h"
#include "berryPagePartSelectionTracker.h"

#include "berryIWorkbenchPage.h"

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
