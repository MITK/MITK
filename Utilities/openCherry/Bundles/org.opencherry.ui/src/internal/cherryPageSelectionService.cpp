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

#include "cherryPageSelectionService.h"

#include "../cherryIPostSelectionProvider.h"

namespace cherry
{

ISelectionService::SelectionEvents& PageSelectionService::GetSelectionEvents(const std::string& partId)
{
  if (partId.empty())
  {
    return selectionEvents;
  }

  return this->GetPerPartTracker(partId)->GetSelectionEvents();
}

void PageSelectionService::SetPage(IWorkbenchPage::Pointer p)
{
  this->page = p;
}

PageSelectionService::PageSelectionService(IWorkbenchPage::Pointer p)
{
  this->SetPage(p);
}

PageSelectionService::SelectionListener::SelectionListener(PageSelectionService* service)
 : m_SelectionService(service)
{ }

void PageSelectionService::SelectionListener::SelectionChanged(SelectionChangedEvent::Pointer event)
{
  m_SelectionService->FireSelection(m_SelectionService->activePart, event->GetSelection());
}

PageSelectionService::PostSelectionListener::PostSelectionListener(PageSelectionService* service)
 : m_SelectionService(service)
{ }

void PageSelectionService::PostSelectionListener::SelectionChanged(
    SelectionChangedEvent::Pointer event)
{
  m_SelectionService->FirePostSelection(m_SelectionService->activePart, event->GetSelection());
}

void PageSelectionService::AddSelectionListener(ISelectionListener::Pointer l)
{
  selectionEvents.selectionChanged +=
    ISelectionService::SelectionEvents::Delegate(l.GetPointer(),
      &ISelectionListener::SelectionChanged);
}

void PageSelectionService::AddSelectionListener(const std::string& partId,
    ISelectionListener::Pointer listener)
{
  this->GetPerPartTracker(partId)->AddSelectionListener(listener);
}

void PageSelectionService::AddPostSelectionListener(
    ISelectionListener::Pointer l)
{
  selectionEvents.postSelectionChanged +=
    ISelectionService::SelectionEvents::Delegate(l.GetPointer(),
      &ISelectionListener::SelectionChanged);
}

void PageSelectionService::AddPostSelectionListener(const std::string& partId,
    ISelectionListener::Pointer listener)
{
  this->GetPerPartTracker(partId)->AddPostSelectionListener(listener);
}

void PageSelectionService::RemoveSelectionListener(ISelectionListener::Pointer l)
{
  selectionEvents.selectionChanged -=
    ISelectionService::SelectionEvents::Delegate(l.GetPointer(),
      &ISelectionListener::SelectionChanged);
}

void PageSelectionService::RemovePostSelectionListener(
    const std::string& partId, ISelectionListener::Pointer listener)
{
  this->GetPerPartTracker(partId)->RemovePostSelectionListener(listener);
}

void PageSelectionService::RemovePostSelectionListener(
    ISelectionListener::Pointer l)
{
  selectionEvents.postSelectionChanged -=
    ISelectionService::SelectionEvents::Delegate(l.GetPointer(),
      &ISelectionListener::SelectionChanged);
}

void PageSelectionService::RemoveSelectionListener(const std::string& partId,
    ISelectionListener::Pointer listener)
{
  this->GetPerPartTracker(partId)->RemoveSelectionListener(listener);
}

IWorkbenchPage::Pointer PageSelectionService::GetPage()
{
  return IWorkbenchPage::Pointer(page);
}

void PageSelectionService::FireSelection(IWorkbenchPart::Pointer part,
    ISelection::Pointer sel)
{
   selectionEvents.selectionChanged(part, sel);
}

void PageSelectionService::FirePostSelection(IWorkbenchPart::Pointer part,
    ISelection::Pointer sel)
{
  selectionEvents.postSelectionChanged(part, sel);
}

PagePartSelectionTracker::Pointer PageSelectionService::GetPerPartTracker(
    const std::string& partId)
{
  PagePartSelectionTracker::Pointer tracker;
  std::map<std::string, PagePartSelectionTracker::Pointer>::iterator res = perPartTrackers.find(partId);
  if (res == perPartTrackers.end())
  {
    tracker = this->CreatePartTracker(partId);
    perPartTrackers[partId] = tracker;
  }
  else
  {
    tracker = res->second;
  }

  return tracker;
}

PagePartSelectionTracker::Pointer PageSelectionService::CreatePartTracker(
    const std::string& partId)
{
  PagePartSelectionTracker::Pointer tracker(new PagePartSelectionTracker(this->GetPage(), partId));
  return tracker;
}

ISelection::Pointer PageSelectionService::GetSelection()
{
  if (activeProvider.IsNotNull())
  {
    return activeProvider->GetSelection();
  }
  else
  {
    return ISelection::Pointer(0);
  }
}

ISelection::Pointer PageSelectionService::GetSelection(const std::string& partId)
{
  return this->GetPerPartTracker(partId)->GetSelection();
}

void PageSelectionService::SetActivePart(IWorkbenchPart::Pointer newPart)
{
  // Optimize.
  if (newPart == activePart)
  {
    return;
  }

  ISelectionProvider::Pointer selectionProvider;

  if (newPart.IsNotNull())
  {
    selectionProvider = newPart->GetSite()->GetSelectionProvider();

    if (selectionProvider.IsNull())
    {
      newPart = 0;
    }
  }

  if (newPart == activePart)
  {
    return;
  }

  if (activePart.IsNotNull())
  {
    if (activeProvider.IsNotNull())
    {
      activeProvider->RemoveSelectionChangedListener(selListener);
      if (activeProvider.Cast<IPostSelectionProvider>().IsNotNull())
      {
        activeProvider.Cast<IPostSelectionProvider>()
        ->RemovePostSelectionChangedListener(postSelListener);
      }
      else
      {
        activeProvider
        ->RemoveSelectionChangedListener(postSelListener);
      }
      activeProvider = 0;
    }
    activePart = 0;
  }

  activePart = newPart;

  if (newPart.IsNotNull())
  {
    activeProvider = selectionProvider;
    // Fire an event if there's an active provider
    activeProvider->AddSelectionChangedListener(selListener);
    ISelection::Pointer sel = activeProvider->GetSelection();
    this->FireSelection(newPart, sel);
    if (activeProvider.Cast<IPostSelectionProvider>().IsNotNull())
    {
      activeProvider.Cast<IPostSelectionProvider>()
      ->AddPostSelectionChangedListener(postSelListener);
    }
    else
    {
      activeProvider->AddSelectionChangedListener(postSelListener);
    }
    this->FirePostSelection(newPart, sel);
  }
  else
  {
    this->FireSelection(IWorkbenchPart::Pointer(0), ISelection::Pointer(0));
    this->FirePostSelection(IWorkbenchPart::Pointer(0), ISelection::Pointer(0));
  }
}

}
