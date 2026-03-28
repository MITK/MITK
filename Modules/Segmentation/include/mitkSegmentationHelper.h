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

namespace mitk::SegmentationHelper
{
  /** \brief Get a template image for a static segmentation of a dynamic (3d+t) reference image.
   */
  MITKSEGMENTATION_EXPORT Image::Pointer GetStaticSegmentationTemplate(const Image* referenceImage);
}

#endif
