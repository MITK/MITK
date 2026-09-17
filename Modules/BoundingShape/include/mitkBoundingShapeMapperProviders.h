/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBoundingShapeMapperProviders_h
#define mitkBoundingShapeMapperProviders_h

#include <MitkBoundingShapeExports.h>

namespace mitk
{
  /** \brief Register the mapper providers for bounding shape rendering.
   *
   * Bounding shapes use plain GeometryData as data type, so their mappers are
   * deliberately not registered when the module is loaded: they would apply
   * to every GeometryData node. Call this function once during application
   * startup to enable bounding shape rendering, typically from a plugin
   * activator. The registration lasts until the module is unloaded.
   */
  MITKBOUNDINGSHAPE_EXPORT void RegisterBoundingShapeMapperProviders();
}

#endif
