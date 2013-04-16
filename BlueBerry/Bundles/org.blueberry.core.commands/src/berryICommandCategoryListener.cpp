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

#include "berryICommandCategoryListener.h"

#include "berryCommandCategory.h"
#include "berryCommandCategoryEvent.h"

namespace berry {

void
ICommandCategoryListener::Events
::AddListener(ICommandCategoryListener::Pointer l)
{
  if (l.IsNull()) return;

  categoryChanged += Delegate(l.GetPointer(), &ICommandCategoryListener::CategoryChanged);
}

void
ICommandCategoryListener::Events
::RemoveListener(ICommandCategoryListener::Pointer l)
{
  if (l.IsNull()) return;

  categoryChanged -= Delegate(l.GetPointer(), &ICommandCategoryListener::CategoryChanged);
}

}


