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

#include "berryAbstractPartSelectionTracker.h"

#include "berryIPostSelectionProvider.h"
#include "berryINullSelectionListener.h"

#include "util/berrySafeRunnable.h"

#include <berrySafeRunner.h>

namespace berry
{

class SafeSelectionRunnable : public SafeRunnable
{
public:

  berryObjectMacro(SafeSelectionRunnable)

  ISelectionListener::Pointer l;

  SafeSelectionRunnable(IWorkbenchPart::Pointer part, ISelection::ConstPointer sel)
  : p(part), s(sel)
  {}

  void Run()
  {
    l->SelectionChanged(p, s);
  }

private:

  IWorkbenchPart::Pointer p;
  ISelection::ConstPointer s;
};

AbstractPartSelectionTracker::AbstractPartSelectionTracker(
    const QString& partId)
{
  this->SetPartId(partId);
}


void AbstractPartSelectionTracker::AddSelectionListener(
    ISelectionListener::Pointer listener)
{
  fListeners.push_back(listener);
}

void AbstractPartSelectionTracker::AddPostSelectionListener(
    ISelectionListener::Pointer listener)
{
  fPostListeners.push_back(listener);
}

void AbstractPartSelectionTracker::RemoveSelectionListener(
    ISelectionListener::Pointer listener)
{
  fListeners.removeAll(listener);
}

void AbstractPartSelectionTracker::RemovePostSelectionListener(
    ISelectionListener::Pointer listener)
{
  fPostListeners.removeAll(listener);
}

AbstractPartSelectionTracker::~AbstractPartSelectionTracker()
{

}

void AbstractPartSelectionTracker::FireSelection(IWorkbenchPart::Pointer part,
    ISelection::ConstPointer sel)
{
  SafeSelectionRunnable::Pointer runnable(new SafeSelectionRunnable(part, sel));
  for (QList<ISelectionListener::Pointer>::iterator i = fListeners.begin();
      i != fListeners.end(); ++i)
  {
    ISelectionListener::Pointer l = *i;
    if ((part && sel) || l.Cast<INullSelectionListener>())
    {
      runnable->l = l;
      SafeRunner::Run(runnable);
    }
  }
}

void AbstractPartSelectionTracker::FirePostSelection(IWorkbenchPart::Pointer part,
    ISelection::ConstPointer sel)
{
  SafeSelectionRunnable::Pointer runnable(new SafeSelectionRunnable(part, sel));
  for (QList<ISelectionListener::Pointer>::iterator i = fPostListeners.begin();
      i != fPostListeners.end(); ++i)
  {
    ISelectionListener::Pointer l = *i;
    if ((part && sel) || l.Cast<INullSelectionListener>())
    {
      runnable->l = l;
      SafeRunner::Run(runnable);
    }
  }
}

QString AbstractPartSelectionTracker::GetPartId()
{
  return fPartId;
}

void AbstractPartSelectionTracker::SetPartId(const QString& partId)
{
  fPartId = partId;
}

}
