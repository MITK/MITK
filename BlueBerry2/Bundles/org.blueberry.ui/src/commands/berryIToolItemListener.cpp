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


#include "berryIToolItemListener.h"
#include "berryIToolItem.h"

namespace berry {

void
IToolItemListener::Events
::AddListener(IToolItemListener::Pointer l)
{
  if (l.IsNull()) return;

  toolItemDisposed += DisposedDelegate(l.GetPointer(), &IToolItemListener::ToolItemDisposed);
  toolItemSelected += SelectedDelegate(l.GetPointer(), &IToolItemListener::ToolItemSelected);
}

void
IToolItemListener::Events
::RemoveListener(IToolItemListener::Pointer l)
{
  if (l.IsNull()) return;

  toolItemDisposed -= DisposedDelegate(l.GetPointer(), &IToolItemListener::ToolItemDisposed);
  toolItemSelected -= SelectedDelegate(l.GetPointer(), &IToolItemListener::ToolItemSelected);
}

}
