/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTransferFunctionProperty.h>
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
      {"ScalarOpacityClamping", tf->GetScalarOpacityFunction()->GetClamping() != 0},
      {"GradientOpacity", gradientOpacity},
      {"GradientOpacityClamping", tf->GetGradientOpacityFunction()->GetClamping() != 0},
      {"Color", color},
      {"ColorSpace", TransferFunctionColorSpaceToString(tf->GetColorSpace())}};

    return true;
  }

  bool TransferFunctionProperty::FromJSON(const nlohmann::json& j)
  {
    auto tf = TransferFunction::New();
    TransferFunction::ControlPoints::value_type point; 

    // Clamping and color space are absent from JSON written before they were serialized.
    // Those transfer functions were rendered with the VTK clamping default and the HSV
    // constructor default, so that is what they must read as.

    tf->ClearScalarOpacityPoints();
    tf->GetScalarOpacityFunction()->SetClamping(j.value("ScalarOpacityClamping", true));

    for (const auto& opacity : j["ScalarOpacity"])
    {
      opacity.get_to(point);
      tf->AddScalarOpacityPoint(point.first, point.second);
    }

    tf->ClearGradientOpacityPoints();
    tf->GetGradientOpacityFunction()->SetClamping(j.value("GradientOpacityClamping", true));

    for (const auto& opacity : j["GradientOpacity"])
    {
      opacity.get_to(point);
      tf->AddGradientOpacityPoint(point.first, point.second);
    }

    auto* ctf = tf->GetColorTransferFunction();
    ctf->RemoveAllPoints();

    auto colorSpace = TransferFunctionColorSpace::HSV;

    if (j.contains("ColorSpace"))
    {
      const auto colorSpaceName = j.at("ColorSpace").get<std::string>();

      if (const auto parsedColorSpace = TransferFunctionColorSpaceFromString(colorSpaceName))
      {
        colorSpace = *parsedColorSpace;
      }
      else
      {
        MITK_WARN << "Unknown transfer function color space \"" << colorSpaceName << "\"; falling back to HSV.";
      }
    }

    tf->SetColorSpace(colorSpace);

    std::array<double, 6> value;

    for (const auto& color : j["Color"])
    {
      color.get_to(value);
      ctf->AddRGBPoint(value[0], value[1], value[2], value[3], value[4], value[5]);
    }

    this->SetValue(tf);

    return true;
  }

} // namespace mitk
