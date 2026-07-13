/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTransferFunctionPresets.h"

#include <mitkLog.h>

#include <vtkColorTransferFunction.h>

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include <nlohmann/json.hpp>

#include <algorithm>

namespace
{
  // OpacityPoints in the preset file are a flat array [x, opacity, x, opacity, ...].
  mitk::TransferFunction::ControlPoints DecodeScalarOpacity(const nlohmann::json &flat)
  {
    mitk::TransferFunction::ControlPoints points;

    for (std::size_t i = 0; i + 1 < flat.size(); i += 2)
      points.emplace_back(flat[i].get<double>(), flat[i + 1].get<double>());

    return points;
  }

  // RGBPoints are a flat array [x, r, g, b, x, r, g, b, ...] with channels in [0, 1].
  mitk::TransferFunction::RGBControlPoints DecodeColor(const nlohmann::json &flat)
  {
    mitk::TransferFunction::RGBControlPoints points;

    for (std::size_t i = 0; i + 3 < flat.size(); i += 4)
    {
      itk::RGBPixel<double> color;
      color[0] = flat[i + 1].get<double>();
      color[1] = flat[i + 2].get<double>();
      color[2] = flat[i + 3].get<double>();

      points.emplace_back(flat[i].get<double>(), color);
    }

    return points;
  }
}

mitk::TransferFunctionPresets::TransferFunctionPresets()
{
  // GetModuleContext() resolves to THIS module (MitkVolumeVisualizationUI),
  // because this translation unit is compiled with its US_MODULE_NAME. That is
  // why the embedded MedicalColorPresets.json is found here.
  auto resource = us::GetModuleContext()->GetModule()->GetResource("MedicalColorPresets.json");

  if (!resource.IsValid())
  {
    MITK_WARN << "Embedded resource \"MedicalColorPresets.json\" not found.";
    return;
  }

  us::ModuleResourceStream stream(resource);

  nlohmann::json presets;

  try
  {
    stream >> presets;
  }
  catch (const nlohmann::json::parse_error &e)
  {
    MITK_WARN << "Failed to parse \"MedicalColorPresets.json\": " << e.what();
    return;
  }

  for (const auto &entry : presets)
  {
    // A preset without a name or without colors is unusable; skip it.
    if (!entry.contains("Name") || !entry.contains("RGBPoints"))
      continue;

    Preset preset;
    preset.name = entry["Name"].get<std::string>();
    preset.colorSpace = entry.value("ColorSpace", std::string("RGB"));
    preset.color = DecodeColor(entry["RGBPoints"]);

    // Opacity is optional (pure colormaps omit it).
    if (entry.contains("OpacityPoints"))
      preset.scalarOpacity = DecodeScalarOpacity(entry["OpacityPoints"]);

    m_Presets.push_back(std::move(preset));
  }
}

std::vector<std::string> mitk::TransferFunctionPresets::GetPresetNames() const
{
  std::vector<std::string> names;
  names.reserve(m_Presets.size());

  for (const auto &preset : m_Presets)
    names.push_back(preset.name);

  return names;
}

mitk::TransferFunction::Pointer mitk::TransferFunctionPresets::CreateTransferFunction(
  const std::string &presetName) const
{
  const auto it = std::find_if(m_Presets.begin(), m_Presets.end(),
    [&presetName](const Preset &preset) { return preset.name == presetName; });

  if (it == m_Presets.end())
  {
    MITK_WARN << "Unknown transfer function preset \"" << presetName << "\".";
    return nullptr;
  }

  auto transferFunction = mitk::TransferFunction::New();

  transferFunction->SetScalarOpacityPoints(it->scalarOpacity);
  transferFunction->SetRGBPoints(it->color);

  // The gradient opacity function is intentionally left at its default
  // (constant 1): the preset file defines no gradient component.

  auto *colorFunction = transferFunction->GetColorTransferFunction();

  if (it->colorSpace == "RGB")
    colorFunction->SetColorSpaceToRGB();
  else if (it->colorSpace == "HSV")
    colorFunction->SetColorSpaceToHSV();
  else if (it->colorSpace == "Lab")
    colorFunction->SetColorSpaceToLab();
  else if (it->colorSpace == "Diverging")
    colorFunction->SetColorSpaceToDiverging();
  else
  {
    MITK_WARN << "Unknown color space \"" << it->colorSpace << "\" in preset \""
              << it->name << "\"; falling back to RGB.";
    colorFunction->SetColorSpaceToRGB();
  }

  return transferFunction;
}
