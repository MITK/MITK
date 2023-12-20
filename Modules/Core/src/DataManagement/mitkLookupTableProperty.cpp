/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLookupTableProperty.h"
#include <nlohmann/json.hpp>

mitk::LookupTableProperty::LookupTableProperty()
{
  mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
  this->SetLookupTable(lut);
}

mitk::LookupTableProperty::LookupTableProperty(const LookupTableProperty &other)
  : mitk::BaseProperty(other), m_LookupTable(other.m_LookupTable)
{
}

mitk::LookupTableProperty::LookupTableProperty(const mitk::LookupTable::Pointer lut)
{
  this->SetLookupTable(lut);
}

bool mitk::LookupTableProperty::IsEqual(const BaseProperty &property) const
{
  return *(this->m_LookupTable) == *(static_cast<const Self &>(property).m_LookupTable);
}

bool mitk::LookupTableProperty::Assign(const BaseProperty &property)
{
  this->m_LookupTable = static_cast<const Self &>(property).m_LookupTable;
  return true;
}

std::string mitk::LookupTableProperty::GetValueAsString() const
{
  std::stringstream ss;
  ss << m_LookupTable;
  return ss.str();
}

mitk::LookupTableProperty::ValueType mitk::LookupTableProperty::GetValue() const
{
  return m_LookupTable;
}

void mitk::LookupTableProperty::SetLookupTable(const mitk::LookupTable::Pointer aLookupTable)
{
  if ((m_LookupTable != aLookupTable) || (*m_LookupTable != *aLookupTable))
  {
    m_LookupTable = aLookupTable;
    Modified();
  }
}

void mitk::LookupTableProperty::SetValue(const ValueType &value)
{
  SetLookupTable(value);
}

bool mitk::LookupTableProperty::ToJSON(nlohmann::json& j) const
{
  auto lut = this->GetValue()->GetVtkLookupTable();

  auto table = nlohmann::json::array();
  auto numTableValues = lut->GetNumberOfTableValues();

  for (decltype(numTableValues) i = 0; i < numTableValues; ++i)
  {
    auto value = nlohmann::json::array();

    for (size_t j = 0; j < 4; ++j)
      value.push_back(lut->GetTableValue(i)[j]);

    table.push_back(value);
  }

  j = nlohmann::json::object();
  j["NumberOfColors"] = static_cast<int>(lut->GetNumberOfTableValues());
  j["Scale"] = lut->GetScale();
  j["Ramp"] = lut->GetRamp();
  j["HueRange"] = nlohmann::json::array({ lut->GetHueRange()[0], lut->GetHueRange()[1] });
  j["ValueRange"] = nlohmann::json::array({ lut->GetValueRange()[0], lut->GetValueRange()[1] });
  j["SaturationRange"] = nlohmann::json::array({ lut->GetSaturationRange()[0], lut->GetSaturationRange()[1] });
  j["AlphaRange"] = nlohmann::json::array({ lut->GetAlphaRange()[0], lut->GetAlphaRange()[1] });
  j["TableRange"] = nlohmann::json::array({ lut->GetTableRange()[0], lut->GetTableRange()[1] });
  j["Table"] = table;

  return true;
}

bool mitk::LookupTableProperty::FromJSON(const nlohmann::json& j)
{
  auto lut = vtkSmartPointer<vtkLookupTable>::New();

  lut->SetNumberOfTableValues(j["NumberOfColors"].get<int>());
  lut->SetScale(j["Scale"].get<int>());
  lut->SetScale(j["Ramp"].get<int>());

  std::array<double, 2> range;

  j["HueRange"][0].get_to(range[0]);
  j["HueRange"][1].get_to(range[1]);
  lut->SetHueRange(range.data());

  j["ValueRange"][0].get_to(range[0]);
  j["ValueRange"][1].get_to(range[1]);
  lut->SetValueRange(range.data());

  j["SaturationRange"][0].get_to(range[0]);
  j["SaturationRange"][1].get_to(range[1]);
  lut->SetSaturationRange(range.data());

  j["AlphaRange"][0].get_to(range[0]);
  j["AlphaRange"][1].get_to(range[1]);
  lut->SetAlphaRange(range.data());

  j["TableRange"][0].get_to(range[0]);
  j["TableRange"][1].get_to(range[1]);
  lut->SetTableRange(range.data());

  std::array<double, 4> rgba;
  vtkIdType i = 0;

  for (const auto& value : j["Table"])
  {
    for (size_t j = 0; j < 4; ++j)
      value[j].get_to(rgba[j]);

    lut->SetTableValue(i++, rgba.data());
  }

  auto mitkLut = LookupTable::New();
  mitkLut->SetVtkLookupTable(lut);
  this->SetLookupTable(mitkLut);

  return true;
}

itk::LightObject::Pointer mitk::LookupTableProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
