/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __itkMaskedNaryStatisticsImageFilter_h
#define __itkMaskedNaryStatisticsImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkImageIterator.h"
#include "itkArray.h"

namespace itk
{
/** \class MaskedNaryStatisticsImageFilter
 * \brief Computes a masked statistic on N images and produces vectors of those statistic results.
 *
 * All the input images must be of the same type.
 *
 * \ingroup IntensityImageFilters
 * \ingroup ITKImageIntensity
 */

template< class TInputImage, class TMaskImage = ::itk::Image<unsigned char, TInputImage::ImageDimension> >
class ITK_EXPORT MaskedNaryStatisticsImageFilter:
  public ImageToImageFilter< TInputImage, TInputImage >

{
public:
  /** Standard class typedefs. */
  typedef MaskedNaryStatisticsImageFilter                 Self;
  typedef ImageToImageFilter< TInputImage, TInputImage >  Superclass;
  typedef SmartPointer< Self >                            Pointer;
  typedef SmartPointer< const Self >                      ConstPointer;
  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(MaskedNaryStatisticsImageFilter, ImageToImageFilter);

  /** Some typedefs. */
  typedef TInputImage                          InputImageType;
  typedef typename InputImageType::Pointer     InputImagePointer;
  typedef typename InputImageType::RegionType  InputImageRegionType;
  typedef typename InputImageType::PixelType   InputImagePixelType;
  typedef typename NumericTraits< InputImagePixelType >::RealType RealType;

  typedef std::vector<InputImagePixelType> PixelVectorType;
  typedef std::vector<RealType> RealVectorType;

  typedef TMaskImage MaskImageType;
  typedef typename MaskImageType::Pointer      MaskImagePointer;
  typedef typename MaskImageType::ConstPointer MaskImageConstPointer;
  typedef typename MaskImageType::RegionType   MaskImageRegionType;

  itkSetConstObjectMacro(Mask, MaskImageType);
  itkGetConstObjectMacro(Mask, MaskImageType);

  itkGetConstReferenceMacro(Mean,RealVectorType);
  itkGetConstReferenceMacro(Sigma,RealVectorType);
  itkGetConstReferenceMacro(Variance,RealVectorType);
  itkGetConstReferenceMacro(Sum,RealVectorType);
  itkGetConstReferenceMacro(Minimum,PixelVectorType);
  itkGetConstReferenceMacro(Maximum,PixelVectorType);

  /** ImageDimension constants */
  itkStaticConstMacro(
    InputImageDimension, unsigned int, TInputImage::ImageDimension);

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro( SameDimensionCheck,
                   ( Concept::SameDimension< InputImageDimension, InputImageDimension > ) );
  itkConceptMacro( OutputHasZeroCheck,
                   ( Concept::HasZero< InputImagePixelType > ) );
  /** End concept checking */
#endif
protected:
  MaskedNaryStatisticsImageFilter();
  ~MaskedNaryStatisticsImageFilter() override {}

  void GenerateData() override;

private:
  MaskedNaryStatisticsImageFilter(const Self &); //purposely not implemented
  void operator=(const Self &);         //purposely not implemented

  RealVectorType      m_Mean;
  RealVectorType      m_Sigma;
  RealVectorType      m_Variance;
  RealVectorType      m_Sum;
  PixelVectorType      m_Minimum;
  PixelVectorType      m_Maximum;

  MaskImageConstPointer m_Mask;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMaskedNaryStatisticsImageFilter.hxx"
#endif

#endif
