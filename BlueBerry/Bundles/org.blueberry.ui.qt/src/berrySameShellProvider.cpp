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

#include "berrySameShellProvider.h"

#include "tweaklets/berryGuiWidgetsTweaklet.h"

namespace berry
{

SameShellProvider::SameShellProvider(void* target) :
  targetControl(target)
{

}

SameShellProvider::SameShellProvider(Shell::Pointer sh) :
  targetControl(0), shell(sh)
{

}

Shell::Pointer SameShellProvider::GetShell()
{
  if (shell != 0)
  {
    return shell;
  }

  return targetControl == 0 ? Shell::Pointer(0)
      : Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetShell(targetControl);
}

}
