/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIParameterTypeListener.h"

#include "berryParameterTypeEvent.h"
#include "berryParameterType.h"

namespace berry {

void
IParameterTypeListener::Events
::AddListener(IParameterTypeListener* l)
{
  if (l == nullptr) return;

  parameterTypeChanged += Delegate(l, &IParameterTypeListener::ParameterTypeChanged);
}

void
IParameterTypeListener::Events
::RemoveListener(IParameterTypeListener* l)
{
  if (l == nullptr) return;

  parameterTypeChanged -= Delegate(l, &IParameterTypeListener::ParameterTypeChanged);
}

IParameterTypeListener::~IParameterTypeListener()
{
}

}

