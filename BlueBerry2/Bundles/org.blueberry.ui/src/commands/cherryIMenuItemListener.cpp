/*=========================================================================

 Program:   openCherry Platform
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

#include "cherryIMenuItemListener.h"
#include "cherryIMenuItem.h"

namespace cherry {

void
IMenuItemListener::Events
::AddListener(IMenuItemListener::Pointer l)
{
  if (l.IsNull()) return;

  menuItemDisposed += DisposedDelegate(l.GetPointer(), &IMenuItemListener::MenuItemDisposed);
  menuItemSelected += SelectedDelegate(l.GetPointer(), &IMenuItemListener::MenuItemSelected);
}

void
IMenuItemListener::Events
::RemoveListener(IMenuItemListener::Pointer l)
{
  if (l.IsNull()) return;

  menuItemDisposed -= DisposedDelegate(l.GetPointer(), &IMenuItemListener::MenuItemDisposed);
  menuItemSelected -= SelectedDelegate(l.GetPointer(), &IMenuItemListener::MenuItemSelected);
}

}
