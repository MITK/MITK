/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __itkLocalVariationImageFilter_h
#define __itkLocalVariationImageFilter_h

#include "itkImage.h"
#include "itkImageToImageFilter.h"

namespace itk
{
  template <class TPixelType>
  class SquaredEuclideanMetric
  {
  public:
    static double Calc(TPixelType p);
  };

  /** \class LocalVariationImageFilter
   * \brief Calculates the local variation in each pixel
   *
   * Reference: Tony F. Chan et al., The digital TV filter and nonlinear denoising
   *
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

    /** MedianImageFilter needs a larger input requested region than
     * the output requested region.  As such, MedianImageFilter needs
     * to provide an implementation for GenerateInputRequestedRegion()
     * in order to inform the pipeline execution model.
     *
     * \sa ImageToImageFilter::GenerateInputRequestedRegion() */
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
#include "itkLocalVariationImageFilter.txx"
#endif

#endif // LocalVariationImageFilter
