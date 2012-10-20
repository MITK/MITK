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

  ISelectionListener* l;

  SafeSelectionRunnable(const IWorkbenchPart::Pointer& part,
                        const ISelection::ConstPointer& sel)
  : p(part), s(sel)
  {}

  void Run()
  {
    l->SelectionChanged(p, s);
  }

private:

  const IWorkbenchPart::Pointer p;
  const ISelection::ConstPointer s;
};

AbstractPartSelectionTracker::AbstractPartSelectionTracker(
    const QString& partId)
{
  this->SetPartId(partId);
}


void AbstractPartSelectionTracker::AddSelectionListener(
    ISelectionListener* listener)
{
  fListeners.push_back(listener);
}

void AbstractPartSelectionTracker::AddPostSelectionListener(
    ISelectionListener* listener)
{
  fPostListeners.push_back(listener);
}

void AbstractPartSelectionTracker::RemoveSelectionListener(
    ISelectionListener* listener)
{
  fListeners.removeAll(listener);
}

void AbstractPartSelectionTracker::RemovePostSelectionListener(
    ISelectionListener* listener)
{
  fPostListeners.removeAll(listener);
}

AbstractPartSelectionTracker::~AbstractPartSelectionTracker()
{

}

void AbstractPartSelectionTracker::FireSelection(const IWorkbenchPart::Pointer& part,
                                                 const ISelection::ConstPointer& sel)
{
  SafeSelectionRunnable::Pointer runnable(new SafeSelectionRunnable(part, sel));
  for (QList<ISelectionListener*>::iterator i = fListeners.begin();
      i != fListeners.end(); ++i)
  {
    ISelectionListener* l = *i;
    if ((part && sel) || dynamic_cast<INullSelectionListener*>(l))
    {
      runnable->l = l;
      SafeRunner::Run(runnable);
    }
  }
}

void AbstractPartSelectionTracker::FirePostSelection(const IWorkbenchPart::Pointer& part,
                                                     const ISelection::ConstPointer& sel)
{
  SafeSelectionRunnable::Pointer runnable(new SafeSelectionRunnable(part, sel));
  for (QList<ISelectionListener*>::iterator i = fPostListeners.begin();
      i != fPostListeners.end(); ++i)
  {
    ISelectionListener* l = *i;
    if ((part && sel) || dynamic_cast<INullSelectionListener*>(l))
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
