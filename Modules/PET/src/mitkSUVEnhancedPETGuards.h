/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSUVEnhancedPETGuards_h
#define mitkSUVEnhancedPETGuards_h

#include <mitkIPropertyProvider.h>
#include <mitkSUVCalculationHelper.h>

namespace mitk
{
  /**
   * \brief Refuse a multi-frame Enhanced PET object that reached MITK
   *        without its functional-group values.
   *
   * The DICOM reader publishes the functional-group attributes of a
   * multi-frame object with one value per slice, or -- for a file whose
   * per-frame item count does not match its frame count -- not at all.
   * This detects the latter: (0028,0008) Number of Frames exceeds one and
   * none of the functional-group attributes the SUV pipeline consumes is
   * present. Such an object carries per-frame values MITK could not map to
   * slices, which every consumer must report as that rather than as absent
   * attributes.
   *
   * The refusal also protects consumers that read no functional-group
   * value: without the frame mapping the reader cannot apply each frame's
   * Pixel Value Transformation either, so the loaded pixels carry the first
   * frame's rescale throughout and are not trustworthy on any path.
   *
   * \param[in] provider Source of DICOM properties.
   *
   * \throw EnhancedPETFramesUnresolvedException if the object is
   *        multi-frame and publishes no functional-group value.
   */
  void RequireEnhancedPETFramesResolved(const IPropertyProvider* provider);
}

#endif
