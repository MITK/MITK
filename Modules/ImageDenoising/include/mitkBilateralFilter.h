/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkBilateralFilter_h
#define mitkBilateralFilter_h

#include <MitkImageDenoisingExports.h>

// MITK
#include <mitkImageToImageFilter.h>
#include <itkImage.h>
#include <mitkImage.h>

namespace mitk
{
  /**
   * \brief Edge-preserving smoothing filter using bilateral filtering.
   *
   * This filter wraps the ITK BilateralImageFilter and applies it to MITK
   * images. Bilateral filtering smooths an image while preserving edges by
   * combining a spatial (domain) Gaussian with a range (intensity) Gaussian.
   * Pixels that are both spatially close and have similar intensities are
   * averaged together, while pixels across edges (with large intensity
   * differences) are not.
   *
   * The filter supports 2D, 2D+t, 3D, 3D+t, and 4D images. Input images
   * with fewer than 2 or more than 4 dimensions are rejected.
   *
   * Default parameter values: DomainSigma = 2.0, RangeSigma = 50.0,
   * AutoKernel = true, KernelRadius = 1.
   *
   * \sa itk::BilateralImageFilter
   * \sa ImageToImageFilter
   *
   * \ingroup ImageFilters
   */
  class MITKIMAGEDENOISING_EXPORT BilateralFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(BilateralFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Set/Get the sigma of the spatial domain Gaussian kernel.
     *
     * Controls how far apart spatially neighboring pixels can be to still
     * influence each other. Larger values produce more spatial smoothing.
     * Default is 2.0.
     */
    itkSetMacro(DomainSigma, float);

    /**
     * \brief Set/Get the sigma of the range (intensity) Gaussian kernel.
     *
     * Controls how different in intensity two pixels can be to still
     * influence each other. Larger values allow pixels with bigger intensity
     * differences to be averaged. Default is 50.0.
     */
    itkSetMacro(RangeSigma, float);

    /**
     * \brief Set/Get whether the kernel size is automatically determined.
     *
     * When true, the kernel size is calculated from DomainSigma by ITK's
     * BilateralImageFilter. When false, the kernel size is set manually
     * via KernelRadius. Default is true.
     */
    itkSetMacro(AutoKernel, bool);

    /**
     * \brief Set/Get the manual kernel radius (used when AutoKernel is false).
     *
     * Specifies the radius of the kernel in pixels. Only effective when
     * AutoKernel is set to false. Default is 1.
     */
    itkSetMacro(KernelRadius, unsigned int);

    /** \brief Get the spatial domain sigma. */
    itkGetMacro(DomainSigma, float);
    /** \brief Get the range (intensity) sigma. */
    itkGetMacro(RangeSigma, float);
    /** \brief Get whether automatic kernel sizing is enabled. */
    itkGetMacro(AutoKernel, bool);
    /** \brief Get the manual kernel radius. */
    itkGetMacro(KernelRadius, unsigned int);

  protected:
    /*!
    \brief standard constructor
    */
    BilateralFilter();
    /*!
    \brief standard destructor
    */
    ~BilateralFilter() override;
    /*!
    \brief Method generating the output information of this filter (e.g. image dimension, image type, etc.).
    The interface ImageToImageFilter requires this implementation. Everything is taken from the input image.
    */
    void GenerateOutputInformation() override;
    /*!
    \brief Method generating the output of this filter. Called in the updated process of the pipeline.
    This method generates the smoothed output image.
    */
    void GenerateData() override;

    /*!
    \brief Internal templated method calling the ITK bilteral filter. Here the actual filtering is performed.
    */
    template <typename TPixel, unsigned int VImageDimension>
    void ItkImageProcessing(const itk::Image<TPixel, VImageDimension> *itkImage);

    float m_DomainSigma;        ///< Sigma of the spatial domain Gaussian kernel.
    float m_RangeSigma;         ///< Sigma of the range (intensity) Gaussian kernel.
    bool m_AutoKernel;          ///< If true, kernel size is computed from DomainSigma; if false, m_KernelRadius is used.
    unsigned int m_KernelRadius; ///< Manual kernel radius, used when m_AutoKernel is false.
  };
} // END mitk namespace
#endif
