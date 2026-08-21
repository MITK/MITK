/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBoundingShapeMapperProviders.h>

#include <mitkBaseRenderer.h>
#include <mitkBoundingShapeVtkMapper2D.h>
#include <mitkBoundingShapeVtkMapper3D.h>
#include <mitkGeometryData.h>
#include <mitkMapperProviderBase.h>

void mitk::RegisterBoundingShapeMapperProviders()
{
  // Bounding shape rendering is enabled once per process and stays enabled:
  // the function-local statics keep the providers registered until the module
  // is unloaded.
  static MapperProviderBase<BoundingShapeVtkMapper2D, GeometryData> provider2D(BaseRenderer::Standard2D);
  static MapperProviderBase<BoundingShapeVtkMapper3D, GeometryData> provider3D(BaseRenderer::Standard3D);
}
