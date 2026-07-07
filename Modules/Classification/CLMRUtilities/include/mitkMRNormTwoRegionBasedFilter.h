/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMRNormTwoRegionBasedFilter_h
#define mitkMRNormTwoRegionBasedFilter_h

#include <mitkCommon.h>
#include <MitkCLMRUtilitiesExports.h>
#include <mitkImageToImageFilter.h>
#include <mitkImageTimeSelector.h>

#include <itkImage.h>

namespace mitk {
  /**
   * \brief Normalizes MR images using linear scaling based on statistics from two distinct mask regions.
   *
   * This filter computes a statistical measure (mean, median, or mode) in each of two
   * masked regions and normalizes the input image linearly between the two values:
   * output = (input - min(value1, value2)) / (max(value1, value2) - min(value1, value2)).
   *
   * \ingroup Process
   */
  class MITKCLMRUTILITIES_EXPORT MRNormTwoRegionsBasedFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(MRNormTwoRegionsBasedFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Set the first mask image defining the first reference region.
     * \param mask The mask image. Voxels with label 1 are included.
     */
    void SetMask1( const mitk::Image* mask );

    /**
     * \brief Set the second mask image defining the second reference region.
     * \param mask The mask image. Voxels with label 1 are included.
     */
    void SetMask2( const mitk::Image* mask );

    /**
     * \brief Get the first mask image.
     * \return Pointer to the first mask image, or nullptr if not set.
     */
    const mitk::Image* GetMask1() const;

    /**
     * \brief Get the second mask image.
     * \return Pointer to the second mask image, or nullptr if not set.
     */
    const mitk::Image* GetMask2() const;

    /** \brief Enumeration for the statistical measure used for normalization. */
    enum NormalizationBase
    {
      MEAN,   ///< Use the arithmetic mean.
      MODE,   ///< Use the histogram mode.
      MEDIAN  ///< Use the median.
    };

    itkGetConstMacro(Area1, NormalizationBase);
    itkGetConstMacro(Area2, NormalizationBase);
    itkSetMacro(Area1, NormalizationBase);
    itkSetMacro(Area2, NormalizationBase);

  protected:
    MRNormTwoRegionsBasedFilter();

    ~MRNormTwoRegionsBasedFilter() override;

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    /** \brief Internal templated implementation that computes statistics and normalizes the image. */
    template < typename TPixel, unsigned int VImageDimension >
    void InternalComputeMask(itk::Image<TPixel, VImageDimension>* itkImage);

    NormalizationBase m_Area1;
    NormalizationBase m_Area2;
  };
} // namespace mitk

#endif
