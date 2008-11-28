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

#include "cherryGuiTkISelectionListener.h"

namespace cherry {

namespace GuiTk {

void
ISelectionListener::Events
::AddListener(ISelectionListener::Pointer l)
{
  if (l.IsNull()) return;

  selected += Delegate(l.GetPointer(), &ISelectionListener::WidgetSelected);
  defaultSelected += Delegate(l.GetPointer(), &ISelectionListener::WidgetDefaultSelected);
}

void
ISelectionListener::Events
::RemoveListener(ISelectionListener::Pointer l)
{
  if (l.IsNull()) return;

  selected -= Delegate(l.GetPointer(), &ISelectionListener::WidgetSelected);
  defaultSelected -= Delegate(l.GetPointer(), &ISelectionListener::WidgetDefaultSelected);
}

}

}
