/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryAbstractHandleObjectEvent.h"

namespace berry
{

int AbstractHandleObjectEvent::CHANGED_DEFINED() { static int i = 1; return i; }
int AbstractHandleObjectEvent::LAST_BIT_USED_ABSTRACT_HANDLE() { return CHANGED_DEFINED(); }

AbstractHandleObjectEvent::AbstractHandleObjectEvent(bool definedChanged)
{
  if (definedChanged)
  {
    changedValues |= CHANGED_DEFINED();
  }
}

bool AbstractHandleObjectEvent::IsDefinedChanged() const
{
  return ((changedValues & CHANGED_DEFINED()) != 0);
}

}
