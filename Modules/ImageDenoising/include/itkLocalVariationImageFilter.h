/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef itkLocalVariationImageFilter_h
#define itkLocalVariationImageFilter_h

#include <itkImage.h>
#include <itkImageToImageFilter.h>

namespace itk
{
  /**
   * \class SquaredEuclideanMetric
   * \brief Computes the squared Euclidean metric for a given pixel value.
   *
   * This helper class provides a static method to compute the squared Euclidean
   * distance of a pixel value from zero. Template specializations exist for
   * itk::VariableLengthVector<float> and itk::VariableLengthVector<double>,
   * which use GetSquaredNorm(). For scalar types, the result is simply p * p.
   *
   * \tparam TPixelType The pixel type for which to compute the metric.
   *
   * \sa LocalVariationImageFilter
   */
  template <class TPixelType>
  class SquaredEuclideanMetric
  {
  public:
    /**
     * \brief Compute the squared Euclidean metric of a pixel value.
     * \param[in] p The pixel value.
     * \return The squared Euclidean distance from zero for the given pixel value.
     */
    static double Calc(TPixelType p);
  };

  /** \class LocalVariationImageFilter
   * \brief Calculates the local variation in each pixel.
   *
   * For each pixel, this filter computes the local variation as the square root
   * of the sum of squared differences between the pixel and each of its
   * face-connected neighbors. A small epsilon (0.0001) is added inside the
   * square root for numerical stability. The filter uses zero-flux Neumann
   * boundary conditions.
   *
   * This filter is used internally by TotalVariationSingleIterationImageFilter
   * as part of the total variation denoising pipeline.
   *
   * Reference: Tony F. Chan et al., The digital TV filter and nonlinear denoising
   *
   * \tparam TInputImage The type of the input image.
   * \tparam TOutputImage The type of the output image.
   *
   * \sa TotalVariationSingleIterationImageFilter
   * \sa TotalVariationDenoisingImageFilter
   * \sa SquaredEuclideanMetric
   * \sa Image
   * \sa Neighborhood
   * \sa NeighborhoodOperator
   * \sa NeighborhoodIterator
   *
   * \ingroup IntensityImageFilters
   */
  template <class TInputImage, class TOutputImage>
  class LocalVariationImageFilter : public ImageToImageFilter<TInputImage, TOutputImage>
  {
  public:
    /** Extract dimension from input and output image. */
    itkStaticConstMacro(InputImageDimension, unsigned int, TInputImage::ImageDimension);
    itkStaticConstMacro(OutputImageDimension, unsigned int, TOutputImage::ImageDimension);

    /** Convenient typedefs for simplifying declarations. */
    typedef TInputImage InputImageType;
    typedef TOutputImage OutputImageType;

    /** Standard class typedefs. */
    typedef LocalVariationImageFilter Self;
    typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** Run-time type information (and related methods). */
      itkTypeMacro(LocalVariationImageFilter, ImageToImageFilter);

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
     * requested region because it accesses face-connected neighbors. The
     * input region is padded by 1 pixel in each direction.
     *
     * \throw itk::InvalidRequestedRegionError if the padded region falls
     *        outside the input's largest possible region.
     *
     * \sa ImageToImageFilter::GenerateInputRequestedRegion()
     */
    void GenerateInputRequestedRegion() override;

  protected:
    LocalVariationImageFilter();
    ~LocalVariationImageFilter() override {}
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

  private:
    LocalVariationImageFilter(const Self &); // purposely not implemented
    void operator=(const Self &);            // purposely not implemented
  };

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include <itkLocalVariationImageFilter.tpp>
#endif

#endif // LocalVariationImageFilter
