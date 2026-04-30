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
#include <mitkProportionalTimeGeometry.h>
#include <mitkPropertyList.h>

#include <limits>

namespace mitk::SceneReaderHelpers
{
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
