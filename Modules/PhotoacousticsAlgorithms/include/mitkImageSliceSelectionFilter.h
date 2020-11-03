/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
