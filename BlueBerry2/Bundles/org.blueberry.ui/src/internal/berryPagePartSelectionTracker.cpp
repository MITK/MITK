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

#include "berryPagePartSelectionTracker.h"

#include "../berryIPostSelectionProvider.h"

namespace berry
{

class SelTrackerPartListener: public IPartListener
{

public:

  SelTrackerPartListener(PagePartSelectionTracker* tracker)
  : tracker(tracker)
  {}

  Events::Types GetPartEventTypes() const
  {
    return Events::CLOSED | Events::OPENED;
  }

  void PartClosed(IWorkbenchPartReference::Pointer partRef)
  {
    if (tracker->GetPartId(partRef->GetPart(false))
        == tracker->AbstractPartSelectionTracker::GetPartId())
    {
      tracker->SetPart(IWorkbenchPart::Pointer(0), true);
    }
  }

  void PartOpened(IWorkbenchPartReference::Pointer partRef)
  {
    if (tracker->GetPartId(partRef->GetPart(false))
        == tracker->AbstractPartSelectionTracker::GetPartId())
    {
      tracker->SetPart(partRef->GetPart(false), true);
    }
  }

private:

  PagePartSelectionTracker* tracker;

};

class SelTrackerPerspectiveListener: public IPerspectiveListener
{
public:

  SelTrackerPerspectiveListener(PagePartSelectionTracker* tracker)
  : tracker(tracker)
  {}

  IPerspectiveListener::Events::Types GetPerspectiveEventTypes() const
  {
    return IPerspectiveListener::Events::PART_CHANGED;
  }

  void PerspectiveChanged(IWorkbenchPage::Pointer page,
      IPerspectiveDescriptor::Pointer perspective,
      IWorkbenchPartReference::Pointer partRef, const std::string& changeId)
  {
    if (!partRef)
      return;
    IWorkbenchPart::Pointer part = partRef->GetPart(false);
    if (!part)
      return;

    if (IWorkbenchPage::CHANGE_VIEW_SHOW == changeId)
    {
      if (tracker->GetPart()) // quick check first, plus avoids double setting
        return;
      if (tracker->GetPartId(part) == tracker->AbstractPartSelectionTracker::GetPartId())
        tracker->SetPart(part, true);
    }
  }

private:

  PagePartSelectionTracker* tracker;
};

class SelTrackerSelectionChangedListener: public ISelectionChangedListener
{
public:

  SelTrackerSelectionChangedListener(PagePartSelectionTracker* tracker)
  : tracker(tracker)
  {}

  void SelectionChanged(SelectionChangedEvent::Pointer event)
  {
    tracker->FireSelection(tracker->GetPart(), event->GetSelection());
  }

private:

  PagePartSelectionTracker* tracker;

};

PagePartSelectionTracker::PostSelectionListener::PostSelectionListener(
    PagePartSelectionTracker* tracker) :
  m_Tracker(tracker)
{
}

void PagePartSelectionTracker::PostSelectionListener::SelectionChanged(
    SelectionChangedEvent::Pointer event)
{
  m_Tracker->FirePostSelection(m_Tracker->GetPart(), event->GetSelection());
}

PagePartSelectionTracker::PagePartSelectionTracker(
    IWorkbenchPage::Pointer page, const std::string& partId) :
  AbstractPartSelectionTracker(partId)
{
  postSelectionListener = new PostSelectionListener(this);
  perspListener = new SelTrackerPerspectiveListener(this);
  selChangedListener = new SelTrackerSelectionChangedListener(this);
  partListener = new SelTrackerPartListener(this);

  this->SetPage(page);
  page->AddPartListener(partListener);
  page->GetWorkbenchWindow()->AddPerspectiveListener(
      perspListener);
  std::string secondaryId;
  std::string primaryId = partId;
  std::string::size_type indexOfColon;
  if ((indexOfColon = partId.find_first_of(':')) != std::string::npos)
  {
    secondaryId = partId.substr(indexOfColon + 1);
    primaryId = partId.substr(0, indexOfColon);
  }
  IViewReference::Pointer part =
      page->FindViewReference(primaryId, secondaryId);
  if (part.IsNotNull() && part->GetView(false).IsNotNull())
  {
    this->SetPart(part->GetView(false), false);
  }
}

ISelection::ConstPointer PagePartSelectionTracker::GetSelection()
{
  IWorkbenchPart::Pointer part = this->GetPart();
  if (part.IsNotNull())
  {
    ISelectionProvider::Pointer sp = part->GetSite()->GetSelectionProvider();
    if (sp.IsNotNull())
    {
      return sp->GetSelection();
    }
  }
  return ISelection::Pointer(0);
}

PagePartSelectionTracker::~PagePartSelectionTracker()
{
  IWorkbenchPage::Pointer page = GetPage();
  page->GetWorkbenchWindow()->RemovePerspectiveListener(
      perspListener);
  page->RemovePartListener(partListener);
  this->SetPart(IWorkbenchPart::Pointer(0), false);
  this->SetPage(IWorkbenchPage::Pointer(0));
}

IWorkbenchPart::Pointer PagePartSelectionTracker::GetPart()
{
  return fPart;
}

IWorkbenchPage::Pointer PagePartSelectionTracker::GetPage()
{
  return fPage;
}

ISelectionProvider::Pointer PagePartSelectionTracker::GetSelectionProvider()
{
  IWorkbenchPart::Pointer part = this->GetPart();
  if (part.IsNotNull())
  {
    return part->GetSite()->GetSelectionProvider();
  }
  return ISelectionProvider::Pointer(0);
}

std::string PagePartSelectionTracker::GetPartId(IWorkbenchPart::Pointer part)
{
  std::string id = part->GetSite()->GetId();
  if (part.Cast<IViewPart> ().IsNotNull())
  {
    std::string secondaryId =
        part.Cast<IViewPart> ()->GetViewSite() ->GetSecondaryId();
    if (secondaryId != "")
    {
      id = id + ':' + secondaryId;
    }
  }
  return id;
}

void PagePartSelectionTracker::SetPage(IWorkbenchPage::Pointer page)
{
  fPage = page;
}

void PagePartSelectionTracker::SetPart(IWorkbenchPart::Pointer part,
    bool notify)
{
  if (fPart.IsNotNull())
  {
    // remove myself as a listener from the existing part
    ISelectionProvider::Pointer sp = fPart->GetSite()->GetSelectionProvider();
    if (sp.IsNotNull())
    {
      sp->RemoveSelectionChangedListener(selChangedListener);
      if (sp.Cast<IPostSelectionProvider> ().IsNotNull())
      {
        sp.Cast<IPostSelectionProvider> () ->RemovePostSelectionChangedListener(
            postSelectionListener);
      }
      else
      {
        sp->RemoveSelectionChangedListener(postSelectionListener);
      }
    }
  }
  fPart = part;
  ISelection::ConstPointer sel;
  if (part.IsNotNull())
  {
    ISelectionProvider::Pointer sp = part->GetSite()->GetSelectionProvider();
    if (sp.IsNotNull())
    {
      sp->AddSelectionChangedListener(selChangedListener);
      if (sp.Cast<IPostSelectionProvider> ().IsNotNull())
      {
        sp.Cast<IPostSelectionProvider> () ->AddPostSelectionChangedListener(
            postSelectionListener);
      }
      else
      {
        sp->AddSelectionChangedListener(postSelectionListener);
      }
      if (notify)
      {
        // get the selection to send below
        sel = sp->GetSelection();
      }
    }
  }
  if (notify)
  {
    this->FireSelection(part, sel);
    this->FirePostSelection(part, sel);
  }
}

}
