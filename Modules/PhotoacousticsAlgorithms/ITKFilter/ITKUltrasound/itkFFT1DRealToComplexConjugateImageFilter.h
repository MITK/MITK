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
#ifndef itkFFT1DRealToComplexConjugateImageFilter_h
#define itkFFT1DRealToComplexConjugateImageFilter_h

#include <complex>

#include "itkImageToImageFilter.h"
#include "itkImageRegionSplitterDirection.h"

namespace itk
{
/** \class FFT1DRealToComplexConjugateImageFilter
 * \brief Perform the Fast Fourier Transform, in the forward direction, with
 * real inputs, but only along one dimension.
 *
 * \ingroup FourierTransform
 * \ingroup Ultrasound
 */
template< typename TInputImage, typename TOutputImage=Image< std::complex< typename TInputImage::PixelType >, TInputImage::ImageDimension > >
class FFT1DRealToComplexConjugateImageFilter:
  public ImageToImageFilter< TInputImage, TOutputImage >
{
public:

  /** Standard class typedefs. */
  typedef TInputImage                                           InputImageType;
  typedef TOutputImage                                          OutputImageType;
  typedef typename OutputImageType::RegionType                  OutputImageRegionType;

  typedef FFT1DRealToComplexConjugateImageFilter                Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType > Superclass;
  typedef SmartPointer< Self >                                  Pointer;
  typedef SmartPointer< const Self >                            ConstPointer;

  itkStaticConstMacro( ImageDimension, unsigned int, InputImageType::ImageDimension );

  itkTypeMacro( FFT1DRealToComplexConjugateImageFilter, ImageToImageFilter );

  /** Customized object creation methods that support configuration-based
    * selection of FFT implementation.
    *
    * Default implementation is VnlFFT1D.
    */
  static Pointer New();

  /** Get the direction in which the filter is to be applied. */
  itkGetMacro(Direction, unsigned int);

  /** Set the direction in which the filter is to be applied. */
  itkSetClampMacro(Direction, unsigned int, 0, ImageDimension - 1);

protected:
  FFT1DRealToComplexConjugateImageFilter();
  virtual ~FFT1DRealToComplexConjugateImageFilter() {}

  void PrintSelf(std::ostream& os, Indent indent) const ITK_OVERRIDE;

  virtual void GenerateInputRequestedRegion() ITK_OVERRIDE;
  virtual void EnlargeOutputRequestedRegion(DataObject *output) ITK_OVERRIDE;

  virtual void BeforeThreadedGenerateData() ITK_OVERRIDE;

  /** Override to return a splitter that does not split along the direction we
   * are performing the transform. */
  virtual const ImageRegionSplitterBase* GetImageRegionSplitter() const ITK_OVERRIDE;

private:
  FFT1DRealToComplexConjugateImageFilter( const Self& );
  void operator=( const Self& );

  ImageRegionSplitterDirection::Pointer m_ImageRegionSplitter;

  /** Direction in which the filter is to be applied
   * this should be in the range [0,ImageDimension-1]. */
  unsigned int m_Direction;

};
}

#ifndef ITK_MANUAL_INSTANTIATION
#ifndef itkVnlFFT1DRealToComplexConjugateImageFilter_h
#ifndef itkVnlFFT1DRealToComplexConjugateImageFilter_hxx
#ifndef itkFFTW1DRealToComplexConjugateImageFilter_h
#ifndef itkFFTW1DRealToComplexConjugateImageFilter_hxx
#include "itkFFT1DRealToComplexConjugateImageFilter.hxx"
#endif
#endif
#endif
#endif
#endif

#endif // itkFFT1DRealToComplexConjugateImageFilter_h
