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

#include "berryICommandManagerListener.h"

#include "berryCommandManagerEvent.h"

namespace berry
{

void ICommandManagerListener::Events::AddListener(
    ICommandManagerListener::Pointer l)
{
  if (l.IsNull())
    return;

  commandManagerChanged += Delegate(l.GetPointer(),
      &ICommandManagerListener::CommandManagerChanged);
}

void ICommandManagerListener::Events::RemoveListener(
    ICommandManagerListener::Pointer l)
{
  if (l.IsNull())
    return;

  commandManagerChanged -= Delegate(l.GetPointer(),
      &ICommandManagerListener::CommandManagerChanged);
}

}

