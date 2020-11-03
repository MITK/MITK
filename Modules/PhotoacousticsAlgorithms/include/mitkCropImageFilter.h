/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITK_CROP_IMAGE_FILTER
#define MITK_CROP_IMAGE_FILTER

#include <MitkPhotoacousticsAlgorithmsExports.h>
#include "mitkImageToImageFilter.h"
#include "itkMacro.h"

namespace mitk {
  /*!
  * \brief Class implementing an mitk::ImageToImageFilter for casting any mitk image to a float image
  */

  class MITKPHOTOACOUSTICSALGORITHMS_EXPORT CropImageFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(CropImageFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    itkGetMacro(XPixelsCropStart, unsigned int);
    itkSetMacro(XPixelsCropStart, unsigned int);
    itkGetMacro(YPixelsCropStart, unsigned int);
    itkSetMacro(YPixelsCropStart, unsigned int);
    itkGetMacro(ZPixelsCropStart, unsigned int);
    itkSetMacro(ZPixelsCropStart, unsigned int);

    itkGetMacro(XPixelsCropEnd, unsigned int);
    itkSetMacro(XPixelsCropEnd, unsigned int);
    itkGetMacro(YPixelsCropEnd, unsigned int);
    itkSetMacro(YPixelsCropEnd, unsigned int);
    itkGetMacro(ZPixelsCropEnd, unsigned int);
    itkSetMacro(ZPixelsCropEnd, unsigned int);

  protected:
    CropImageFilter();

    ~CropImageFilter() override;

    void GenerateData() override;

    void SanityCheckPreconditions();

    unsigned int m_XPixelsCropStart;
    unsigned int m_YPixelsCropStart;
    unsigned int m_ZPixelsCropStart;
    unsigned int m_XPixelsCropEnd;
    unsigned int m_YPixelsCropEnd;
    unsigned int m_ZPixelsCropEnd;
 };
} // namespace mitk

#endif //MITK_CROP_IMAGE_FILTER
