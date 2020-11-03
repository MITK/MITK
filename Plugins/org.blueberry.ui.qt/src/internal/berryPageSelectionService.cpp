/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    const QString& partId) const
{
  AbstractPartSelectionTracker::Pointer tracker(new PagePartSelectionTracker(GetPage(), partId));
  return tracker;
}

}
