/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIDragOverListener.h"

namespace berry {

void IDragOverListener::Events::AddListener(IDragOverListener* l)
{
  if (l == nullptr) return;

  drag += DragDelegate(l, &IDragOverListener::Drag);

}

void IDragOverListener::Events::RemoveListener(IDragOverListener* l)
{
  if (l == nullptr) return;

  drag -= DragDelegate(l, &IDragOverListener::Drag);

}

IDragOverListener::~IDragOverListener()
{
}

}
