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

#include "berryAbstractSelectionService.h"
#include "berryWorkbenchPlugin.h"

#include "berryIPostSelectionProvider.h"
#include "berryINullSelectionListener.h"

namespace berry
{

//ISelectionService::SelectionEvents& AbstractSelectionService::GetSelectionEvents(const QString& partId)
//{
//  if (partId.empty())
//  {
//    return selectionEvents;
//  }
//
//  return this->GetPerPartTracker(partId)->GetSelectionEvents();
//}

AbstractSelectionService::AbstractSelectionService()
{
  selListener = new SelectionListener(this);
  postSelListener = new PostSelectionListener(this);
}

AbstractSelectionService::SelectionListener::SelectionListener(AbstractSelectionService* service)
 : m_SelectionService(service)
{ }

void AbstractSelectionService::SelectionListener::SelectionChanged(SelectionChangedEvent::Pointer event)
{
  m_SelectionService->FireSelection(m_SelectionService->activePart, event->GetSelection());
}

AbstractSelectionService::PostSelectionListener::PostSelectionListener(AbstractSelectionService* service)
 : m_SelectionService(service)
{ }

void AbstractSelectionService::PostSelectionListener::SelectionChanged(
    SelectionChangedEvent::Pointer event)
{
  m_SelectionService->FirePostSelection(m_SelectionService->activePart, event->GetSelection());
}

void AbstractSelectionService::AddSelectionListener(ISelectionListener::Pointer l)
{
  fListeners.push_back(l);
}

void AbstractSelectionService::AddSelectionListener(const QString& partId,
    ISelectionListener::Pointer listener)
{
  this->GetPerPartTracker(partId)->AddSelectionListener(listener);
}

void AbstractSelectionService::AddPostSelectionListener(
    ISelectionListener::Pointer l)
{
  fPostListeners.push_back(l);
}

void AbstractSelectionService::AddPostSelectionListener(const QString& partId,
    ISelectionListener::Pointer listener)
{
  this->GetPerPartTracker(partId)->AddPostSelectionListener(listener);
}

void AbstractSelectionService::RemoveSelectionListener(ISelectionListener::Pointer l)
{
  fListeners.removeAll(l);
}

void AbstractSelectionService::RemovePostSelectionListener(
    const QString& partId, ISelectionListener::Pointer listener)
{
  this->GetPerPartTracker(partId)->RemovePostSelectionListener(listener);
}

void AbstractSelectionService::RemovePostSelectionListener(
    ISelectionListener::Pointer l)
{
  fPostListeners.removeAll(l);
}

void AbstractSelectionService::RemoveSelectionListener(const QString& partId,
    ISelectionListener::Pointer listener)
{
  this->GetPerPartTracker(partId)->RemoveSelectionListener(listener);
}

void AbstractSelectionService::FireSelection(IWorkbenchPart::Pointer part,
    ISelection::ConstPointer sel)
{
  for (QList<ISelectionListener::Pointer>::iterator i = fListeners.begin();
      i != fListeners.end(); ++i)
  {
    ISelectionListener::Pointer l = *i;
    if ((part && sel) || l.Cast<INullSelectionListener>())
    {
      try
      {
        l->SelectionChanged(part, sel);
      }
      catch (const ctkRuntimeException& rte)
      {
        WorkbenchPlugin::Log(rte);
      }
      catch (const std::exception& e)
      {
        WorkbenchPlugin::Log(e.what());
      }
    }
  }
}

void AbstractSelectionService::FirePostSelection(IWorkbenchPart::Pointer part,
    ISelection::ConstPointer sel)
{
  for (QList<ISelectionListener::Pointer>::iterator i = fPostListeners.begin();
      i != fPostListeners.end(); ++i)
  {
    ISelectionListener::Pointer l = *i;
    if ((part && sel) || l.Cast<INullSelectionListener>())
    {
      try
      {
        l->SelectionChanged(part, sel);
      }
      catch (const ctkRuntimeException& rte)
      {
        WorkbenchPlugin::Log(rte);
      }
      catch (const std::exception& e)
      {
        WorkbenchPlugin::Log(e.what());
      }
    }
  }
}

AbstractPartSelectionTracker::Pointer AbstractSelectionService::GetPerPartTracker(
    const QString& partId)
{
  AbstractPartSelectionTracker::Pointer tracker;
  QHash<QString, AbstractPartSelectionTracker::Pointer>::const_iterator res = perPartTrackers.find(partId);
  if (res == perPartTrackers.end())
  {
    tracker = this->CreatePartTracker(partId);
    perPartTrackers[partId] = tracker;
  }
  else
  {
    tracker = res.value();
  }

  return tracker;
}

ISelection::ConstPointer AbstractSelectionService::GetSelection() const
{
  if (activeProvider.IsNotNull())
  {
    return activeProvider->GetSelection();
  }
  else
  {
    return ISelection::ConstPointer(0);
  }
}

ISelection::ConstPointer AbstractSelectionService::GetSelection(const QString& partId)
{
  return this->GetPerPartTracker(partId)->GetSelection();
}

void AbstractSelectionService::SetActivePart(IWorkbenchPart::Pointer newPart)
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
    ISelection::ConstPointer sel = activeProvider->GetSelection();
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
    this->FireSelection(IWorkbenchPart::Pointer(0), ISelection::ConstPointer(0));
    this->FirePostSelection(IWorkbenchPart::Pointer(0), ISelection::ConstPointer(0));
  }
}

}
