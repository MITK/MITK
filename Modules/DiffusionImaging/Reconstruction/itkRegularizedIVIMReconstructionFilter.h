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

#ifndef __itkRegularizedIVIMReconstructionFilter_h
#define __itkRegularizedIVIMReconstructionFilter_h

#include "itkRegularizedIVIMReconstructionSingleIteration.h"
#include "itkImageToImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkImage.h"

namespace itk
{
/** \class RegularizedIVIMReconstructionFilter
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
class RegularizedIVIMReconstructionFilter :
public ImageToImageFilter< itk::Image<itk::Vector<TInputPixel,3>, 3>, itk::Image<itk::Vector<TOutputPixel,3>, 3> >
{
public:

  typedef TInputPixel InputPixelType;
  typedef TOutputPixel OutputPixelType;
  typedef TRefPixelType RefPixelType;

  /** Convenient typedefs for simplifying declarations. */
  typedef itk::Image<itk::Vector<TInputPixel,3>, 3> InputImageType;
  typedef itk::Image<itk::Vector<TOutputPixel,3>, 3> OutputImageType;
  typedef itk::VectorImage<TRefPixelType,3> RefImageType;

  /** Image typedef support. */
  typedef typename InputImageType::PixelType InputVectorType;
  typedef typename OutputImageType::PixelType OutputVectorType;
  typedef typename RefImageType::PixelType RefVectorType;

  /** Extract dimension from input and output image. */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      InputImageType::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      InputImageType::ImageDimension);

  /** Standard class typedefs. */
  typedef RegularizedIVIMReconstructionFilter Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(RegularizedIVIMReconstructionFilter, ImageToImageFilter);

  typedef typename InputImageType::RegionType InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;
  typedef typename RefImageType::RegionType RefImageRegionType;

  typedef typename InputImageType::SizeType InputSizeType;

  typedef RegularizedIVIMReconstructionSingleIteration
      <OutputPixelType, OutputPixelType, RefPixelType>
    SingleIterationFilterType;

  typedef typename itk::CastImageFilter< InputImageType, OutputImageType > CastType;

  itkSetMacro(Lambda, double);
  itkGetMacro(Lambda, double);

  itkSetMacro(NumberIterations, int);
  itkGetMacro(NumberIterations, int);

  void SetBValues( vnl_vector<double> bvals )
  { this->m_BValues = bvals; }
  vnl_vector<double> GetBValues()
  { return this->m_BValues; }

  void SetReferenceImage( typename RefImageType::Pointer refimg )
  { this->m_ReferenceImage = refimg; }
  typename RefImageType::Pointer GetReferenceImage()
  { return this->m_ReferenceImage; }

protected:
  RegularizedIVIMReconstructionFilter();
  virtual ~RegularizedIVIMReconstructionFilter() {}
  void PrintSelf(std::ostream& os, Indent indent) const;

  void GenerateData();

  double m_Lambda;

  int m_NumberIterations;

  typename RefImageType::Pointer m_ReferenceImage;

  vnl_vector<double> m_BValues;

private:
  RegularizedIVIMReconstructionFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkRegularizedIVIMReconstructionFilter.txx"
#endif

#endif //__itkRegularizedIVIMReconstructionFilter__
