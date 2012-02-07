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

#include "berryStackPresentation.h"

#include "../berryConstants.h"

namespace berry
{

const int StackPresentation::AS_INACTIVE = 0;
const int StackPresentation::AS_ACTIVE_FOCUS = 1;
const int StackPresentation::AS_ACTIVE_NOFOCUS = 2;


StackPresentation::StackPresentation(IStackPresentationSite::Pointer stackSite)
 : site(stackSite)
{
  poco_assert(stackSite.IsNotNull());
}

StackPresentation::~StackPresentation()
{
  // Do not remove empty destructor. Otherwise, the WeakPointer member "site"
  // will be destroyed in a strange way and the destruction of a DefaultStackPresentationSite
  // instance will notify the WeakPointer "site" (although it was destroyed, the
  // DefaultStackPresentationSite somehow still has a MessageDelegate for the destroy
  // event, pointing to the destroyed "site").
}

IStackPresentationSite::Pointer StackPresentation::GetSite()
{
  return site.Lock();
}

Point StackPresentation::ComputeMinimumSize()
{
  return Point(0, 0);
}

int StackPresentation::GetSizeFlags(bool  /*width*/)
{
  bool hasMaximumSize = this->GetSite()->GetState()
      == IStackPresentationSite::STATE_MINIMIZED;

  return Constants::MIN | (hasMaximumSize ? Constants::MAX : 0);
}

int StackPresentation::ComputePreferredSize(bool width, int /*availableParallel*/,
    int  /*availablePerpendicular*/, int preferredResult)
{
  Point p = this->ComputeMinimumSize();
  int minSize = width ? p.x : p.y;

  if (this->GetSite()->GetState() == IStackPresentationSite::STATE_MINIMIZED
      || preferredResult < minSize)
  {
    return minSize;
  }

  return preferredResult;
}

void StackPresentation::MovePart(IPresentablePart::Pointer toMove,
    Object::Pointer cookie)
{
  this->RemovePart(toMove);
  this->AddPart(toMove, cookie);

  if (this->GetSite()->GetSelectedPart() == toMove)
  {
    this->SelectPart(toMove);
    toMove->SetFocus();
  }
}

void StackPresentation::SaveState(IPresentationSerializer*  /*context*/, IMemento::Pointer  /*memento*/)
{

}

void StackPresentation::RestoreState(IPresentationSerializer*  /*context*/, IMemento::Pointer  /*memento*/)
{

}

void StackPresentation::ShowPartList()
{

}

}
