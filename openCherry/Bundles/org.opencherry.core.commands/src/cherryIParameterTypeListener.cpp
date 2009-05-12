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

#include "cherryIParameterTypeListener.h"

#include "cherryParameterTypeEvent.h"
#include "cherryParameterType.h"

namespace cherry {

void
IParameterTypeListener::Events
::AddListener(IParameterTypeListener::Pointer l)
{
  if (l.IsNull()) return;

  parameterTypeChanged += Delegate(l.GetPointer(), &IParameterTypeListener::ParameterTypeChanged);
}

void
IParameterTypeListener::Events
::RemoveListener(IParameterTypeListener::Pointer l)
{
  if (l.IsNull()) return;

  parameterTypeChanged -= Delegate(l.GetPointer(), &IParameterTypeListener::ParameterTypeChanged);
}

}

