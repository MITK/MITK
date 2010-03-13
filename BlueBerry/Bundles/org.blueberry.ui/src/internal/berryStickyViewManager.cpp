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

#include "berryStickyViewManager.h"

#include "berryPerspective.h"
#include "berryWorkbenchPlugin.h"

#include <berryIStickyViewDescriptor.h>
#include <berryIWorkbenchPage.h>

namespace berry
{

StickyViewManager::StickyViewManager(IWorkbenchPage* page) :
  page(page)
{

}

IStickyViewManager::Pointer StickyViewManager::GetInstance(IWorkbenchPage* page)
{
  IStickyViewManager::Pointer stickyViewMan;
  //    IPreferenceStore preferenceStore = PrefUtil.getAPIPreferenceStore();
  //    boolean enable32Behavior = preferenceStore
  //        .getBoolean(IWorkbenchPreferenceConstants.ENABLE_32_STICKY_CLOSE_BEHAVIOR);
  //    if (enable32Behavior)
  //      stickyViewMan = new StickyViewManager32(page);
  //    else
  stickyViewMan = IStickyViewManager::Pointer(new StickyViewManager(page));

  return stickyViewMan;
}

void StickyViewManager::Add(const std::string& /*perspectiveId*/, const std::set<std::string>& /*stickyViewSet*/)
{
  // do nothing
}

void StickyViewManager::Clear()
{
  // do nothing
}

void StickyViewManager::Remove(const std::string& /*perspectiveId*/)
{
  // do nothing
}

void StickyViewManager::Restore(IMemento::Pointer /*memento*/)
{
  // do nothing
}

void StickyViewManager::Save(IMemento::Pointer /*memento*/)
{
  // do nothing
}

void StickyViewManager::Update(Perspective::Pointer oldPersp,
    Perspective::Pointer newPersp)
{
  if (oldPersp == 0 || newPersp == 0)
  {
    return;
  }
  IViewRegistry* viewReg = WorkbenchPlugin::GetDefault()->GetViewRegistry();
  std::vector<IStickyViewDescriptor::Pointer> stickyDescs(
      viewReg->GetStickyViews());
  for (std::size_t i = 0; i < stickyDescs.size(); i++)
  {
    const std::string viewId = stickyDescs[i]->GetId();
    try
    {
      // show a sticky view if it was in the last perspective and
      // hasn't already been activated in this one
      if (oldPersp->FindView(viewId))
      {
        page->ShowView(viewId, "", IWorkbenchPage::VIEW_CREATE);
      }
      // remove a view if it's sticky and its not visible in the old
      // perspective
      else if (newPersp->FindView(viewId) && oldPersp->FindView(viewId) == 0)
      {
        page->HideView(newPersp->FindView(viewId));
      }
    } catch (PartInitException& e)
    {
      //TODO IStatus
      //        WorkbenchPlugin::Log(
      //                "Could not open view :" + viewId, new Status(IStatus.ERROR, WorkbenchPlugin.PI_WORKBENCH, IStatus.ERROR, "Could not open view :" + viewId, e)); //$NON-NLS-1$ //$NON-NLS-2$
      WorkbenchPlugin::Log("Could not open view: " + viewId, e);
    }
  }
}

}
