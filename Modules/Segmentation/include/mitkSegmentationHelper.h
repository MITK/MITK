/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationHelper_h
#define mitkSegmentationHelper_h

#include <mitkImage.h>
#include <MitkSegmentationExports.h>

/**
 * \brief Helper functions for segmentation operations.
 * \sa MultiLabelSegmentation
 */
namespace mitk::SegmentationHelper
{
  /**
   * \brief Creates a static 3D template image suitable for segmenting a dynamic (3D+t) reference image.
   *
   * If the reference image is 3D+t, this function creates a single 3D image
   * with matching geometry that can be used as a segmentation template.
   *
   * \param[in] referenceImage The reference image (may be 3D or 3D+t).
   * \return A 3D image with matching spatial geometry, suitable as segmentation template.
   * \pre referenceImage must point to a valid instance.
   */
  MITKSEGMENTATION_EXPORT Image::Pointer GetStaticSegmentationTemplate(const Image* referenceImage);
}

#endif
