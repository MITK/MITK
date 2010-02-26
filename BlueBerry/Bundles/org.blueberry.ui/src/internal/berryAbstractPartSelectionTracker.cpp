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

#include "berryAbstractPartSelectionTracker.h"

#include "../berryIPostSelectionProvider.h"
#include "../berryINullSelectionListener.h"

#include "../util/berrySafeRunnable.h"

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
    const std::string& partId)
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
  fListeners.remove(listener);
}

void AbstractPartSelectionTracker::RemovePostSelectionListener(
    ISelectionListener::Pointer listener)
{
  fPostListeners.remove(listener);
}

AbstractPartSelectionTracker::~AbstractPartSelectionTracker()
{

}

void AbstractPartSelectionTracker::FireSelection(IWorkbenchPart::Pointer part,
    ISelection::ConstPointer sel)
{
  SafeSelectionRunnable::Pointer runnable(new SafeSelectionRunnable(part, sel));
  for (std::list<ISelectionListener::Pointer>::iterator i = fListeners.begin();
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
  for (std::list<ISelectionListener::Pointer>::iterator i = fPostListeners.begin();
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

std::string AbstractPartSelectionTracker::GetPartId()
{
  return fPartId;
}

void AbstractPartSelectionTracker::SetPartId(const std::string& partId)
{
  fPartId = partId;
}

}
