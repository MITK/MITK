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

#include "berryGuiTkIMenuListener.h"

namespace berry {

namespace GuiTk {

void
IMenuListener::Events
::AddListener(IMenuListener::Pointer l)
{
  if (l.IsNull()) return;

  menuAboutToShow += Delegate(l.GetPointer(), &IMenuListener::MenuAboutToShow);
  menuAboutToHide += Delegate(l.GetPointer(), &IMenuListener::MenuAboutToHide);
}

void
IMenuListener::Events
::RemoveListener(IMenuListener::Pointer l)
{
  if (l.IsNull()) return;

  menuAboutToShow -= Delegate(l.GetPointer(), &IMenuListener::MenuAboutToShow);
  menuAboutToHide -= Delegate(l.GetPointer(), &IMenuListener::MenuAboutToHide);
}

}

}
