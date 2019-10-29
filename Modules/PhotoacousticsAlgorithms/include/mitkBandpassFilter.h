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

#ifndef MITK_BANDPASS_FILTER
#define MITK_BANDPASS_FILTER

#include "mitkImageToImageFilter.h"
#include "MitkPhotoacousticsAlgorithmsExports.h"
#include "mitkPhotoacousticFilterService.h"

namespace mitk {
  /*!
  * \brief Class implementing an mitk::ImageToImageFilter for casting any mitk image to a float image
  */

  class MITKPHOTOACOUSTICSALGORITHMS_EXPORT BandpassFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(BandpassFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    itkSetMacro(HighPass, float);
    itkSetMacro(LowPass, float);
    itkSetMacro(HighPassAlpha, float);
    itkSetMacro(LowPassAlpha, float);
    itkSetMacro(SpeedOfSound, float);
    itkSetMacro(TimeSpacing, float);
    itkSetMacro(IsBFImage, bool);

  protected:
    BandpassFilter();

    ~BandpassFilter() override;

    void SanityCheckPreconditions();

    float m_SpeedOfSound;
    float m_TimeSpacing;
    bool m_IsBFImage;
    float m_HighPass;
    float m_LowPass;
    float m_HighPassAlpha;
    float m_LowPassAlpha;
    mitk::PhotoacousticFilterService::Pointer m_FilterService;

    void GenerateData() override;
  };
} // namespace mitk

#endif //MITK_CAST_TO_FLOAT_IMAGE_FILTER
