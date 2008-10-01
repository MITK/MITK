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

#include "cherrySelectionChangedEvent.h"
#include "cherryISelectionProvider.h"


namespace cherry
{

SelectionChangedEvent::SelectionChangedEvent(
    ISelectionProvider::Pointer source, ISelection::Pointer selection)
{
  poco_assert(source.IsNotNull());
  poco_assert(selection.IsNotNull());

  this->source = source;
  this->selection = selection;
}

ISelectionProvider::Pointer SelectionChangedEvent::GetSource()
{
  return source;
}

ISelection::Pointer SelectionChangedEvent::GetSelection()
{
  return selection;
}

ISelectionProvider::Pointer SelectionChangedEvent::GetSelectionProvider()
{
  return this->GetSource();
}

}
