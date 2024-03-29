/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "mitkIsoDoseLevelVectorProperty.h"


mitk::IsoDoseLevelVectorProperty::IsoDoseLevelVectorProperty()
{
}

mitk::IsoDoseLevelVectorProperty::IsoDoseLevelVectorProperty(const mitk::IsoDoseLevelVectorProperty& other)
  : BaseProperty(other)
  , m_IsoLevelVector(other.m_IsoLevelVector)
{
}

mitk::IsoDoseLevelVectorProperty::IsoDoseLevelVectorProperty(IsoDoseLevelVector* levelVector) : m_IsoLevelVector(levelVector)
{
}

mitk::IsoDoseLevelVectorProperty::~IsoDoseLevelVectorProperty()
{
}

bool mitk::IsoDoseLevelVectorProperty::IsEqual(const BaseProperty& property) const
{
  return this->m_IsoLevelVector == static_cast<const Self&>(property).m_IsoLevelVector;
}

bool mitk::IsoDoseLevelVectorProperty::Assign(const BaseProperty& property)
{
  this->m_IsoLevelVector = static_cast<const Self&>(property).m_IsoLevelVector;
  return true;
}

const mitk::IsoDoseLevelVector * mitk::IsoDoseLevelVectorProperty::GetIsoDoseLevelVector() const
{
  return m_IsoLevelVector;
}

const mitk::IsoDoseLevelVector * mitk::IsoDoseLevelVectorProperty::GetValue() const
{
  return GetIsoDoseLevelVector();
}

mitk::IsoDoseLevelVector * mitk::IsoDoseLevelVectorProperty::GetIsoDoseLevelVector()
{
  return m_IsoLevelVector;
}

mitk::IsoDoseLevelVector * mitk::IsoDoseLevelVectorProperty::GetValue()
{
  return GetIsoDoseLevelVector();
}


void mitk::IsoDoseLevelVectorProperty::SetIsoDoseLevelVector(IsoDoseLevelVector* levelVector)
{
  if(m_IsoLevelVector != levelVector)
  {
    m_IsoLevelVector = levelVector;
    Modified();
  }
}

void mitk::IsoDoseLevelVectorProperty::SetValue(IsoDoseLevelVector* levelVector)
{
  SetIsoDoseLevelVector(levelVector);
}

std::string mitk::IsoDoseLevelVectorProperty::GetValueAsString() const
{
  std::stringstream myStr;

  myStr << "IsoDoseLevels: ";

  if (m_IsoLevelVector.IsNotNull())
  {
    myStr << m_IsoLevelVector->Size() << std::endl;

    for (IsoDoseLevelVector::ConstIterator pos = m_IsoLevelVector->Begin(); pos != m_IsoLevelVector->End(); ++pos)
    {
      myStr << "  " << 100*(pos->Value()->GetDoseValue()) << "% : ("<<pos->Value()->GetColor()<< "); iso line: " << pos->Value()->GetVisibleIsoLine() << std::endl;
    }
  }
  return myStr.str();
}

bool mitk::IsoDoseLevelVectorProperty::ToJSON(nlohmann::json&) const
{
  return false; // Not implemented
}

bool mitk::IsoDoseLevelVectorProperty::FromJSON(const nlohmann::json&)
{
  return false; // Not implemented
}

itk::LightObject::Pointer mitk::IsoDoseLevelVectorProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  return result;
}
