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

#ifndef __itkRegularizedIVIMLocalVariationImageFilter_h
#define __itkRegularizedIVIMLocalVariationImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"

namespace itk
{

  template<class TPixelType>
  class IVIMSquaredEuclideanMetric
  {
  public:
    static double Calc(TPixelType p)
    {
      return p*p;
    }
  };

  template<>
  class IVIMSquaredEuclideanMetric<itk::Vector<double,3> >
  {
  public:
    static double Calc(itk::Vector<double,3> p)
    {
      return p[1]*p[1];
    }
  };

  template<>
  class IVIMSquaredEuclideanMetric<itk::VariableLengthVector<float> >
  {
  public:
    static double Calc(itk::VariableLengthVector<float> p)
    {
      return p.GetSquaredNorm();
    }
  };

  template<>
  class IVIMSquaredEuclideanMetric<itk::VariableLengthVector<double> >
  {
  public:
    static double Calc(itk::VariableLengthVector<double> p)
    {
      return p.GetSquaredNorm();
    }
  };

/** \class RegularizedIVIMLocalVariationImageFilter
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
class RegularizedIVIMLocalVariationImageFilter :
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

  /** Standard class typedefs. */
  typedef RegularizedIVIMLocalVariationImageFilter Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(RegularizedIVIMLocalVariationImageFilter, ImageToImageFilter);

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
  virtual void GenerateInputRequestedRegion()
    throw(InvalidRequestedRegionError);

protected:
  RegularizedIVIMLocalVariationImageFilter();
  virtual ~RegularizedIVIMLocalVariationImageFilter() {}
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

private:
  RegularizedIVIMLocalVariationImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkRegularizedIVIMLocalVariationImageFilter.txx"
#endif

#endif //RegularizedIVIMLocalVariationImageFilter
