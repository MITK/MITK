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
