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

#include "berryAbstractNamedHandleEvent.h"

namespace berry
{

int AbstractNamedHandleEvent::CHANGED_DESCRIPTION() { static int i = AbstractNamedHandleEvent::LAST_BIT_USED_ABSTRACT_HANDLE() << 1; return i; }
int AbstractNamedHandleEvent::CHANGED_NAME() { static int i = AbstractNamedHandleEvent::LAST_BIT_USED_ABSTRACT_HANDLE() << 2; return i; }
int AbstractNamedHandleEvent::LAST_USED_BIT() { static int i = AbstractNamedHandleEvent::CHANGED_NAME(); return i; }

bool AbstractNamedHandleEvent::IsDescriptionChanged() const
{
  return ((changedValues & CHANGED_DESCRIPTION()) != 0);
}

bool AbstractNamedHandleEvent::IsNameChanged() const
{
  return ((changedValues & CHANGED_NAME()) != 0);
}

AbstractNamedHandleEvent::AbstractNamedHandleEvent(bool definedChanged,
    bool descriptionChanged, bool nameChanged) :
  AbstractHandleObjectEvent(definedChanged)
{
  if (descriptionChanged)
  {
    changedValues |= CHANGED_DESCRIPTION();
  }
  if (nameChanged)
  {
    changedValues |= CHANGED_NAME();
  }
}

}

