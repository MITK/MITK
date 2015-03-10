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

#include "berryIParameterTypeListener.h"

#include "berryParameterTypeEvent.h"
#include "berryParameterType.h"

namespace berry {

void
IParameterTypeListener::Events
::AddListener(IParameterTypeListener* l)
{
  if (l == 0) return;

  parameterTypeChanged += Delegate(l, &IParameterTypeListener::ParameterTypeChanged);
}

void
IParameterTypeListener::Events
::RemoveListener(IParameterTypeListener* l)
{
  if (l == 0) return;

  parameterTypeChanged -= Delegate(l, &IParameterTypeListener::ParameterTypeChanged);
}

IParameterTypeListener::~IParameterTypeListener()
{
}

}

