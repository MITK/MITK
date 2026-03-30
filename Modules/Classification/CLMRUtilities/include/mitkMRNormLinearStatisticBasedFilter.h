/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMRNormLinearStatisticBasedFilter_h
#define mitkMRNormLinearStatisticBasedFilter_h

#include <mitkCommon.h>
#include <MitkCLMRUtilitiesExports.h>
#include <mitkImageToImageFilter.h>
#include <mitkImageTimeSelector.h>

#include <itkImage.h>

namespace mitk {
  /**
   * \brief Normalizes MR images using linear scaling based on statistical measures within a mask region.
   *
   * This filter computes a statistical center (mean, median, or mode) and the standard deviation
   * within a masked region of the input image, then applies a linear transformation to normalize
   * the image: output = (input - (center + TargetValue)) / (stddev * TargetWidth).
   * Optionally, outlier voxels (below 2nd or above 98th percentile) can be clamped.
   *
   * \ingroup Process
   */
  class MITKCLMRUTILITIES_EXPORT MRNormLinearStatisticBasedFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(MRNormLinearStatisticBasedFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Set the mask image defining the region used for computing statistics.
     * \param mask The mask image. Voxels with label 1 are included.
     */
    void SetMask( const mitk::Image* mask );

    /**
     * \brief Get the mask image used for computing statistics.
     * \return Pointer to the mask image, or nullptr if not set.
     */
    const mitk::Image* GetMask() const;

    /** \brief Enumeration for the statistical measure used as normalization center. */
    enum NormalizationBase
    {
      MEAN,   ///< Use the arithmetic mean as center.
      MODE,   ///< Use the histogram mode as center.
      MEDIAN  ///< Use the median as center.
    };

    itkGetConstMacro(CenterMode, NormalizationBase);
    itkSetMacro(CenterMode, NormalizationBase);

    itkGetConstMacro(IgnoreOutlier, bool);
    itkSetMacro(IgnoreOutlier, bool);

    itkGetConstMacro(TargetValue, double);
    itkSetMacro(TargetValue, double);

    itkGetConstMacro(TargetWidth, double);
    itkSetMacro(TargetWidth, double);

  protected:
    MRNormLinearStatisticBasedFilter();

    ~MRNormLinearStatisticBasedFilter() override;

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    /** \brief Internal templated implementation that computes statistics and normalizes the image. */
    template < typename TPixel, unsigned int VImageDimension >
    void InternalComputeMask(itk::Image<TPixel, VImageDimension>* itkImage);

    NormalizationBase m_CenterMode;
    bool m_IgnoreOutlier;
  private:
    double m_TargetValue;
    double m_TargetWidth;

  };
} // namespace mitk

#endif
