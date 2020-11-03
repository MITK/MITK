/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryParameterType.h"
#include "internal/berryCommandUtils.h"
#include "common/berryCommandExceptions.h"

#include "berryParameterTypeEvent.h"
#include "berryIParameterValueConverter.h"

#include <sstream>

namespace berry
{

void ParameterType::AddListener(IParameterTypeListener* listener)
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

void ParameterType::Define(const QString&  type,
                           const QSharedPointer<IParameterValueConverter>& parameterTypeConverter)
{
  const bool definedChanged = !this->defined;
  this->defined = true;

  this->type = type.isNull() ? QObject::staticMetaObject.className() : type;
  this->parameterTypeConverter = parameterTypeConverter;

  ParameterTypeEvent::Pointer event(
      new ParameterTypeEvent(ParameterType::Pointer(this), definedChanged));
  this->FireParameterTypeChanged(event);
}

IParameterValueConverter* ParameterType::GetValueConverter() const
{
  if (!this->IsDefined())
  {
    throw NotDefinedException(
        "Cannot use GetValueConverter() with an undefined ParameterType"); //$NON-NLS-1$
  }

  return parameterTypeConverter.data();
}

bool ParameterType::IsCompatible(const QObject* const value) const
{
  if (!this->IsDefined())
  {
    throw NotDefinedException(
        "Cannot use IsCompatible() with an undefined ParameterType");
  }
  return value->inherits(qPrintable(type));
}

void ParameterType::RemoveListener(IParameterTypeListener* listener)
{
  parameterTypeEvents.RemoveListener(listener);
}

QString ParameterType::ToString() const
{
  if (str.isEmpty())
  {
    QTextStream stringBuffer(&str);
    stringBuffer << "ParameterType(" << id << "," << defined << ")";
  }
  return str;
}

void ParameterType::Undefine()
{
  str = "";

  const bool definedChanged = defined;
  defined = false;

  parameterTypeConverter.clear();

  ParameterTypeEvent::Pointer event(
      new ParameterTypeEvent(ParameterType::Pointer(this), definedChanged));
  this->FireParameterTypeChanged(event);
}

ParameterType::ParameterType(const QString& id) :
  HandleObject(id)
{

}

void ParameterType::FireParameterTypeChanged(const SmartPointer<
    ParameterTypeEvent> event)
{
  if (!event)
  {
    throw ctkInvalidArgumentException("Cannot send a null event to listeners.");
  }

  parameterTypeEvents.parameterTypeChanged(event);
}

}
