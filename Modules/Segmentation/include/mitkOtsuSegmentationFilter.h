/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkOtsuSegmentationFilter_h
#define mitkOtsuSegmentationFilter_h

//#include "MitkSBExports.h"
#include <mitkITKImageImport.h>
#include <mitkImage.h>
#include <mitkImageToImageFilter.h>

#include <itkImage.h>

#include <MitkSegmentationExports.h>

namespace mitk
{
  /**
    \brief A filter that performs a multiple threshold otsu image segmentation.

    This class being an mitk::ImageToImageFilter performs a multiple threshold otsu image segmentation based on the
    image histogram.
    Internally, the itk::OtsuMultipleThresholdsImageFilter is used.

    @remark In contrast to the itk filter. The MITK version generates classes starting with the pixel value 1
    (and not 0 like the itk version). MITK uses 0 in Segmentation to indicate unlabeled pixel, but after otsu
    every pixel has a proposed class.
  */
  class MITKSEGMENTATION_EXPORT OtsuSegmentationFilter : public ImageToImageFilter
  {
  public:
    typedef unsigned short OutputPixelType;
    typedef itk::Image<OutputPixelType, 3> itkOutputImageType;
    typedef mitk::ITKImageImport<itkOutputImageType> ImageConverterType;

    mitkClassMacro(OtsuSegmentationFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns the number of thresholds to compute. */
    itkGetMacro(NumberOfThresholds, unsigned int);

    /**
     * \brief Sets the number of thresholds for Otsu's method.
     *
     * The number of resulting classes will be NumberOfThresholds + 1.
     *
     * \param[in] number Number of thresholds, must be >= 1.
     * \note Values less than 1 are ignored with a warning.
     */
    void SetNumberOfThresholds(unsigned int number)
    {
      if (number < 1)
      {
        MITK_WARN << "Tried to set an invalid number of thresholds in the OtsuSegmentationFilter.";
        return;
      }
      m_NumberOfThresholds = number;
    }

    /**
     * \brief Enables or disables valley emphasis in the Otsu threshold computation.
     * \param[in] useValley If true, valley emphasis is used for better threshold selection
     *            in images with unequal class variances.
     */
    void SetValleyEmphasis(bool useValley) { m_ValleyEmphasis = useValley; }

    /**
     * \brief Sets the number of histogram bins for Otsu's threshold computation.
     * \param[in] number Number of bins, must be >= 1.
     * \note Values less than 1 are ignored with a warning.
     */
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
