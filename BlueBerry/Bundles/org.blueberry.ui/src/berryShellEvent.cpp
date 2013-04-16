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
