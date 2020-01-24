/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkOtsuSegmentationFilter_h_Included
#define mitkOtsuSegmentationFilter_h_Included

//#include "MitkSBExports.h"
#include "mitkITKImageImport.h"
#include "mitkImage.h"
#include "mitkImageToImageFilter.h"

#include "itkImage.h"

#include <MitkSegmentationExports.h>

namespace mitk
{
  /**
    \brief A filter that performs a multiple threshold otsu image segmentation.

    This class being an mitk::ImageToImageFilter performs a multiple threshold otsu image segmentation based on the
    image histogram.
    Internally, the itk::OtsuMultipleThresholdsImageFilter is used.

    $Author: somebody$
  */
  class MITKSEGMENTATION_EXPORT OtsuSegmentationFilter : public ImageToImageFilter
  {
  public:
    typedef unsigned char OutputPixelType;
    typedef itk::Image<OutputPixelType, 3> itkOutputImageType;
    typedef mitk::ITKImageImport<itkOutputImageType> ImageConverterType;

    mitkClassMacro(OtsuSegmentationFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    itkGetMacro(NumberOfThresholds, unsigned int);

    void SetNumberOfThresholds(unsigned int number)
    {
      if (number < 1)
      {
        MITK_WARN << "Tried to set an invalid number of thresholds in the OtsuSegmentationFilter.";
        return;
      }
      m_NumberOfThresholds = number;
    }

    void SetValleyEmphasis(bool useValley) { m_ValleyEmphasis = useValley; }
    void SetNumberOfBins(unsigned int number)
    {
      if (number < 1)
      {
        MITK_WARN << "Tried to set an invalid number of bins in the OtsuSegmentationFilter.";
        return;
      }
      m_NumberOfBins = number;
    }

  protected:
    OtsuSegmentationFilter();
    ~OtsuSegmentationFilter() override;
    void GenerateData() override;
    // virtual void GenerateOutputInformation();

  private:
    unsigned int m_NumberOfThresholds;
    bool m_ValleyEmphasis;
    unsigned int m_NumberOfBins;

  }; // class

} // namespace
#endif
