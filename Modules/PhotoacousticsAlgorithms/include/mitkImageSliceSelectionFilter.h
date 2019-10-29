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

#ifndef MITK_IMAGE_SLICE_SELECTION_FILTER
#define MITK_IMAGE_SLICE_SELECTION_FILTER

#include "mitkImageToImageFilter.h"

namespace mitk {
  /*!
  * \brief Class implementing an mitk::ImageToImageFilter for casting any mitk image to a float image
  */

  class ImageSliceSelectionFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(ImageSliceSelectionFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

  protected:
    ImageSliceSelectionFilter();

    ~ImageSliceSelectionFilter() override;

    void GenerateData() override;
  };
} // namespace mitk

#endif //MITK_IMAGE_SLICE_SELECTION_FILTER
