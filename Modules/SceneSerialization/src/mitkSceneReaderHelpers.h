/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSceneReaderHelpers_h
#define mitkSceneReaderHelpers_h

#include <mitkBaseData.h>
#include <mitkCoreServices.h>
#include <mitkIPropertyTransience.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkPropertyList.h>

#include <limits>
#include <string>
#include <vector>

namespace mitk::SceneReaderHelpers
{
  /**
   * \brief Remove transient properties (e.g. the "selected" UI flag) from a
   *        freshly read property list.
   *
   * Older scene files may carry transient keys that the save path now excludes;
   * stripping them on load keeps a reloaded scene from resurrecting runtime or
   * UI state. Transience is decided per the node's BaseData type. \p data may be
   * null for a data-less node, in which case rules registered for mitk::BaseData
   * still apply.
   *
   * Shared between SceneReaderV1 (XML) and SceneJsonReader so both scene readers
   * agree on the behavior.
   */
  inline void StripTransientProperties(PropertyList &propertyList, const BaseData *data)
  {
    CoreServicePointer<IPropertyTransience> transience(CoreServices::GetPropertyTransience());
    if (!transience)
      return;

    std::vector<std::string> transientKeys;
    for (const auto &property : *propertyList.GetMap())
    {
      if (transience->IsTransient(data, property.first))
        transientKeys.push_back(property.first);
    }

    for (const auto &key : transientKeys)
      propertyList.DeleteProperty(key);
  }

  /**
   * \brief Reconstruct a proportional time geometry from properties on a
   *        BaseData's property list.
   *
   * Workaround for file formats (notably Surfaces) that cannot persist
   * time-geometry information. If \p data carries a ProportionalTimeGeometry
   * and its property list contains `ProportionalTimeGeometry.FirstTimePoint`
   * and/or `ProportionalTimeGeometry.StepDuration`, those values are applied
   * back to the geometry.
   *
   * Shared between SceneReaderV1 (XML) and SceneJsonReader so both scene
   * readers agree on the behavior.
   */
  inline void ApplyProportionalTimeGeometryProperties(BaseData *data)
  {
    auto *geometry = dynamic_cast<ProportionalTimeGeometry *>(data->GetTimeGeometry());
    if (geometry == nullptr)
      return;

    PropertyList::ConstPointer properties = data->GetPropertyList();
    if (properties == nullptr)
      return;

    float value = 0.0f;
    if (properties->GetFloatProperty("ProportionalTimeGeometry.FirstTimePoint", value))
    {
      if (value == -std::numeric_limits<float>::infinity())
        value = std::numeric_limits<float>::lowest();
      geometry->SetFirstTimePoint(value);
    }
    if (properties->GetFloatProperty("ProportionalTimeGeometry.StepDuration", value))
      geometry->SetStepDuration(value);
  }
}

#endif
