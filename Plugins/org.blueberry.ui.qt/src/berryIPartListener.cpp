/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIPartListener.h"

namespace berry {

void
IPartListener::Events
::AddListener(IPartListener* l)
{
  if (l == nullptr) return;

  Types t = l->GetPartEventTypes();

  if (t & ACTIVATED)
    partActivated += Delegate(l, &IPartListener::PartActivated);
  if (t & BROUGHT_TO_TOP)
    partBroughtToTop += Delegate(l, &IPartListener::PartBroughtToTop);
  if (t & CLOSED)
    partClosed += Delegate(l, &IPartListener::PartClosed);
  if (t & DEACTIVATED)
    partDeactivated += Delegate(l, &IPartListener::PartDeactivated);
  if (t & OPENED)
    partOpened += Delegate(l, &IPartListener::PartOpened);
  if (t & HIDDEN)
    partHidden += Delegate(l, &IPartListener::PartHidden);
  if (t & VISIBLE)
    partVisible += Delegate(l, &IPartListener::PartVisible);
  if (t & INPUT_CHANGED)
    partInputChanged += Delegate(l, &IPartListener::PartInputChanged);
}

void
IPartListener::Events
::RemoveListener(IPartListener* l)
{
  if (l == nullptr) return;

  partActivated -= Delegate(l, &IPartListener::PartActivated);
  partBroughtToTop -= Delegate(l, &IPartListener::PartBroughtToTop);
  partClosed -= Delegate(l, &IPartListener::PartClosed);
  partDeactivated -= Delegate(l, &IPartListener::PartDeactivated);
  partOpened -= Delegate(l, &IPartListener::PartOpened);
  partHidden -= Delegate(l, &IPartListener::PartHidden);
  partVisible -= Delegate(l, &IPartListener::PartVisible);
  partInputChanged -= Delegate(l, &IPartListener::PartInputChanged);
}

IPartListener::~IPartListener()
{
}

}
