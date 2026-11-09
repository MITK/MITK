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
  /**
   * \brief Helper functions related to BaseRenderer geometry alignment.
   */
  namespace BaseRendererHelper
  {
    /**
     * \brief Test if the given time geometry aligns with the current world geometry of the
     *        given base renderer.
     *
     * To compare the two geometries, the given time geometry is first converted into
     * an oriented time geometry depending on the current view direction of the base
     * renderer. A time-extracted 3D geometry is created using its time step for the
     * renderer's time point, clamped to its valid time steps. If the bounding boxes of these two
     * geometries are equal (within a small epsilon), the function returns true;
     * otherwise false.
     *
     * If the given time geometry is nullptr, the function returns true, since nullptr
     * implies that no requirements on the geometry exist.
     *
     * \param[in] renderer      The base renderer whose world geometry is compared.
     * \param[in] timeGeometry  The time geometry to compare against. May be nullptr.
     * \return True if the geometries are aligned or if timeGeometry is nullptr, false otherwise.
     * \throw mitk::Exception  If the time geometry has no time steps.
     */
    MITKCORE_EXPORT bool IsRendererGeometryAlignedWithGeometry(BaseRenderer* renderer, const TimeGeometry* timeGeometry);

  } // namespace BaseRendererHelper
} // namespace mitk

#endif
