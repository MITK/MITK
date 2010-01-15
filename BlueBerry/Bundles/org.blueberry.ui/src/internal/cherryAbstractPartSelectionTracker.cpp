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

#include "cherryAbstractPartSelectionTracker.h"

#include "../cherryIPostSelectionProvider.h"

#include "../util/cherrySafeRunnable.h"

#include <cherrySafeRunner.h>

namespace cherry
{

class SafeSelectionRunnable : public SafeRunnable
{
public:

  cherryObjectMacro(SafeSelectionRunnable)

  ISelectionService::SelectionEvents::SelectionEvent::AbstractDelegate* l;

  SafeSelectionRunnable(IWorkbenchPart::Pointer part, ISelection::ConstPointer sel)
  : p(part), s(sel)
  {}

  void Run()
  {
    l->Execute(p, s);
  }

private:

  IWorkbenchPart::Pointer p;
  ISelection::ConstPointer s;
};

AbstractPartSelectionTracker::AbstractPartSelectionTracker(
    const std::string& partId)
{
  this->SetPartId(partId);
}


void AbstractPartSelectionTracker::AddSelectionListener(
    ISelectionListener::Pointer listener)
{
  selectionEvents.selectionChanged +=
    ISelectionService::SelectionEvents::Delegate(listener.GetPointer(), &ISelectionListener::SelectionChanged);
}

void AbstractPartSelectionTracker::AddPostSelectionListener(
    ISelectionListener::Pointer listener)
{
  selectionEvents.postSelectionChanged +=
    ISelectionService::SelectionEvents::Delegate(listener.GetPointer(), &ISelectionListener::SelectionChanged);
}

void AbstractPartSelectionTracker::RemoveSelectionListener(
    ISelectionListener::Pointer listener)
{
  selectionEvents.selectionChanged -=
    ISelectionService::SelectionEvents::Delegate(listener.GetPointer(), &ISelectionListener::SelectionChanged);
}

void AbstractPartSelectionTracker::RemovePostSelectionListener(
    ISelectionListener::Pointer listener)
{
  selectionEvents.postSelectionChanged -=
    ISelectionService::SelectionEvents::Delegate(listener.GetPointer(), &ISelectionListener::SelectionChanged);
}

AbstractPartSelectionTracker::~AbstractPartSelectionTracker()
{

}

void AbstractPartSelectionTracker::FireSelection(IWorkbenchPart::Pointer part,
    ISelection::ConstPointer sel)
{
  typedef ISelectionService::SelectionEvents::SelectionEvent::ListenerList ListType;
  ListType listeners(selectionEvents.selectionChanged.GetListeners());

  SafeSelectionRunnable::Pointer runnable(new SafeSelectionRunnable(part, sel));
  for (ListType::iterator i = listeners.begin(); i != listeners.end(); ++i)
  {
    runnable->l = *i;
    SafeRunner::Run(runnable);
  }
}

void AbstractPartSelectionTracker::FirePostSelection(IWorkbenchPart::Pointer part,
    ISelection::ConstPointer sel)
{
  typedef ISelectionService::SelectionEvents::SelectionEvent::ListenerList ListType;
  ListType listeners(selectionEvents.postSelectionChanged.GetListeners());

  SafeSelectionRunnable::Pointer runnable(new SafeSelectionRunnable(part, sel));
  for (ListType::iterator i = listeners.begin(); i != listeners.end(); ++i)
  {
    runnable->l = *i;
    SafeRunner::Run(runnable);
  }
}

std::string AbstractPartSelectionTracker::GetPartId()
{
  return fPartId;
}

void AbstractPartSelectionTracker::SetPartId(const std::string& partId)
{
  fPartId = partId;
}

}
