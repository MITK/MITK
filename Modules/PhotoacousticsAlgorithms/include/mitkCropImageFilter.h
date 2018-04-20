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

#ifndef MITK_CROP_IMAGE_FILTER
#define MITK_CROP_IMAGE_FILTER

#include "mitkImageToImageFilter.h"

namespace mitk {
  /*!
  * \brief Class implementing an mitk::ImageToImageFilter for casting any mitk image to a float image
  */

  class CropImageFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(CropImageFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

  protected:
    CropImageFilter();

    ~CropImageFilter() override;

    void GenerateData() override;
  };
} // namespace mitk

#endif //MITK_CROP_IMAGE_FILTER
