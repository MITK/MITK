/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLevelWindowProperty.h"

mitk::LevelWindowProperty::LevelWindowProperty()
{
}

mitk::LevelWindowProperty::LevelWindowProperty(const mitk::LevelWindowProperty &other)
  : BaseProperty(other), m_LevWin(other.m_LevWin)
{
}

mitk::LevelWindowProperty::LevelWindowProperty(const mitk::LevelWindow &levWin)
{
  SetLevelWindow(levWin);
}

mitk::LevelWindowProperty::~LevelWindowProperty()
{
}

bool mitk::LevelWindowProperty::IsEqual(const BaseProperty &property) const
{
  return this->m_LevWin == static_cast<const Self &>(property).m_LevWin;
}

bool mitk::LevelWindowProperty::Assign(const BaseProperty &property)
{
  this->m_LevWin = static_cast<const Self &>(property).m_LevWin;
  return true;
}

const mitk::LevelWindow &mitk::LevelWindowProperty::GetLevelWindow() const
{
  return m_LevWin;
}

const mitk::LevelWindow &mitk::LevelWindowProperty::GetValue() const
{
  return GetLevelWindow();
}

void mitk::LevelWindowProperty::SetLevelWindow(const mitk::LevelWindow &levWin)
{
  if (m_LevWin != levWin)
  {
    m_LevWin = levWin;
    Modified();
  }
}

void mitk::LevelWindowProperty::SetValue(const ValueType &levWin)
{
  SetLevelWindow(levWin);
}

std::string mitk::LevelWindowProperty::GetValueAsString() const
{
  std::stringstream myStr;
  myStr << "L:" << m_LevWin.GetLevel() << " W:" << m_LevWin.GetWindow();
  return myStr.str();
}

bool mitk::LevelWindowProperty::ToJSON(nlohmann::json& j) const
{
  j = this->GetLevelWindow();
  return true;
}

bool mitk::LevelWindowProperty::FromJSON(const nlohmann::json& j)
{
  this->SetLevelWindow(j.get<LevelWindow>());
  return true;
}

itk::LightObject::Pointer mitk::LevelWindowProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
