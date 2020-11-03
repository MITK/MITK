/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "tweaklets/berryGuiWidgetsTweaklet.h"

#include "berrySameShellProvider.h"

namespace berry
{

SameShellProvider::SameShellProvider(QWidget* target) :
  targetControl(target)
{

}

SameShellProvider::SameShellProvider(Shell::Pointer sh) :
  targetControl(nullptr), shell(sh)
{

}

Shell::Pointer SameShellProvider::GetShell() const
{
  if (shell != 0)
  {
    return shell;
  }

  return targetControl == nullptr ? Shell::Pointer(nullptr)
      : Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetShell(targetControl);
}

}
