/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkVolumeRenderingLightingModel.h"

#include "mitkVolumeRenderingMaterial.h"

#include <mitkDataNode.h>
#include <mitkExceptionMacro.h>

#include <algorithm>

namespace
{
  // Written by ApplyTo and read nowhere else, so unlike MODEL_PROPERTY these
  // stay private to this translation unit.
  constexpr const char *SCATTERING_BLEND_PROPERTY = "volumerendering.scattering.blend";
  constexpr const char *SCATTERING_REACH_PROPERTY = "volumerendering.scattering.reach";
  constexpr const char *SCATTERING_ANISOTROPY_PROPERTY = "volumerendering.scattering.anisotropy";
  constexpr const char *NORMALS_FROM_OPACITY_PROPERTY = "volumerendering.normalsFromOpacity";

  using LightingMode = mitk::VtkPropRenderer::LightingMode;
}

const std::vector<mitk::VolumeRenderingLightingModel> &mitk::VolumeRenderingLightingModel::GetAllModels()
{
  // The studio row names the state an unconfigured node already renders in, so
  // it takes its material from the definition of that state rather than a copy.
  constexpr VolumeRenderingMaterial defaults {};

  // Function-local static: built once on first use, and alive for the rest of
  // the program, which is what lets FromId and FromNode return pointers into it.
  static const std::vector<VolumeRenderingLightingModel> models {
    //  id           label                blend  reach  aniso  nFromOp ambient diffuse specular power  rig
    { "headlight", "Headlight",           0.00f, 0.00f, 0.0f,  false,  0.20f,  0.70f,  0.10f,   30.0f, LightingMode::Headlight },
    { "keylight",  "Key light",           0.40f, 0.12f, 0.0f,  false,  0.00f,  0.80f,  0.10f,   30.0f, LightingMode::KeyLight  },
    { "studio",    "Default lighting",    0.00f, 0.00f, 0.0f,  false,
      defaults.ambient, defaults.diffuse, defaults.specular, defaults.specularPower, LightingMode::Studio },
  };

  return models;
}

const mitk::VolumeRenderingLightingModel *mitk::VolumeRenderingLightingModel::FromId(const std::string &id)
{
  const auto &models = GetAllModels();

  const auto it = std::find_if(models.begin(), models.end(),
    [&id](const VolumeRenderingLightingModel &model) { return model.id == id; });

  return it != models.end() ? &*it : nullptr;
}

const mitk::VolumeRenderingLightingModel *mitk::VolumeRenderingLightingModel::FromLightingMode(
  VtkPropRenderer::LightingMode mode)
{
  const auto &models = GetAllModels();

  const auto it = std::find_if(models.begin(), models.end(),
    [mode](const VolumeRenderingLightingModel &model) { return model.lightingMode == mode; });

  return it != models.end() ? &*it : nullptr;
}

const mitk::VolumeRenderingLightingModel *mitk::VolumeRenderingLightingModel::FromNode(const DataNode *node)
{
  if (node == nullptr)
    return nullptr;

  std::string modelId;

  if (!node->GetStringProperty(MODEL_PROPERTY, modelId))
    return nullptr;

  return FromId(modelId);
}

void mitk::VolumeRenderingLightingModel::ApplyTo(DataNode *node) const
{
  if (node == nullptr)
    mitkThrow() << "Cannot apply a volume rendering lighting model to a null data node.";

  VolumeRenderingMaterial material;

  // Every model describes a shading rig, and with shading off VTK ignores all
  // of it: the four material values below and both scattering parameters.
  material.shade = true;

  material.ambient = ambient;
  material.diffuse = diffuse;
  material.specular = specular;
  material.specularPower = specularPower;

  material.ApplyTo(node);

  node->SetFloatProperty(SCATTERING_BLEND_PROPERTY, blend);
  node->SetFloatProperty(SCATTERING_REACH_PROPERTY, reach);
  node->SetFloatProperty(SCATTERING_ANISOTROPY_PROPERTY, anisotropy);
  node->SetBoolProperty(NORMALS_FROM_OPACITY_PROPERTY, normalsFromOpacity);

  node->SetStringProperty(MODEL_PROPERTY, id.c_str());
}
