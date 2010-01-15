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

#include "cherryICommandManagerListener.h"

#include "cherryCommandManagerEvent.h"

namespace cherry
{

void ICommandManagerListener::Events::AddListener(
    ICommandManagerListener::Pointer l)
{
  if (l.IsNull())
    return;

  commandManagerChanged += Delegate(l.GetPointer(),
      &ICommandManagerListener::CommandManagerChanged);
}

void ICommandManagerListener::Events::RemoveListener(
    ICommandManagerListener::Pointer l)
{
  if (l.IsNull())
    return;

  commandManagerChanged -= Delegate(l.GetPointer(),
      &ICommandManagerListener::CommandManagerChanged);
}

}

