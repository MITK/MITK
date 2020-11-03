/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryParameterTypeEvent.h"
#include "berryParameterType.h"

namespace berry
{

SmartPointer<ParameterType> ParameterTypeEvent::GetParameterType() const
{
  return parameterType;
}

ParameterTypeEvent::ParameterTypeEvent(
    const SmartPointer<ParameterType> parameterType, bool definedChanged) :
  AbstractHandleObjectEvent(definedChanged), parameterType(parameterType)
{
  if (!parameterType)
  {
    throw Poco::NullPointerException("ParameterType cannot be null");
  }
}

}
