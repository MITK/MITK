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
#ifndef itkFFT1DComplexConjugateToRealImageFilter_h
#define itkFFT1DComplexConjugateToRealImageFilter_h

#include <complex>

#include "itkImageToImageFilter.h"
#include "itkImageRegionSplitterDirection.h"

namespace itk
{
/** \class FFT1DComplexConjugateToRealImageFilter
 * \brief Perform the Fast Fourier Transform, in the reverse direction, with
 * real output, but only along one dimension.
 *
 * \ingroup FourierTransform
 * \ingroup Ultrasound
 */
template< typename TInputImage, typename TOutputImage=Image< typename NumericTraits< typename TInputImage::PixelType >::ValueType, TInputImage::ImageDimension > >
class FFT1DComplexConjugateToRealImageFilter:
  public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef TInputImage                          InputImageType;
  typedef TOutputImage                         OutputImageType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  typedef FFT1DComplexConjugateToRealImageFilter                Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType > Superclass;
  typedef SmartPointer< Self >                                  Pointer;
  typedef SmartPointer< const Self >                            ConstPointer;

  itkStaticConstMacro(ImageDimension, unsigned int, InputImageType::ImageDimension );

  itkTypeMacro( FFT1DComplexConjugateToRealImageFilter, ImageToImageFilter );

  /** Customized object creation methods that support configuration-based
    * selection of FFT implementation.
    *
    * Default implementation is VnlFFT1D.
    */
  static Pointer New(void);

  /** Get the direction in which the filter is to be applied. */
  itkGetMacro(Direction, unsigned int);

  /** Set the direction in which the filter is to be applied. */
  itkSetClampMacro(Direction, unsigned int, 0, ImageDimension - 1);

protected:
  FFT1DComplexConjugateToRealImageFilter();
  ~FFT1DComplexConjugateToRealImageFilter() override {}

  void PrintSelf(std::ostream& os, Indent indent) const ITK_OVERRIDE;

  void GenerateInputRequestedRegion() ITK_OVERRIDE;
  void EnlargeOutputRequestedRegion(DataObject *output) ITK_OVERRIDE;

  void BeforeThreadedGenerateData() ITK_OVERRIDE;

  /** Override to return a splitter that does not split along the direction we
   * are performing the transform. */
  const ImageRegionSplitterBase* GetImageRegionSplitter() const ITK_OVERRIDE;

  /** Direction in which the filter is to be applied
   * this should be in the range [0,ImageDimension-1]. */
  unsigned int m_Direction;

private:
  FFT1DComplexConjugateToRealImageFilter( const Self& );
  void operator=( const Self& );

  ImageRegionSplitterDirection::Pointer m_ImageRegionSplitter;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#ifndef itkVnlFFT1DComplexConjugateToRealImageFilter_h
#ifndef itkVnlFFT1DComplexConjugateToRealImageFilter_hxx
#ifndef itkFFTW1DComplexConjugateToRealImageFilter_h
#ifndef itkFFTW1DComplexConjugateToRealImageFilter_hxx
#include "itkFFT1DComplexConjugateToRealImageFilter.hxx"
#endif
#endif
#endif
#endif
#endif

#endif // itkFFT1DComplexConjugateToRealImageFilter_h
