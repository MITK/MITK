/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCommandParameter.h"

#include "berryIConfigurationElement.h"
#include "berryIParameterValues.h"
#include "berryCoreException.h"
#include "berryCommandExceptions.h"
#include "berryParameterType.h"

namespace berry {

const QString CommandParameter::ATTRIBUTE_VALUES = "values";
const int CommandParameter::HASH_CODE_NOT_COMPUTED = 0;
const int CommandParameter::HASH_FACTOR = 89;
const int CommandParameter::HASH_INITIAL = qHash(CommandParameter::GetStaticClassName());

CommandParameter::CommandParameter(const QString& id, const QString& name,
                 const SmartPointer<IConfigurationElement>& values,
                 const SmartPointer<ParameterType>& parameterType,
                 const bool optional)
  : name(name)
  , optional(optional)
  , parameterType(parameterType)
  , valuesConfigurationElement(values)
  , id(id)
{
  if (id.isNull())
  {
    throw ctkInvalidArgumentException("Cannot create a parameter with a null id");
  }

  if (name.isNull())
  {
    throw ctkInvalidArgumentException("The name of a parameter cannot be null.");
  }

  if (values.IsNull())
  {
    throw ctkInvalidArgumentException("The values for a parameter cannot be null.");
  }
}

bool CommandParameter::operator==(const Object* object) const
{
  if (this == object)
  {
    return true;
  }

  if (const CommandParameter* parameter = dynamic_cast<const CommandParameter*>(object))
  {
    if (id != parameter->id)
    {
      return false;
    }
    if (name != parameter->name)
    {
      return false;
    }
    if (values != parameter->values)
    {
      return false;
    }

    return optional == parameter->optional;
  }
  else
  {
    return false;
  }
}

QString CommandParameter::GetId() const
{
  return id;
}

QString CommandParameter::GetName() const
{
  return name;
}

SmartPointer<ParameterType> CommandParameter::GetParameterType() const
{
  return parameterType;
}

IParameterValues* CommandParameter::GetValues() const
{
  if (values.isNull())
  {
    try
    {
      values.reset(valuesConfigurationElement->CreateExecutableExtension<IParameterValues>(ATTRIBUTE_VALUES));
    }
    catch (const CoreException& e)
    {
      throw ParameterValuesException("Problem creating parameter values", e);
    }

    if (values.isNull())
    {
      throw ParameterValuesException(
          "Parameter values were not an instance of IParameterValues");
    }
  }

  return values.data();
}

bool CommandParameter::IsOptional() const
{
  return optional;
}

QString CommandParameter::ToString() const
{
  if (str.isNull())
  {
    QString paramValuesStr;
    if (!values.isNull())
    {
      QDebug dbg(&paramValuesStr);
      dbg << values->GetParameterValues();
    }
    str = "Parameter(" + id + ',' + name + ',' + paramValuesStr + ',' + optional +')';
  }

  return str;
}

}
