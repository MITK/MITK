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

#include "cherryPagePartSelectionTracker.h"

#include "../cherryIPostSelectionProvider.h"

namespace cherry
{

void PagePartSelectionTracker::SelectionChanged(
    SelectionChangedEvent::Pointer event)
{
  this->FireSelection(this->GetPart(), event->GetSelection());
}

ISelectionService::SelectionEvents& PagePartSelectionTracker::GetSelectionEvents()
{
  return selectionEvents;
}

PagePartSelectionTracker::
PostSelectionListener::PostSelectionListener(PagePartSelectionTracker* tracker)
 : m_Tracker(tracker)
{ }

void PagePartSelectionTracker::PostSelectionListener::SelectionChanged(
    SelectionChangedEvent::Pointer event)
{
  m_Tracker->FirePostSelection(m_Tracker->GetPart(), event->GetSelection());
}

PagePartSelectionTracker::PagePartSelectionTracker(
    IWorkbenchPage::Pointer page, const std::string& partId)
{
  postSelectionListener = new PostSelectionListener(this);

  this->SetPartId(partId);
  this->SetPage(page);

  page->AddPartListener(this);
  std::string secondaryId;
  std::string primaryId = partId;
  std::string::size_type indexOfColon;
  if ((indexOfColon = partId.find_first_of(':')) != std::string::npos)
  {
    secondaryId = partId.substr(indexOfColon + 1);
    primaryId = partId.substr(0, indexOfColon);
  }
  IViewReference::Pointer part = page->FindViewReference(primaryId, secondaryId);
  if (part.IsNotNull() && part->GetView(false).IsNotNull())
  {
    this->SetPart(part->GetView(false), false);
  }
}

void PagePartSelectionTracker::PartActivated(IWorkbenchPartReference::Pointer partRef)
{
}

void PagePartSelectionTracker::PartBroughtToTop(IWorkbenchPartReference::Pointer partRef)
{
}

void PagePartSelectionTracker::PartClosed(IWorkbenchPartReference::Pointer partRef)
{
  if (this->GetPartId(partRef->GetPart(false)) == this->GetPartId())
  {
    this->SetPart(0, true);
  }
}

void PagePartSelectionTracker::PartDeactivated(IWorkbenchPartReference::Pointer partRef)
{
}

void PagePartSelectionTracker::PartOpened(IWorkbenchPartReference::Pointer partRef)
{
  if (this->GetPartId(partRef->GetPart(false)) == this->GetPartId())
  {
    this->SetPart(partRef->GetPart(false), true);
  }
}

void PagePartSelectionTracker::PartHidden(IWorkbenchPartReference::Pointer partRef)
{

}

void PagePartSelectionTracker::PartVisible(IWorkbenchPartReference::Pointer partRef)
{

}

void PagePartSelectionTracker::PartInputChanged(IWorkbenchPartReference::Pointer partRef)
{

}

void PagePartSelectionTracker::AddSelectionListener(
    ISelectionListener::Pointer listener)
{
  selectionEvents.selectionChanged +=
    ISelectionService::SelectionEvents::Delegate(listener.GetPointer(), &ISelectionListener::SelectionChanged);
}

void PagePartSelectionTracker::AddPostSelectionListener(
    ISelectionListener::Pointer listener)
{
  selectionEvents.postSelectionChanged +=
    ISelectionService::SelectionEvents::Delegate(listener.GetPointer(), &ISelectionListener::SelectionChanged);
}

ISelection::Pointer PagePartSelectionTracker::GetSelection()
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
  return 0;
}

void PagePartSelectionTracker::RemoveSelectionListener(
    ISelectionListener::Pointer listener)
{
  selectionEvents.selectionChanged -=
    ISelectionService::SelectionEvents::Delegate(listener.GetPointer(), &ISelectionListener::SelectionChanged);
}

void PagePartSelectionTracker::RemovePostSelectionListener(
    ISelectionListener::Pointer listener)
{
  selectionEvents.postSelectionChanged -=
    ISelectionService::SelectionEvents::Delegate(listener.GetPointer(), &ISelectionListener::SelectionChanged);
}

PagePartSelectionTracker::~PagePartSelectionTracker()
{
  this->SetPart(0, false);
  this->SetPage(0);
}

void PagePartSelectionTracker::FireSelection(IWorkbenchPart::Pointer part,
    ISelection::Pointer sel)
{
  selectionEvents.selectionChanged(part, sel);
}

void PagePartSelectionTracker::FirePostSelection(IWorkbenchPart::Pointer part,
    ISelection::Pointer sel)
{
  selectionEvents.postSelectionChanged(part, sel);
}

std::string PagePartSelectionTracker::GetPartId()
{
  return fPartId;
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
  return 0;
}

void PagePartSelectionTracker::SetPartId(const std::string& partId)
{
  fPartId = partId;
}

std::string PagePartSelectionTracker::GetPartId(IWorkbenchPart::Pointer part)
{
  std::string id = part->GetSite()->GetId();
  if (part.Cast<IViewPart>().IsNotNull())
  {
    std::string secondaryId = part.Cast<IViewPart>()->GetViewSite()
    ->GetSecondaryId();
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

void PagePartSelectionTracker::SetPart(IWorkbenchPart::Pointer part, bool notify)
{
  if (fPart.IsNotNull())
  {
    // remove myself as a listener from the existing part
    ISelectionProvider::Pointer sp = fPart->GetSite()->GetSelectionProvider();
    if (sp.IsNotNull())
    {
      sp->RemoveSelectionChangedListener(this);
      if (sp.Cast<IPostSelectionProvider>().IsNotNull())
      {
        sp.Cast<IPostSelectionProvider>()
        ->RemovePostSelectionChangedListener(postSelectionListener);
      }
      else
      {
        sp->RemoveSelectionChangedListener(postSelectionListener);
      }
    }
  }
  fPart = part;
  ISelection::Pointer sel;
  if (part.IsNotNull())
  {
    ISelectionProvider::Pointer sp = part->GetSite()->GetSelectionProvider();
    if (sp.IsNotNull())
    {
      sp->AddSelectionChangedListener(this);
      if (sp.Cast<IPostSelectionProvider>().IsNotNull())
      {
        sp.Cast<IPostSelectionProvider>()
        ->AddPostSelectionChangedListener(postSelectionListener);
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
