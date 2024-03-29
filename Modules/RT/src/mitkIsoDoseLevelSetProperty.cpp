/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "mitkIsoDoseLevelSetProperty.h"


mitk::IsoDoseLevelSetProperty::IsoDoseLevelSetProperty()
{
}

mitk::IsoDoseLevelSetProperty::IsoDoseLevelSetProperty(const mitk::IsoDoseLevelSetProperty& other)
  : BaseProperty(other)
  , m_IsoLevelSet(other.m_IsoLevelSet)
{
}

mitk::IsoDoseLevelSetProperty::IsoDoseLevelSetProperty(IsoDoseLevelSet* levelSet) : m_IsoLevelSet(levelSet)
{
}

mitk::IsoDoseLevelSetProperty::~IsoDoseLevelSetProperty()
{
}

bool mitk::IsoDoseLevelSetProperty::IsEqual(const BaseProperty& property) const
{
  return this->m_IsoLevelSet == static_cast<const Self&>(property).m_IsoLevelSet;
}

bool mitk::IsoDoseLevelSetProperty::Assign(const BaseProperty& property)
{
  this->m_IsoLevelSet = static_cast<const Self&>(property).m_IsoLevelSet;
  return true;
}

const mitk::IsoDoseLevelSet * mitk::IsoDoseLevelSetProperty::GetIsoDoseLevelSet() const
{
  return m_IsoLevelSet;
}

const mitk::IsoDoseLevelSet * mitk::IsoDoseLevelSetProperty::GetValue() const
{
  return GetIsoDoseLevelSet();
}

mitk::IsoDoseLevelSet * mitk::IsoDoseLevelSetProperty::GetIsoDoseLevelSet()
{
  return m_IsoLevelSet;
}

mitk::IsoDoseLevelSet * mitk::IsoDoseLevelSetProperty::GetValue()
{
  return GetIsoDoseLevelSet();
}


void mitk::IsoDoseLevelSetProperty::SetIsoDoseLevelSet(IsoDoseLevelSet* levelSet)
{
  if(m_IsoLevelSet != levelSet)
  {
    m_IsoLevelSet = levelSet;
    Modified();
  }
}

void mitk::IsoDoseLevelSetProperty::SetValue(IsoDoseLevelSet* levelSet)
{
  SetIsoDoseLevelSet(levelSet);
}

std::string mitk::IsoDoseLevelSetProperty::GetValueAsString() const
{
  std::stringstream myStr;

  myStr << "IsoDoseLevels: ";

  if (m_IsoLevelSet.IsNotNull())
  {
    myStr << m_IsoLevelSet->Size() << std::endl;

    for (IsoDoseLevelSet::ConstIterator pos = m_IsoLevelSet->Begin(); pos != m_IsoLevelSet->End(); ++pos)
    {
      myStr << "  " << 100*(pos->GetDoseValue()) << "% : ("<<pos->GetColor()<< "); iso line: " << pos->GetVisibleIsoLine() << "; color wash: "<<pos->GetVisibleColorWash() << std::endl;
    }
  }
  return myStr.str();
}

bool mitk::IsoDoseLevelSetProperty::ToJSON(nlohmann::json&) const
{
  return false; // Not implemented
}

bool mitk::IsoDoseLevelSetProperty::FromJSON(const nlohmann::json&)
{
  return false; // Not implemented
}

itk::LightObject::Pointer mitk::IsoDoseLevelSetProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  return result;
}
