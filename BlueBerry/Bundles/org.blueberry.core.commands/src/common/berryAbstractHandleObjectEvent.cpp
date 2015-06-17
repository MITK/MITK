/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
