/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkVolumeRenderingMaterial.h"

#include <mitkDataNode.h>
#include <mitkExceptionMacro.h>

namespace
{
  constexpr const char *SHADE_PROPERTY = "volumerendering.shade";
  constexpr const char *AMBIENT_PROPERTY = "volumerendering.ambient";
  constexpr const char *DIFFUSE_PROPERTY = "volumerendering.diffuse";
  constexpr const char *SPECULAR_PROPERTY = "volumerendering.specular";
  constexpr const char *SPECULAR_POWER_PROPERTY = "volumerendering.specular.power";
}

mitk::VolumeRenderingMaterial mitk::VolumeRenderingMaterial::FromNode(const DataNode *node)
{
  VolumeRenderingMaterial material;

  if (node == nullptr)
    return material;

  // These leave their target untouched for a key the node does not carry, so
  // each field simply keeps its default initialiser. No presence checks needed.
  node->GetBoolProperty(SHADE_PROPERTY, material.shade);
  node->GetFloatProperty(AMBIENT_PROPERTY, material.ambient);
  node->GetFloatProperty(DIFFUSE_PROPERTY, material.diffuse);
  node->GetFloatProperty(SPECULAR_PROPERTY, material.specular);
  node->GetFloatProperty(SPECULAR_POWER_PROPERTY, material.specularPower);

  return material;
}

void mitk::VolumeRenderingMaterial::ApplyTo(DataNode *node) const
{
  if (node == nullptr)
    mitkThrow() << "Cannot apply a volume rendering material to a null data node.";

  node->SetBoolProperty(SHADE_PROPERTY, shade);
  node->SetFloatProperty(AMBIENT_PROPERTY, ambient);
  node->SetFloatProperty(DIFFUSE_PROPERTY, diffuse);
  node->SetFloatProperty(SPECULAR_PROPERTY, specular);
  node->SetFloatProperty(SPECULAR_POWER_PROPERTY, specularPower);
}
