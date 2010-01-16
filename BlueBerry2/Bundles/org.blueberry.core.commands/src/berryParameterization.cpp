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

#include "berryParameterization.h"
#include "berryIParameter.h"

#include <Poco/Hash.h>

namespace berry
{

const std::size_t Parameterization::HASH_CODE_NOT_COMPUTED = 0;
const std::size_t Parameterization::HASH_FACTOR = 89;
const std::size_t Parameterization::HASH_INITIAL = Poco::hash("berry::Parameterization");

Parameterization::Parameterization(const SmartPointer<const IParameter> parameter, const std::string& value)
: hashCode(HASH_CODE_NOT_COMPUTED), parameter(parameter), value(value)
{
  if (!parameter)
  {
    throw Poco::NullPointerException(
        "You cannot parameterize a null parameter");
  }
}

Parameterization::Parameterization(const Parameterization& p)
: hashCode(p.hashCode), parameter(p.parameter), value(p.value)
{

}

Parameterization& Parameterization::operator=(const Parameterization& p)
{
  this->hashCode = p.hashCode;
  this->parameter = p.parameter;
  this->value = p.value;

  return *this;
}

bool Parameterization::operator==(const Parameterization& parameterization) const
{
  if (this == &parameterization)
  {
    return true;
  }

  //    if (!(object instanceof Parameterization)) {
  //      return false;
  //    }

  if (this->parameter->GetId() != parameterization.parameter->GetId())
  {
    return false;
  }

  return (this->value == parameterization.value);
}

Parameterization::operator bool() const
{
  return true;
}

SmartPointer<const IParameter> Parameterization::GetParameter() const
{
  return parameter;
}

std::string Parameterization::GetValue() const
{
  return value;
}

std::string Parameterization::GetValueName() const throw(ParameterValuesException)
{
  const IParameterValues::ParameterValuesMap parameterValues = parameter->GetValues()->GetParameterValues();

  std::string returnValue;
  for (IParameterValues::ParameterValuesMap::const_iterator parameterValueItr = parameterValues.begin();
      parameterValueItr != parameterValues.end(); ++ parameterValueItr)
  {
    const std::string currentValue(parameterValueItr->second);
    if (value == currentValue)
    {
      returnValue = parameterValueItr->first;
      break;
    }
  }

  return returnValue;
}

std::size_t Parameterization::HashCode() const
{
  if (hashCode == HASH_CODE_NOT_COMPUTED)
  {
    hashCode = HASH_INITIAL * HASH_FACTOR + parameter->HashCode();
    hashCode = hashCode * HASH_FACTOR + Poco::hash(value);
    if (hashCode == HASH_CODE_NOT_COMPUTED)
    {
      hashCode++;
    }
  }
  return hashCode;
}

}
