/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBaseRendererHelper_h
#define mitkBaseRendererHelper_h

#include <mitkBaseRenderer.h>
#include <mitkTimeGeometry.h>

namespace mitk
{
  namespace BaseRendererHelper
  {
    /*
     * \brief Test if the given time geometry aligns with the current world geometry of the given base renderer.
     *        To compare the two geometries the given time geometry is first converted into a oriented time geometry,
     *        depending on the current view direction of the base renderer.
     *        A time-extracted 3D geometry is created by using the current timepoint of the global time navigation
     *        controller.
     *        If the bounding boxes of these two geometries are equal (within a small epsilon), the function returns true,
     *        otherwise false.
     *        If the given time geometry is a nullptr, the function returns true as well, since a nullptr implies that
     *        no requirements on the geometry exist.
     */
    MITKCORE_EXPORT bool IsRendererGeometryAlignedWithGeometry(BaseRenderer* renderer, const TimeGeometry* timeGeometry);

  } // namespace BaseRendererHelper
} // namespace mitk

#endif
