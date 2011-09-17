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
