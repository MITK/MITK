/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
   * \brief Convert mitk::Image to mitk::LabelSetImage, templating and differentation between itk::Image and itk::VectorImage is internal
   */
  MITKMULTILABEL_EXPORT LabelSetImage::Pointer ConvertImageToLabelSetImage(Image::Pointer image);
}

#endif
