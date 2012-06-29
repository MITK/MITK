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

#include "berryIDragOverListener.h"

namespace berry {

void
IDragOverListener::Events
::AddListener(IDragOverListener::Pointer l)
{
  if (l.IsNull()) return;

  drag += DragDelegate(l.GetPointer(), &IDragOverListener::Drag);

}

void
IDragOverListener::Events
::RemoveListener(IDragOverListener::Pointer l)
{
  if (l.IsNull()) return;

  drag -= DragDelegate(l.GetPointer(), &IDragOverListener::Drag);

}

}
