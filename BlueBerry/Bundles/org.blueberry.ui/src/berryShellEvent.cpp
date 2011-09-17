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
