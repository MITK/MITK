/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPagePartSelectionTracker.h"

#include "berryIPostSelectionProvider.h"

namespace berry
{

class SelTrackerPartListener: public IPartListener
{

public:

  SelTrackerPartListener(PagePartSelectionTracker* tracker)
  : tracker(tracker)
  {}

  Events::Types GetPartEventTypes() const override
  {
    return Events::CLOSED | Events::OPENED;
  }

  void PartClosed(const IWorkbenchPartReference::Pointer& partRef) override
  {
    if (tracker->GetPartId(partRef->GetPart(false))
        == tracker->AbstractPartSelectionTracker::GetPartId())
    {
      tracker->SetPart(IWorkbenchPart::Pointer(nullptr), true);
    }
  }

  void PartOpened(const IWorkbenchPartReference::Pointer& partRef) override
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

  IPerspectiveListener::Events::Types GetPerspectiveEventTypes() const override
  {
    return IPerspectiveListener::Events::PART_CHANGED;
  }

  using IPerspectiveListener::PerspectiveChanged;
  void PerspectiveChanged(const IWorkbenchPage::Pointer&,
                          const IPerspectiveDescriptor::Pointer&,
                          const IWorkbenchPartReference::Pointer& partRef,
                          const QString& changeId) override
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

  void SelectionChanged(const SelectionChangedEvent::Pointer& event) override
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
    const SelectionChangedEvent::Pointer& event)
{
  m_Tracker->FirePostSelection(m_Tracker->GetPart(), event->GetSelection());
}

PagePartSelectionTracker::PagePartSelectionTracker(
    IWorkbenchPage* page, const QString& partId) :
  AbstractPartSelectionTracker(partId)
{
  postSelectionListener.reset(new PostSelectionListener(this));
  perspListener.reset(new SelTrackerPerspectiveListener(this));
  selChangedListener.reset(new SelTrackerSelectionChangedListener(this));
  partListener.reset(new SelTrackerPartListener(this));

  this->SetPage(page);
  page->AddPartListener(partListener.data());
  page->GetWorkbenchWindow()->AddPerspectiveListener(perspListener.data());
  QString secondaryId;
  QString primaryId = partId;
  int indexOfColon;
  if ((indexOfColon = partId.indexOf(':')) != -1)
  {
    secondaryId = partId.mid(indexOfColon + 1);
    primaryId = partId.left(indexOfColon);
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
  return ISelection::Pointer(nullptr);
}

PagePartSelectionTracker::~PagePartSelectionTracker()
{
  IWorkbenchPage::Pointer page = GetPage();
  page->GetWorkbenchWindow()->RemovePerspectiveListener(perspListener.data());
  page->RemovePartListener(partListener.data());
  this->SetPart(IWorkbenchPart::Pointer(nullptr), false);
  this->SetPage(nullptr);
}

IWorkbenchPart::Pointer PagePartSelectionTracker::GetPart()
{
  return fPart;
}

IWorkbenchPage::Pointer PagePartSelectionTracker::GetPage()
{
  return IWorkbenchPage::Pointer(fPage);
}

ISelectionProvider::Pointer PagePartSelectionTracker::GetSelectionProvider()
{
  IWorkbenchPart::Pointer part = this->GetPart();
  if (part.IsNotNull())
  {
    return part->GetSite()->GetSelectionProvider();
  }
  return ISelectionProvider::Pointer(nullptr);
}

QString PagePartSelectionTracker::GetPartId(IWorkbenchPart::Pointer part)
{
  QString id = part->GetSite()->GetId();
  if (part.Cast<IViewPart> ().IsNotNull())
  {
    QString secondaryId =
        part.Cast<IViewPart> ()->GetViewSite() ->GetSecondaryId();
    if (secondaryId != "")
    {
      id = id + ':' + secondaryId;
    }
  }
  return id;
}

void PagePartSelectionTracker::SetPage(IWorkbenchPage* page)
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
      sp->RemoveSelectionChangedListener(selChangedListener.data());
      if (sp.Cast<IPostSelectionProvider> ().IsNotNull())
      {
        sp.Cast<IPostSelectionProvider> () ->RemovePostSelectionChangedListener(
            postSelectionListener.data());
      }
      else
      {
        sp->RemoveSelectionChangedListener(postSelectionListener.data());
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
      sp->AddSelectionChangedListener(selChangedListener.data());
      if (sp.Cast<IPostSelectionProvider> ().IsNotNull())
      {
        sp.Cast<IPostSelectionProvider> () ->AddPostSelectionChangedListener(
            postSelectionListener.data());
      }
      else
      {
        sp->AddSelectionChangedListener(postSelectionListener.data());
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
