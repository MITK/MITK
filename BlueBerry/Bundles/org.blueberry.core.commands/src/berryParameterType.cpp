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

#include "berryParameterType.h"
#include "internal/berryCommandUtils.h"
#include "common/berryCommandExceptions.h"

#include "berryParameterTypeEvent.h"
#include "berryIParameterValueConverter.h"

#include <sstream>

namespace berry
{

void ParameterType::AddListener(const IParameterTypeListener::Pointer listener)
{
  parameterTypeEvents.AddListener(listener);
}

bool ParameterType::operator<(const Object* object) const
{
  const ParameterType* castedObject = dynamic_cast<const ParameterType*>(object);
  int compareTo = CommandUtils::Compare(defined, castedObject->defined);
  if (compareTo == 0)
  {
    compareTo = CommandUtils::Compare(id, castedObject->id);
  }
  return compareTo < 0;
}

void ParameterType::Define(
    const SmartPointer<IParameterValueConverter> parameterTypeConverter)
{

  const bool definedChanged = !this->defined;
  this->defined = true;

  this->parameterTypeConverter = parameterTypeConverter;

  ParameterTypeEvent::Pointer event(
      new ParameterTypeEvent(ParameterType::Pointer(this), definedChanged));
  this->FireParameterTypeChanged(event);
}

SmartPointer<IParameterValueConverter> ParameterType::GetValueConverter() const
{
  if (!this->IsDefined())
  {
    throw NotDefinedException(
        "Cannot use GetValueConverter() with an undefined ParameterType"); //$NON-NLS-1$
  }

  return parameterTypeConverter;
}

bool ParameterType::IsCompatible(const Object::ConstPointer value) const
{
  if (!this->IsDefined())
  {
    throw NotDefinedException(
        "Cannot use IsCompatible() with an undefined ParameterType"); //$NON-NLS-1$
  }
  return parameterTypeConverter->IsCompatible(value);
}

void ParameterType::RemoveListener(
    const IParameterTypeListener::Pointer listener)
{
  parameterTypeEvents.RemoveListener(listener);
}

std::string ParameterType::ToString() const
{
  if (str.empty())
  {
    std::stringstream stringBuffer;
    stringBuffer << "ParameterType(" << id << "," << defined << ")";
    str = stringBuffer.str();
  }
  return str;
}

void ParameterType::Undefine()
{
  str = "";

  const bool definedChanged = defined;
  defined = false;

  parameterTypeConverter = 0;

  ParameterTypeEvent::Pointer event(
      new ParameterTypeEvent(ParameterType::Pointer(this), definedChanged));
  this->FireParameterTypeChanged(event);
}

ParameterType::ParameterType(const std::string& id) :
  HandleObject(id)
{

}

void ParameterType::FireParameterTypeChanged(const SmartPointer<
    ParameterTypeEvent> event)
{
  if (!event)
  {
    throw Poco::NullPointerException("Cannot send a null event to listeners."); //$NON-NLS-1$
  }

  parameterTypeEvents.parameterTypeChanged(event);
}

}
