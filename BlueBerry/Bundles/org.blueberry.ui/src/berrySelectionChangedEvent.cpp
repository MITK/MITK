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
