/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryShellEvent.h"

#include "berryShell.h"

namespace berry
{

ShellEvent::ShellEvent(Shell::Pointer src) :
  doit(true), source(src)
{

}

Shell::Pointer ShellEvent::GetSource()
{
  return source;
}

}
