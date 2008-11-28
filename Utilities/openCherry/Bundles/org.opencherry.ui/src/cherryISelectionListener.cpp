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

#include "cherryISelectionListener.h"

namespace cherry {

void
ISelectionListener::Events
::AddListener(ISelectionListener::Pointer listener)
{
  if (listener.IsNull()) return;

  this->selectionChanged += Delegate(listener.GetPointer(), &ISelectionListener::SelectionChanged);
}

void
ISelectionListener::Events
::RemoveListener(ISelectionListener::Pointer listener)
{
  if (listener.IsNull()) return;

  this->selectionChanged -= Delegate(listener.GetPointer(), &ISelectionListener::SelectionChanged);
}

}
