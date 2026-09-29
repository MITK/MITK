/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNSliceIndex_h
#define QmitkMxNSliceIndex_h

namespace mitk
{
  class BaseRenderer;
}

namespace QmitkMxNSliceIndex
{
  /**
   * \brief Whether the cell's displayed slice index runs opposite to its slice
   *        stepper.
   *
   * The displayed index follows the image's own index axis for the view
   * direction (see mitk::SliceNavigationHelper::IsSliceIndexInverted). The
   * navigator shows it, and slice offsets are measured in it, so both must use
   * this one definition. Anything not yet set up (no input geometry, no
   * renderer geometry, an 'Original' view) has no image axis to follow, so the
   * stepper position is shown as is.
   */
  bool IsDisplayedSliceInverted(mitk::BaseRenderer* renderer);
}

#endif
