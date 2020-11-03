/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryParameterization.h"
#include "berryIParameter.h"
#include "berryIParameterValues.h"

namespace berry
{

const uint Parameterization::HASH_CODE_NOT_COMPUTED = 0;
const uint Parameterization::HASH_FACTOR = 89;
const uint Parameterization::HASH_INITIAL = qHash("berry::Parameterization");

Parameterization::Parameterization(const SmartPointer<const IParameter> parameter, const QString& value)
: hashCode(HASH_CODE_NOT_COMPUTED), parameter(parameter), value(value)
{
  if (!parameter)
  {
    throw ctkInvalidArgumentException(
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

QString Parameterization::GetValue() const
{
  return value;
}

QString Parameterization::GetValueName() const
{
  const QHash<QString,QString> parameterValues = parameter->GetValues()->GetParameterValues();

  QString returnValue;
  for (IParameter::ParameterValues::const_iterator parameterValueItr = parameterValues.begin();
      parameterValueItr != parameterValues.end(); ++ parameterValueItr)
  {
    const QString currentValue(parameterValueItr.value());
    if (value == currentValue)
    {
      returnValue = parameterValueItr.key();
      break;
    }
  }

  return returnValue;
}

uint Parameterization::HashCode() const
{
  if (hashCode == HASH_CODE_NOT_COMPUTED)
  {
    hashCode = HASH_INITIAL * HASH_FACTOR + parameter->HashCode();
    hashCode = hashCode * HASH_FACTOR + qHash(value);
    if (hashCode == HASH_CODE_NOT_COMPUTED)
    {
      hashCode++;
    }
  }
  return hashCode;
}

}
