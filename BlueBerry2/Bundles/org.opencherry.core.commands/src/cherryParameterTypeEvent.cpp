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

#include "cherryParameterTypeEvent.h"
#include "cherryParameterType.h"

namespace cherry
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
