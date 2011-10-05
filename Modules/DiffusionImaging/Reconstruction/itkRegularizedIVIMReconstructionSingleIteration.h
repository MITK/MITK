/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkRegularizedIVIMReconstructionSingleIteration_h
#define __itkRegularizedIVIMReconstructionSingleIteration_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include "itkVectorImage.h"

namespace itk
{
/** \class RegularizedIVIMReconstructionSingleIteration
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
template <class TInputPixel, class TOutputPixel, class TRefPixelType>
class RegularizedIVIMReconstructionSingleIteration :
    public ImageToImageFilter< itk::Image<itk::Vector<TInputPixel,3>, 3>, itk::Image<itk::Vector<TOutputPixel,3>, 3> >
{
public:
  /** Convenient typedefs for simplifying declarations. */
  typedef itk::Image<itk::Vector<TInputPixel,3>, 3> InputImageType;
  typedef itk::Image<itk::Vector<TOutputPixel,3>, 3> OutputImageType;
  typedef itk::VectorImage<TRefPixelType,3> RefImageType;

  /** Extract dimension from input and output image. */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      InputImageType::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      OutputImageType::ImageDimension);

  typedef itk::Image<float,InputImageDimension> LocalVariationImageType;

  /** Standard class typedefs. */
  typedef RegularizedIVIMReconstructionSingleIteration Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(RegularizedIVIMReconstructionSingleIteration, ImageToImageFilter);
  
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

  void SetBValues(vnl_vector<double> bvals)
  { this->m_BValues = bvals; }
  vnl_vector<double> GetBValues()
  { return this->m_BValues; }

  void SetOriginalImage(RefImageType* in)
  { this->m_OriginalImage = in; }
  typename RefImageType::Pointer GetOriginialImage()
  { return this->m_OriginalImage; }

protected:
  RegularizedIVIMReconstructionSingleIteration();
  virtual ~RegularizedIVIMReconstructionSingleIteration() {}
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
                            int threadId );

  void BeforeThreadedGenerateData();

  typename LocalVariationImageType::Pointer m_LocalVariation;

  typename RefImageType::Pointer m_OriginalImage;

  double m_Lambda;

  vnl_vector<double> m_BValues;

private:

  RegularizedIVIMReconstructionSingleIteration(const Self&);

  void operator=(const Self&);

};
  
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkRegularizedIVIMReconstructionSingleIteration.txx"
#endif

#endif //__itkRegularizedIVIMReconstructionSingleIteration__
