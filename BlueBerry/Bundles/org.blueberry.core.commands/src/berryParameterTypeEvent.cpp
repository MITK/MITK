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
