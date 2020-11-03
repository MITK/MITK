/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryICommandCategoryListener.h"

#include "berryCommandCategory.h"
#include "berryCommandCategoryEvent.h"

namespace berry {

void
ICommandCategoryListener::Events
::AddListener(ICommandCategoryListener* l)
{
  if (l == nullptr) return;

  categoryChanged += Delegate(l, &ICommandCategoryListener::CategoryChanged);
}

void
ICommandCategoryListener::Events
::RemoveListener(ICommandCategoryListener* l)
{
  if (l == nullptr) return;

  categoryChanged -= Delegate(l, &ICommandCategoryListener::CategoryChanged);
}

ICommandCategoryListener::~ICommandCategoryListener()
{
}

}


