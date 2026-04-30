/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef itkTotalVariationSingleIterationImageFilter_h
#define itkTotalVariationSingleIterationImageFilter_h

#include <itkImage.h>
#include <itkImageToImageFilter.h>

namespace itk
{
  /** \class TotalVariationSingleIterationImageFilter
   * \brief Performs a single iteration of total variation denoising.
   *
   * This filter implements one step of the iterative total variation (TV)
   * denoising algorithm. Given the current estimate of the denoised image
   * (as the filter input) and the original noisy image (set via
   * SetOriginalImage()), it computes a weighted average at each pixel
   * that balances fidelity to the original with local smoothness.
   *
   * The weighting is controlled by the Lambda parameter and the local
   * variation in the image (computed via LocalVariationImageFilter in
   * BeforeThreadedGenerateData()). Larger Lambda values favor fidelity
   * to the original image.
   *
   * This filter is typically not used directly but is called iteratively
   * by TotalVariationDenoisingImageFilter.
   *
   * Reference: Tony F. Chan et al., The digital TV filter and nonlinear denoising
   *
   * \tparam TInputImage The type of the input image.
   * \tparam TOutputImage The type of the output image.
   *
   * \sa TotalVariationDenoisingImageFilter
   * \sa LocalVariationImageFilter
   * \sa Image
   * \sa Neighborhood
   * \sa NeighborhoodOperator
   * \sa NeighborhoodIterator
   *
   * \ingroup IntensityImageFilters
   */
  template <class TInputImage, class TOutputImage>
  class TotalVariationSingleIterationImageFilter : public ImageToImageFilter<TInputImage, TOutputImage>
  {
  public:
    /** Extract dimension from input and output image. */
    itkStaticConstMacro(InputImageDimension, unsigned int, TInputImage::ImageDimension);
    itkStaticConstMacro(OutputImageDimension, unsigned int, TOutputImage::ImageDimension);

    /** Convenient typedefs for simplifying declarations. */
    typedef TInputImage InputImageType;
    typedef TOutputImage OutputImageType;

    /** Float image type used to store the local variation at each pixel. */
    typedef itk::Image<float, InputImageDimension> LocalVariationImageType;

    /** Standard class typedefs. */
    typedef TotalVariationSingleIterationImageFilter Self;
    typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** Run-time type information (and related methods). */
      itkTypeMacro(TotalVariationSingleIterationImageFilter, ImageToImageFilter);

    /** Image typedef support. */
    typedef typename InputImageType::PixelType InputPixelType;
    typedef typename OutputImageType::PixelType OutputPixelType;

    typedef typename InputImageType::RegionType InputImageRegionType;
    typedef typename OutputImageType::RegionType OutputImageRegionType;

    typedef typename InputImageType::SizeType InputSizeType;

    /**
     * \brief Generates the input requested region, padded by a radius of 1.
     *
     * This filter requires a larger input requested region than the output
     * requested region because it accesses face-connected neighbors.
     *
     * \throw itk::InvalidRequestedRegionError if the padded region falls
     *        outside the input's largest possible region.
     *
     * \sa ImageToImageFilter::GenerateInputRequestedRegion()
     */
    void GenerateInputRequestedRegion() override;

    /**
     * \brief Set/Get the regularization parameter Lambda.
     *
     * Lambda controls the trade-off between fidelity to the original image
     * and smoothness in the denoised output. Default is 1.0.
     */
    itkSetMacro(Lambda, double);
    itkGetMacro(Lambda, double);

    /**
     * \brief Set the original (undenoised) image used as a reference
     *        for the data fidelity term.
     * \param[in] in Pointer to the original input image.
     */
    void SetOriginalImage(InputImageType *in) { this->m_OriginalImage = in; }

    /**
     * \brief Get the original (undenoised) image.
     * \return Pointer to the original image.
     */
    typename InputImageType::Pointer GetOriginialImage() { return this->m_OriginalImage; }
  protected:
    TotalVariationSingleIterationImageFilter();
    ~TotalVariationSingleIterationImageFilter() override {}
    void PrintSelf(std::ostream &os, Indent indent) const override;

    /** MedianImageFilter can be implemented as a multithreaded filter.
     * Therefore, this implementation provides a ThreadedGenerateData()
     * routine which is called for each processing thread. The output
     * image data is allocated automatically by the superclass prior to
     * calling ThreadedGenerateData().  ThreadedGenerateData can only
     * write to the portion of the output image specified by the
     * parameter "outputRegionForThread"
     *
     * \sa ImageToImageFilter::ThreadedGenerateData(),
     *     ImageToImageFilter::GenerateData() */
    void ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId) override;

    void BeforeThreadedGenerateData() override;

    typename LocalVariationImageType::Pointer m_LocalVariation;

    typename InputImageType::Pointer m_OriginalImage;

    double m_Lambda;

  private:
    TotalVariationSingleIterationImageFilter(const Self &);

    void operator=(const Self &);
  };

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include <itkTotalVariationSingleIterationImageFilter.tpp>
#endif

#endif //__itkTotalVariationSingleIterationImageFilter__
