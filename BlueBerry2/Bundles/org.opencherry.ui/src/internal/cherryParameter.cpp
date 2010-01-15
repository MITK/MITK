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

#include "cherryParameter.h"

#include <Poco/Hash.h>

#include <sstream>

namespace cherry
{

const std::string Parameter::ATTRIBUTE_VALUES = "values"; //$NON-NLS-1$
const std::size_t Parameter::HASH_CODE_NOT_COMPUTED = 0;
const std::size_t Parameter::HASH_FACTOR = 89;
const std::size_t Parameter::HASH_INITIAL = Poco::hash(
    HandleObject::GetStaticClassName());

Parameter::Parameter(const std::string& id, const std::string& name,
    const IConfigurationElement::Pointer values,
    const ParameterType::Pointer parameterType, const bool optional) :
  hashCode(HASH_CODE_NOT_COMPUTED), name(name), optional(optional),
      parameterType(parameterType), valuesConfigurationElement(values), id(id)
{
  if (id.empty())
  {
    throw Poco::NullPointerException("Cannot create a parameter with a null id"); //$NON-NLS-1$
  }

  if (name.empty())
  {
    throw Poco::NullPointerException("The name of a parameter cannot be null."); //$NON-NLS-1$
  }

  if (!values)
  {
    throw Poco::NullPointerException(
        "The values for a parameter cannot be null."); //$NON-NLS-1$
  }

}

bool Parameter::operator==(const Object* object) const
{
  if (this == object)
  {
    return true;
  }

  if (const Parameter * const parameter = dynamic_cast<const Parameter*>(object))
  {

    if (id != parameter->id)
    {
      return false;
    }
    if (name != parameter->name)
    {
      return false;
    }
    if (!(values.IsNull() ? parameter->values.IsNull() : (parameter->values
        && values == parameter->values)))
    {
      return false;
    }

    return optional == parameter->optional;
  }

  return false;
}

std::string Parameter::GetId() const
{
  return id;
}

std::string Parameter::GetName() const
{
  return name;
}

ParameterType::Pointer Parameter::GetParameterType() const
{
  return parameterType;
}

IParameterValues::Pointer Parameter::GetValues() const
    throw(ParameterValuesException)
{
  if (!values)
  {
    try
    {
      values = valuesConfigurationElement->CreateExecutableExtension<
          IParameterValues> (ATTRIBUTE_VALUES);
    } catch (const CoreException& e)
    {
      throw ParameterValuesException("Problem creating parameter values", e);
    }

    if (!values)
    {
      throw ParameterValuesException(
          "Parameter values were not an instance of IParameterValues");
    }
  }

  return values;
}

std::size_t Parameter::HashCode() const
{
  if (hashCode == HASH_CODE_NOT_COMPUTED)
  {
    hashCode = HASH_INITIAL * HASH_FACTOR + Poco::hash(id);
    if (hashCode == HASH_CODE_NOT_COMPUTED)
    {
      hashCode++;
    }
  }
  return hashCode;
}

bool Parameter::IsOptional() const
{
  return optional;
}

std::string Parameter::ToString() const
{
  if (str.empty())
  {
    std::stringstream buffer;

    buffer << "Parameter(" << id << "," << name << "," << values << ","
        << optional << ")";

    str = buffer.str();
  }

  return str;
}

}
