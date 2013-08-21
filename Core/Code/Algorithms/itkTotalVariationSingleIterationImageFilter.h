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

#ifndef __itkTotalVariationSingleIterationImageFilter_h
#define __itkTotalVariationSingleIterationImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"

namespace itk
{
/** \class TotalVariationSingleIterationImageFilter
 * \brief Applies a total variation denoising filter to an image
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
class TotalVariationSingleIterationImageFilter :
    public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Extract dimension from input and output image. */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      TOutputImage::ImageDimension);

  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage InputImageType;
  typedef TOutputImage OutputImageType;

  typedef itk::Image<float,InputImageDimension> LocalVariationImageType;

  /** Standard class typedefs. */
  typedef TotalVariationSingleIterationImageFilter Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(TotalVariationSingleIterationImageFilter, ImageToImageFilter);

  /** Image typedef support. */
  typedef typename InputImageType::PixelType InputPixelType;
  typedef typename OutputImageType::PixelType OutputPixelType;

  typedef typename InputImageType::RegionType InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  typedef typename InputImageType::SizeType InputSizeType;

  /** A larger input requested region than
   * the output requested region is required.
   * Therefore, an implementation for GenerateInputRequestedRegion()
   * is provided.
   *
   * \sa ImageToImageFilter::GenerateInputRequestedRegion() */
  virtual void GenerateInputRequestedRegion()
    throw(InvalidRequestedRegionError);

  itkSetMacro(Lambda, double);
  itkGetMacro(Lambda, double);

  void SetOriginalImage(InputImageType* in)
  { this->m_OriginalImage = in; }
  typename InputImageType::Pointer GetOriginialImage()
  { return this->m_OriginalImage; }

protected:
  TotalVariationSingleIterationImageFilter();
  virtual ~TotalVariationSingleIterationImageFilter() {}
  void PrintSelf(std::ostream& os, Indent indent) const;

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
  void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                            ThreadIdType threadId );

  void BeforeThreadedGenerateData();

  typename LocalVariationImageType::Pointer m_LocalVariation;

  typename InputImageType::Pointer m_OriginalImage;

  double m_Lambda;

private:

  TotalVariationSingleIterationImageFilter(const Self&);

  void operator=(const Self&);

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTotalVariationSingleIterationImageFilter.txx"
#endif

#endif //__itkTotalVariationSingleIterationImageFilter__
