/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkVolumeBlendMode.h"

#include <mitkDataNode.h>
#include <mitkExceptionMacro.h>

#include <vtkVolumeMapper.h>

#include <algorithm>

namespace
{
  /** \brief The mode a vtkVolumeMapper::BlendModes value stands for.
   *
   * File-local, unlike its counterpart ToVtkBlendMode: the enum exists so that
   * callers never handle a raw mode value, and the one place a raw one arrives
   * from outside - the node property - is served by GetVolumeBlendMode.
   *
   * \return Nothing for isosurface, slice, or any value outside the enum.
   */
  std::optional<mitk::VolumeBlendMode> FromVtkBlendMode(int vtkBlendMode)
  {
    switch (vtkBlendMode)
    {
      case vtkVolumeMapper::COMPOSITE_BLEND:
        return mitk::VolumeBlendMode::Composite;
      case vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND:
        return mitk::VolumeBlendMode::MaximumIntensity;
      case vtkVolumeMapper::MINIMUM_INTENSITY_BLEND:
        return mitk::VolumeBlendMode::MinimumIntensity;
      case vtkVolumeMapper::AVERAGE_INTENSITY_BLEND:
        return mitk::VolumeBlendMode::AverageIntensity;
      case vtkVolumeMapper::ADDITIVE_BLEND:
        return mitk::VolumeBlendMode::Additive;
      default:
        return std::nullopt;
    }
  }
}

const std::vector<mitk::VolumeBlendModeDescription> &mitk::VolumeBlendModeDescription::GetAll()
{
  static const std::vector<VolumeBlendModeDescription> descriptions {
    {VolumeBlendMode::Composite, "Composite", "Composite (3D)",
     "Builds a 3D image in which nearer tissue hides what is behind it."},
    {VolumeBlendMode::MaximumIntensity, "MaximumIntensity", "Maximum intensity (MIP)",
     "Shows the brightest value along each ray, for example contrast-filled vessels."},
    {VolumeBlendMode::MinimumIntensity, "MinimumIntensity", "Minimum intensity (MinIP)",
     "Shows the darkest value along each ray, for example airways."},
    {VolumeBlendMode::AverageIntensity, "AverageIntensity", "Average intensity",
     "Shows the average along each ray, like an X-ray image. Greyscale only."},
    {VolumeBlendMode::Additive, "Additive", "Additive intensity",
     "Adds up the values along each ray. Greyscale only."}
  };

  return descriptions;
}

const mitk::VolumeBlendModeDescription *mitk::VolumeBlendModeDescription::FromMode(VolumeBlendMode mode)
{
  const auto &descriptions = GetAll();

  const auto it = std::find_if(descriptions.begin(), descriptions.end(),
    [mode](const VolumeBlendModeDescription &description) { return description.mode == mode; });

  return it != descriptions.end() ? &*it : nullptr;
}

const mitk::VolumeBlendModeDescription *mitk::VolumeBlendModeDescription::FromId(const std::string &id)
{
  const auto &descriptions = GetAll();

  const auto it = std::find_if(descriptions.begin(), descriptions.end(),
    [&id](const VolumeBlendModeDescription &description) { return description.id == id; });

  return it != descriptions.end() ? &*it : nullptr;
}

int mitk::ToVtkBlendMode(VolumeBlendMode mode)
{
  switch (mode)
  {
    case VolumeBlendMode::Composite:
      return vtkVolumeMapper::COMPOSITE_BLEND;
    case VolumeBlendMode::MaximumIntensity:
      return vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND;
    case VolumeBlendMode::MinimumIntensity:
      return vtkVolumeMapper::MINIMUM_INTENSITY_BLEND;
    case VolumeBlendMode::AverageIntensity:
      return vtkVolumeMapper::AVERAGE_INTENSITY_BLEND;
    case VolumeBlendMode::Additive:
      return vtkVolumeMapper::ADDITIVE_BLEND;
  }

  mitkThrow() << "Unhandled volume blend mode " << static_cast<int>(mode) << ".";
}

void mitk::SetVolumeBlendMode(DataNode *node, VolumeBlendMode mode)
{
  if (node == nullptr)
    mitkThrow() << "Cannot set a volume blend mode on a null data node.";

  node->SetIntProperty(VOLUME_BLEND_MODE_PROPERTY, ToVtkBlendMode(mode));
}

std::optional<mitk::VolumeBlendMode> mitk::GetVolumeBlendMode(const DataNode *node)
{
  if (node == nullptr)
    return std::nullopt;

  // Seeded rather than asked about: GetIntProperty leaves its target alone for
  // a key the node does not carry, so a node naming no mode reads as composite
  // - what the mapper renders it as - without absence needing a case of its
  // own. That leaves an empty return to mean one thing only, a mode outside the
  // set, which is the distinction callers act on.
  int vtkBlendMode = ToVtkBlendMode(VolumeBlendMode::Composite);
  node->GetIntProperty(VOLUME_BLEND_MODE_PROPERTY, vtkBlendMode);

  return FromVtkBlendMode(vtkBlendMode);
}
