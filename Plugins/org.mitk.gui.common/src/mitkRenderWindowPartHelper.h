/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRenderWindowPartHelper_h
#define mitkRenderWindowPartHelper_h

#include <org_mitk_gui_common_Export.h>

namespace mitk
{
  class BaseGeometry;
  struct IRenderWindowPart;

  /**
   * \brief Helper functions related to IRenderWindowPart geometry alignment.
   */
  namespace RenderWindowPartHelper
  {
    /**
     * \brief Test if the render windows of the given part are aligned with the given geometry.
     *
     * The test applies to parts with coupled render windows only, which share a single
     * world geometry. That world geometry counts as aligned if its bounding box equals
     * the bounding box of the given geometry (within a small epsilon).
     *
     * The function returns true whenever there is nothing to check: no part, no geometry,
     * a part with decoupled render windows (each renderer tracks its own alignment, see
     * BaseRenderer::GetReferenceGeometryAligned), no 3D render window, or no world
     * geometry yet.
     *
     * \note The bounding boxes are compared in index coordinates, so geometries with the
     *       same extents in voxels but a different origin, spacing, or orientation still
     *       count as aligned. A stricter comparison of the index-to-world transforms
     *       is not done yet.
     *
     * \note The test is anchored to the world geometry the render windows slice, not to the
     *       orientation of the individual planes. Rotating the planes with the crosshair or
     *       reorienting them via SliceNavigationController::ReorientSlices leaves that world
     *       geometry untouched, so oblique slices of the given geometry count as aligned.
     *       This is deliberate: placing data on a tilted slice stays possible for users who
     *       ask for it.
     *
     * \param[in] renderWindowPart  The render window part whose world geometry is compared. May be nullptr.
     * \param[in] geometry          The geometry to compare against. May be nullptr.
     * \return False if the part has coupled render windows whose world geometry differs from
     *         the given geometry; true otherwise.
     */
    MITK_GUI_COMMON_PLUGIN bool IsRenderWindowPartAlignedWithGeometry(const IRenderWindowPart* renderWindowPart, const BaseGeometry* geometry);

  } // namespace RenderWindowPartHelper
} // namespace mitk

#endif
