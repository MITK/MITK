/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLabelSetImageConverter_h
#define mitkLabelSetImageConverter_h

#include <mitkLabelSetImage.h>

namespace mitk
{
  /**
   * \brief Convert mitk::LabelSetImage to mitk::Image (itk::VectorImage)
   */
  MITKMULTILABEL_EXPORT Image::Pointer ConvertLabelSetImageToImage(LabelSetImage::ConstPointer labelSetImage);

  /**
   * \brief Convert mitk::Image to mitk::LabelSetImage, templating and differentation between itk::Image and
   * itk::VectorImage is internal
   */
  MITKMULTILABEL_EXPORT LabelSetImage::Pointer ConvertImageToLabelSetImage(Image::Pointer image);
}

#endif
