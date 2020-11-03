/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITK_CAST_TO_FLOAT_IMAGE_FILTER
#define MITK_CAST_TO_FLOAT_IMAGE_FILTER

#include "mitkImageToImageFilter.h"
#include "MitkPhotoacousticsAlgorithmsExports.h"

namespace mitk {
  /*!
  * \brief Class implementing an mitk::ImageToImageFilter for casting any mitk image to a float image
  */

  class MITKPHOTOACOUSTICSALGORITHMS_EXPORT CastToFloatImageFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(CastToFloatImageFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

  protected:
    CastToFloatImageFilter();

    ~CastToFloatImageFilter() override;

    void GenerateData() override;
  };
} // namespace mitk

#endif //MITK_CAST_TO_FLOAT_IMAGE_FILTER
