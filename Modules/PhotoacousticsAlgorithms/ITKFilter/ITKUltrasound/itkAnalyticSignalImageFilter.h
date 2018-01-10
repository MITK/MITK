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
/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkAnalyticSignalImageFilter_h
#define itkAnalyticSignalImageFilter_h

#include <complex>

#include "itkFFT1DComplexToComplexImageFilter.h"
#include "itkFFT1DRealToComplexConjugateImageFilter.h"
#include "itkImageRegionSplitterDirection.h"

namespace itk
{
/** \class AnalyticSignalImageFilter
 * \brief Generates the analytic signal from one direction of an image.
 *
 * This filter generates the complex valued analytic signal along one direction
 * of an image.  This input is a real valued image, and the output is a complex
 * image.
 *
 * The analytic signal is given by
 *
 * f_a(x) = f(x) - i f_H(x)
 *
 * Where i is the square root of one and f_H(x) is the Hibert transform of f(x).
 *
 * Since the Hilbert transform in the Fourier domain is
 *
 * F_H(k) = F(k) i sign(k),
 *
 * f_a(x) is calculated by
 *
 * f_a(x) = F^{-1}( F(k) 2 U(k) )
 *
 * where U(k) is the unit step function.
 *
 * \ingroup FourierTransform
 * \ingroup Ultrasound
 */
template< typename TInputImage, typename TOutputImage >
class AnalyticSignalImageFilter:
  public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef TInputImage                                         InputImageType;
  typedef TOutputImage                                        OutputImageType;
  typedef typename OutputImageType::RegionType                OutputImageRegionType;

  itkStaticConstMacro(ImageDimension, unsigned int, InputImageType::ImageDimension);

  typedef AnalyticSignalImageFilter                             Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType > Superclass;
  typedef SmartPointer< Self >                                  Pointer;
  typedef SmartPointer< const Self >                            ConstPointer;

  itkTypeMacro( AnalyticSignalImageFilter, ImageToImageFilter );
  itkNewMacro( Self );

  /** Get the direction in which the filter is to be applied. */
  virtual unsigned int GetDirection() const
    {
    return this->m_FFTRealToComplexFilter->GetDirection();
    }

  /** Set the direction in which the filter is to be applied. */
  virtual void SetDirection( const unsigned int direction )
    {
    if( this->m_FFTRealToComplexFilter->GetDirection() != direction )
      {
      this->m_FFTRealToComplexFilter->SetDirection( direction );
      this->m_FFTComplexToComplexFilter->SetDirection( direction );
      this->Modified();
      }
    }

protected:
  AnalyticSignalImageFilter();
  virtual ~AnalyticSignalImageFilter() {}

  void PrintSelf(std::ostream& os, Indent indent) const ITK_OVERRIDE;

  // These behave like their analogs in FFT1DRealToComplexConjugateImageFilter.
  virtual void GenerateInputRequestedRegion() ITK_OVERRIDE;
  virtual void EnlargeOutputRequestedRegion(DataObject *output) ITK_OVERRIDE;

  virtual void BeforeThreadedGenerateData() ITK_OVERRIDE;
  virtual void ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, ThreadIdType threadId ) ITK_OVERRIDE;
  virtual void AfterThreadedGenerateData() ITK_OVERRIDE;

  typedef FFT1DRealToComplexConjugateImageFilter< InputImageType, OutputImageType > FFTRealToComplexType;
  typedef FFT1DComplexToComplexImageFilter< OutputImageType, OutputImageType >      FFTComplexToComplexType;

  typename FFTRealToComplexType::Pointer m_FFTRealToComplexFilter;
  typename FFTComplexToComplexType::Pointer m_FFTComplexToComplexFilter;

  /** Override to return a splitter that does not split along the direction we
   * are performing the transform. */
  virtual const ImageRegionSplitterBase* GetImageRegionSplitter() const ITK_OVERRIDE;

private:
  AnalyticSignalImageFilter( const Self& ); // purposely not implemented
  void operator=( const Self& ); // purposely not implemented

  ImageRegionSplitterDirection::Pointer m_ImageRegionSplitter;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAnalyticSignalImageFilter.hxx"
#endif

#endif // itkAnalyticSignalImageFilter_h
