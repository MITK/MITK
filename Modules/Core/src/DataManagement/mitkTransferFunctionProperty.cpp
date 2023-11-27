/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTransferFunctionProperty.h"
#include <nlohmann/json.hpp>

namespace mitk
{
  bool TransferFunctionProperty::IsEqual(const BaseProperty &property) const
  {
    return *(this->m_Value) == *(static_cast<const Self &>(property).m_Value);
  }

  bool TransferFunctionProperty::Assign(const BaseProperty &property)
  {
    this->m_Value = static_cast<const Self &>(property).m_Value;
    return true;
  }

  std::string TransferFunctionProperty::GetValueAsString() const
  {
    std::stringstream myStr;
    myStr << GetValue();
    return myStr.str();
  }

  TransferFunctionProperty::TransferFunctionProperty() : BaseProperty(), m_Value(mitk::TransferFunction::New()) {}
  TransferFunctionProperty::TransferFunctionProperty(const TransferFunctionProperty &other)
    : BaseProperty(other), m_Value(other.m_Value->Clone())
  {
  }

  TransferFunctionProperty::TransferFunctionProperty(mitk::TransferFunction::Pointer value)
    : BaseProperty(), m_Value(value)
  {
  }

  bool TransferFunctionProperty::ToJSON(nlohmann::json& j) const
  {
    auto tf = this->GetValue();

    auto scalarOpacity = nlohmann::json::array();

    for (const auto& point : tf->GetScalarOpacityPoints())
      scalarOpacity.push_back(point);

    auto gradientOpacity = nlohmann::json::array();

    for (const auto& point : tf->GetGradientOpacityPoints())
      gradientOpacity.push_back(point);

    auto* ctf = tf->GetColorTransferFunction();
    auto size = ctf->GetSize();

    std::array<double, 6> value;
    auto color = nlohmann::json::array();

    for (int i = 0; i < size; ++i)
    {
      ctf->GetNodeValue(i, value.data());
      color.push_back(value);
    }

    j = nlohmann::json{
      {"ScalarOpacity", scalarOpacity},
      {"GradientOpacity", gradientOpacity},
      {"Color", color}};

    return true;
  }

  bool TransferFunctionProperty::FromJSON(const nlohmann::json& j)
  {
    auto tf = TransferFunction::New();
    TransferFunction::ControlPoints::value_type point; 

    tf->ClearScalarOpacityPoints();

    for (const auto& opacity : j["ScalarOpacity"])
    {
      opacity.get_to(point);
      tf->AddScalarOpacityPoint(point.first, point.second);
    }

    tf->ClearGradientOpacityPoints();

    for (const auto& opacity : j["GradientOpacity"])
    {
      opacity.get_to(point);
      tf->AddGradientOpacityPoint(point.first, point.second);
    }

    auto* ctf = tf->GetColorTransferFunction();
    ctf->RemoveAllPoints();

    std::array<double, 6> value;

    for (const auto& color : j["Color"])
    {
      color.get_to(value);
      ctf->AddRGBPoint(value[0], value[1], value[2], value[3], value[4], value[5]);
    }

    this->SetValue(tf);

    return true;
  }

  itk::LightObject::Pointer TransferFunctionProperty::InternalClone() const
  {
    itk::LightObject::Pointer result(new Self(*this));
    result->UnRegister();
    return result;
  }

} // namespace mitk
