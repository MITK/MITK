/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTransferFunctionPresets.h"

#include <mitkExceptionMacro.h>
#include <mitkImage.h>
#include <mitkLog.h>
#include <mitkPropertyNameHelper.h>

#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <iomanip>
#include <istream>
#include <ostream>

namespace
{
  // OpacityPoints in the preset file are a flat array [x, opacity, x, opacity, ...].
  mitk::TransferFunction::ControlPoints DecodeScalarOpacity(const nlohmann::json &flat)
  {
    mitk::TransferFunction::ControlPoints points;

    for (std::size_t i = 0; i + 1 < flat.size(); i += 2)
    {
      points.emplace_back(flat[i].get<double>(), flat[i + 1].get<double>());
    }

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

  // Inverse of the SetColorSpaceTo* switch in BuildTransferFunction.
  std::string ColorSpaceToString(int vtkColorSpace)
  {
    switch (vtkColorSpace)
    {
      case VTK_CTF_HSV:
        return "HSV";
      case VTK_CTF_LAB:
        return "Lab";
      case VTK_CTF_DIVERGING:
        return "Diverging";
      case VTK_CTF_RGB:
        return "RGB";
      default:
        return "RGB";
    }
  }
}

mitk::TransferFunctionPresets::TransferFunctionPresets()
{
  // GetModuleContext() resolves to THIS module (MitkVolumeVisualization),
  // because this translation unit is compiled with its US_MODULE_NAME. That is
  // why the embedded MedicalColorPresets.json is found here.
  auto resource = us::GetModuleContext()->GetModule()->GetResource("MedicalColorPresets.json");

  if (!resource.IsValid())
  {
    MITK_WARN << "Embedded resource \"MedicalColorPresets.json\" not found.";
    return;
  }

  us::ModuleResourceStream stream(resource);
  m_Presets = ReadPresets(stream);
}

std::vector<mitk::TransferFunctionPresets::Preset> mitk::TransferFunctionPresets::ReadPresets(std::istream &stream)
{
  std::vector<Preset> presets;

  nlohmann::json json;

  try
  {
    stream >> json;
  }
  catch (const nlohmann::json::parse_error &e)
  {
    MITK_WARN << "Failed to parse transfer function JSON: " << e.what();
    return presets;
  }

  const nlohmann::json entries = json.is_array() ? json : nlohmann::json::array({ json });

  for (const auto &entry : entries)
  {
    // A volume-rendering preset needs a name, a color function, and an opacity
    // function. The file also bundles plain colormaps (e.g. "2hot", "Blue to Red
    // Rainbow") that carry only RGBPoints; those are not volume presets, so skip
    // them.
    if (!entry.contains("Name") || !entry.contains("RGBPoints") || !entry.contains("OpacityPoints"))
      continue;

    Preset preset;

    // The conversions below throw when the file has the right shape but the
    // wrong types. Caught per entry, so one malformed entry is skipped like the
    // colormaps above rather than discarding the whole catalog.
    try
    {
      preset.name = entry["Name"].get<std::string>();
      preset.colorSpace = entry.value("ColorSpace", std::string("RGB"));
      preset.color = DecodeColor(entry["RGBPoints"]);
      preset.scalarOpacity = DecodeScalarOpacity(entry["OpacityPoints"]);

      // Absent for a colormap taken from elsewhere, which was authored without
      // the question in mind. Left to Preset::blendMode's own initialiser
      // rather than defaulted to an id here, so that composite is asserted in
      // one place only and a missing key can never be reported as an unknown
      // mode.
      if (entry.contains("BlendMode"))
      {
        const auto blendModeId = entry["BlendMode"].get<std::string>();

        if (const auto *description = VolumeBlendModeDescription::FromId(blendModeId);
            description != nullptr)
        {
          preset.blendMode = description->mode;
        }
        else
        {
          MITK_WARN << "Unknown blend mode \"" << blendModeId << "\" in preset \"" << preset.name
                    << "\"; falling back to composite.";
        }
      }

      // The effective range is the intensity window a preset is designed for,
      // i.e. the range of voxel values over which the transfer function
      // actually varies. Without one, fall back to the first and last opacity
      // point, so the whole range rather than a subset.
      if (entry.contains("EffectiveRange") && entry["EffectiveRange"].is_array() &&
          entry["EffectiveRange"].size() >= 2)
      {
        preset.effectiveRange = entry["EffectiveRange"].get<std::array<double, 2>>();
      }
      else if (!preset.scalarOpacity.empty())
      {
        preset.effectiveRange =
          std::array<double, 2>{ preset.scalarOpacity.front().first, preset.scalarOpacity.back().first };
      }
    }
    catch (const nlohmann::json::exception &e)
    {
      MITK_WARN << "Skipping malformed transfer function preset entry: " << e.what();
      continue;
    }

    presets.push_back(std::move(preset));
  }

  return presets;
}

mitk::TransferFunction::Pointer mitk::TransferFunctionPresets::BuildTransferFunction(const Preset &preset)
{
  auto transferFunction = mitk::TransferFunction::New();

  if (!preset.scalarOpacity.empty())
    transferFunction->SetScalarOpacityPoints(preset.scalarOpacity);

  // A preset defines opacity only between its control points; treat values
  // outside that range as fully transparent rather than clamping to the end value.
  transferFunction->GetScalarOpacityFunction()->ClampingOff();

  transferFunction->SetRGBPoints(preset.color);

  // The gradient opacity function is intentionally left at its default
  // (constant 1): the preset format defines no gradient component.

  auto *colorFunction = transferFunction->GetColorTransferFunction();

  if (preset.colorSpace == "RGB")
  {
    colorFunction->SetColorSpaceToRGB();
  }
  else if (preset.colorSpace == "HSV")
  {
    colorFunction->SetColorSpaceToHSV();
  }
  else if (preset.colorSpace == "Lab")
  {
    colorFunction->SetColorSpaceToLab();
  }
  else if (preset.colorSpace == "Diverging")
  {
    colorFunction->SetColorSpaceToDiverging();
  }
  else
  {
    MITK_WARN << "Unknown color space \"" << preset.colorSpace << "\" in preset \""
              << preset.name << "\"; falling back to RGB.";
    colorFunction->SetColorSpaceToRGB();
  }

  return transferFunction;
}

std::vector<std::string> mitk::TransferFunctionPresets::GetPresetNames() const
{
  std::vector<std::string> names;
  names.reserve(m_Presets.size());

  for (const auto &preset : m_Presets)
  {
    names.push_back(preset.name);
  }

  return names;
}

std::string mitk::TransferFunctionPresets::GetDefaultPresetName(const Image *image) const
{
  std::string modality;

  if (image != nullptr)
  {
    GetBackwardsCompatibleDICOMPropertyValue(
      0x0008, 0x0060, "modality", image->GetPropertyList(), modality);
  }

  const std::string name = modality == "MR" ? "MR-Default" : "CT-AAA";

  const auto it = std::find_if(m_Presets.begin(), m_Presets.end(),
    [&name](const Preset &preset) { return preset.name == name; });

  if (it != m_Presets.end())
    return it->name;

  // Only reachable through an edited or replaced catalog file. Falling back to
  // the first entry keeps the promise the return value makes - that the name is
  // one this catalog holds - and it is what this view did before any modality
  // was consulted, so the failure mode is the old behaviour.
  MITK_WARN << "The catalog holds no preset \"" << name << "\" to default to.";

  return m_Presets.empty() ? std::string() : m_Presets.front().name;
}

mitk::TransferFunction::Pointer mitk::TransferFunctionPresets::CreateTransferFunction(
  const std::string &presetName, VolumeBlendMode &blendMode) const
{
  const auto it = std::find_if(m_Presets.begin(), m_Presets.end(),
    [&presetName](const Preset &preset) { return preset.name == presetName; });

  if (it == m_Presets.end())
  {
    MITK_WARN << "Unknown transfer function preset \"" << presetName << "\".";
    return nullptr;
  }

  blendMode = it->blendMode;

  return BuildTransferFunction(*it);
}

mitk::TransferFunction::Pointer mitk::TransferFunctionPresets::LoadTransferFunction(
  std::istream &stream, VolumeBlendMode &blendMode)
{
  const auto presets = ReadPresets(stream);

  if (presets.empty())
  {
    MITK_WARN << "No valid transfer function found in stream.";
    return nullptr;
  }

  blendMode = presets.front().blendMode;

  return BuildTransferFunction(presets.front());
}

bool mitk::TransferFunctionPresets::SaveTransferFunction(
  std::ostream &stream, const std::string &name, mitk::TransferFunction *transferFunction,
  VolumeBlendMode blendMode)
{
  if (transferFunction == nullptr || !stream.good())
    return false;

  auto *scalarOpacityFunction = transferFunction->GetScalarOpacityFunction();
  auto *colorFunction = transferFunction->GetColorTransferFunction();

  // Read points straight from the VTK functions: they are the source of truth
  // the editor canvases mutate (the STL-copy getters can lag behind).
  auto opacityPoints = nlohmann::ordered_json::array();
  double opacityNode[4];
  for (int i = 0; i < scalarOpacityFunction->GetSize(); ++i)
  {
    scalarOpacityFunction->GetNodeValue(i, opacityNode);
    opacityPoints.push_back(opacityNode[0]);
    opacityPoints.push_back(opacityNode[1]);
  }

  auto rgbPoints = nlohmann::ordered_json::array();
  double colorNode[6];
  for (int i = 0; i < colorFunction->GetSize(); ++i)
  {
    colorFunction->GetNodeValue(i, colorNode);
    rgbPoints.push_back(colorNode[0]); // x
    rgbPoints.push_back(colorNode[1]); // r
    rgbPoints.push_back(colorNode[2]); // g
    rgbPoints.push_back(colorNode[3]); // b
  }

  std::array<double, 2> effectiveRange{ 0.0, 0.0 };
  if (scalarOpacityFunction->GetSize() > 0)
    scalarOpacityFunction->GetRange(effectiveRange.data());

  const auto *blendModeDescription = VolumeBlendModeDescription::FromMode(blendMode);

  // GetAll() covers every enumerator, so this can only fire for a value cast
  // in from outside the enum. Thrown rather than reported like the failures
  // above - a bad stream, a null function - which a caller can put to the user
  // and have retried. This one is a programming error, and ToVtkBlendMode
  // already treats the same input that way.
  if (blendModeDescription == nullptr)
    mitkThrow() << "Unhandled volume blend mode " << static_cast<int>(blendMode) << ".";

  nlohmann::ordered_json entry;
  entry["Name"] = name;
  entry["ColorSpace"] = ColorSpaceToString(colorFunction->GetColorSpace());
  entry["BlendMode"] = blendModeDescription->id;
  entry["OpacityPoints"] = opacityPoints;
  entry["RGBPoints"] = rgbPoints;
  entry["EffectiveRange"] = effectiveRange;

  auto presets = nlohmann::ordered_json::array();
  presets.push_back(entry);

  stream << std::setw(2) << presets << std::endl;

  return stream.good();
}

