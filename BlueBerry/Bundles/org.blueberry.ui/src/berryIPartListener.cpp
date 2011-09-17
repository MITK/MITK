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

#include "berryIPartListener.h"

namespace berry {

void
IPartListener::Events
::AddListener(IPartListener::Pointer l)
{
  if (l.IsNull()) return;

  Types t = l->GetPartEventTypes();

  if (t & ACTIVATED)
    partActivated += Delegate(l.GetPointer(), &IPartListener::PartActivated);
  if (t & BROUGHT_TO_TOP)
    partBroughtToTop += Delegate(l.GetPointer(), &IPartListener::PartBroughtToTop);
  if (t & CLOSED)
    partClosed += Delegate(l.GetPointer(), &IPartListener::PartClosed);
  if (t & DEACTIVATED)
    partDeactivated += Delegate(l.GetPointer(), &IPartListener::PartDeactivated);
  if (t & OPENED)
    partOpened += Delegate(l.GetPointer(), &IPartListener::PartOpened);
  if (t & HIDDEN)
    partHidden += Delegate(l.GetPointer(), &IPartListener::PartHidden);
  if (t & VISIBLE)
    partVisible += Delegate(l.GetPointer(), &IPartListener::PartVisible);
  if (t & INPUT_CHANGED)
    partInputChanged += Delegate(l.GetPointer(), &IPartListener::PartInputChanged);
}

void
IPartListener::Events
::RemoveListener(IPartListener::Pointer l)
{
  if (l.IsNull()) return;

  partActivated -= Delegate(l.GetPointer(), &IPartListener::PartActivated);
  partBroughtToTop -= Delegate(l.GetPointer(), &IPartListener::PartBroughtToTop);
  partClosed -= Delegate(l.GetPointer(), &IPartListener::PartClosed);
  partDeactivated -= Delegate(l.GetPointer(), &IPartListener::PartDeactivated);
  partOpened -= Delegate(l.GetPointer(), &IPartListener::PartOpened);
  partHidden -= Delegate(l.GetPointer(), &IPartListener::PartHidden);
  partVisible -= Delegate(l.GetPointer(), &IPartListener::PartVisible);
  partInputChanged -= Delegate(l.GetPointer(), &IPartListener::PartInputChanged);
}

}
