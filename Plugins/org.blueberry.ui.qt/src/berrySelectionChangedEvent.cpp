/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berrySelectionChangedEvent.h"
#include "berryISelectionProvider.h"


namespace berry
{

SelectionChangedEvent::SelectionChangedEvent(
    ISelectionProvider::Pointer source, ISelection::ConstPointer selection)
{
  poco_assert(source.IsNotNull());
  poco_assert(selection.IsNotNull());

  this->source = source;
  this->selection = selection;
}

ISelectionProvider::Pointer SelectionChangedEvent::GetSource() const
{
  return source;
}

ISelection::ConstPointer SelectionChangedEvent::GetSelection() const
{
  return selection;
}

ISelectionProvider::Pointer SelectionChangedEvent::GetSelectionProvider() const
{
  return this->GetSource();
}

}
