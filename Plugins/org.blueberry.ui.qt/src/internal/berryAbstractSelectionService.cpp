/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  : selListener(new SelectionListener(this))
  , postSelListener(new PostSelectionListener(this))
{
}

AbstractSelectionService::SelectionListener::SelectionListener(AbstractSelectionService* service)
 : m_SelectionService(service)
{ }

void AbstractSelectionService::SelectionListener::SelectionChanged(const SelectionChangedEvent::Pointer& event)
{
  m_SelectionService->FireSelection(m_SelectionService->activePart, event->GetSelection());
}

AbstractSelectionService::PostSelectionListener::PostSelectionListener(AbstractSelectionService* service)
 : m_SelectionService(service)
{ }

void AbstractSelectionService::PostSelectionListener::SelectionChanged(
    const SelectionChangedEvent::Pointer& event)
{
  m_SelectionService->FirePostSelection(m_SelectionService->activePart, event->GetSelection());
}

void AbstractSelectionService::AddSelectionListener(ISelectionListener* l)
{
  fListeners.push_back(l);
}

void AbstractSelectionService::AddSelectionListener(const QString& partId,
                                                    ISelectionListener* listener)
{
  this->GetPerPartTracker(partId)->AddSelectionListener(listener);
}

void AbstractSelectionService::AddPostSelectionListener(ISelectionListener* l)
{
  fPostListeners.push_back(l);
}

void AbstractSelectionService::AddPostSelectionListener(const QString& partId,
                                                        ISelectionListener* listener)
{
  this->GetPerPartTracker(partId)->AddPostSelectionListener(listener);
}

void AbstractSelectionService::RemoveSelectionListener(ISelectionListener* l)
{
  fListeners.removeAll(l);
}

void AbstractSelectionService::RemovePostSelectionListener(
    const QString& partId, ISelectionListener* listener)
{
  this->GetPerPartTracker(partId)->RemovePostSelectionListener(listener);
}

void AbstractSelectionService::RemovePostSelectionListener(
    ISelectionListener* l)
{
  fPostListeners.removeAll(l);
}

void AbstractSelectionService::RemoveSelectionListener(const QString& partId,
                                                       ISelectionListener* listener)
{
  this->GetPerPartTracker(partId)->RemoveSelectionListener(listener);
}

void AbstractSelectionService::FireSelection(const IWorkbenchPart::Pointer& part,
                                             const ISelection::ConstPointer& sel)
{
  for (QList<ISelectionListener*>::iterator i = fListeners.begin();
      i != fListeners.end(); ++i)
  {
    ISelectionListener* l = *i;
    if ((part && sel) || dynamic_cast<INullSelectionListener*>(l))
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

void AbstractSelectionService::FirePostSelection(const IWorkbenchPart::Pointer& part,
                                                 const ISelection::ConstPointer& sel)
{
  for (QList<ISelectionListener*>::iterator i = fPostListeners.begin();
      i != fPostListeners.end(); ++i)
  {
    ISelectionListener* l = *i;
    if ((part && sel) || dynamic_cast<INullSelectionListener*>(l))
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
    return ISelection::ConstPointer(nullptr);
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
      newPart = nullptr;
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
      activeProvider->RemoveSelectionChangedListener(selListener.data());
      if (activeProvider.Cast<IPostSelectionProvider>().IsNotNull())
      {
        activeProvider.Cast<IPostSelectionProvider>()
        ->RemovePostSelectionChangedListener(postSelListener.data());
      }
      else
      {
        activeProvider
        ->RemoveSelectionChangedListener(postSelListener.data());
      }
      activeProvider = nullptr;
    }
    activePart = nullptr;
  }

  activePart = newPart;

  if (newPart.IsNotNull())
  {
    activeProvider = selectionProvider;
    // Fire an event if there's an active provider
    activeProvider->AddSelectionChangedListener(selListener.data());
    ISelection::ConstPointer sel = activeProvider->GetSelection();
    this->FireSelection(newPart, sel);
    if (activeProvider.Cast<IPostSelectionProvider>().IsNotNull())
    {
      activeProvider.Cast<IPostSelectionProvider>()
      ->AddPostSelectionChangedListener(postSelListener.data());
    }
    else
    {
      activeProvider->AddSelectionChangedListener(postSelListener.data());
    }
    this->FirePostSelection(newPart, sel);
  }
  else
  {
    this->FireSelection(IWorkbenchPart::Pointer(nullptr), ISelection::ConstPointer(nullptr));
    this->FirePostSelection(IWorkbenchPart::Pointer(nullptr), ISelection::ConstPointer(nullptr));
  }
}

}
