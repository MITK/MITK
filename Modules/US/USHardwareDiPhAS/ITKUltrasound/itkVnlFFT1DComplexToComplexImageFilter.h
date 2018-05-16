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
#ifndef itkVnlFFT1DComplexToComplexImageFilter_h
#define itkVnlFFT1DComplexToComplexImageFilter_h

#include "itkFFT1DComplexToComplexImageFilter.h"
#include <complex>

namespace itk
{

/** \class VnlFFT1DComplexToComplexImageFilter
 *
 * \brief Perform the FFT along one dimension of an image using Vnl as a
 * backend.
 *
 * \ingroup Ultrasound
 */
template< typename TInputImage, typename TOutputImage >
class VnlFFT1DComplexToComplexImageFilter:
    public FFT1DComplexToComplexImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef VnlFFT1DComplexToComplexImageFilter                           Self;
  typedef FFT1DComplexToComplexImageFilter< TInputImage, TOutputImage > Superclass;
  typedef SmartPointer< Self >                                          Pointer;
  typedef SmartPointer< const Self >                                    ConstPointer;

  typedef typename Superclass::InputImageType                           InputImageType;
  typedef typename Superclass::OutputImageType                          OutputImageType;
  typedef typename OutputImageType::RegionType                          OutputImageRegionType;

  typedef typename Superclass::TransformDirectionType                   TransformDirectionType;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Run-time type information (and related methods). */
  itkTypeMacro( VnlFFT1DComplexToComplexImageFilter, FFT1DComplexToComplexImageFilter );

protected:
  VnlFFT1DComplexToComplexImageFilter() {}
  virtual ~VnlFFT1DComplexToComplexImageFilter() {}

  virtual void ThreadedGenerateData( const OutputImageRegionType&, ThreadIdType threadID ) ITK_OVERRIDE;

private:
  VnlFFT1DComplexToComplexImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkVnlFFT1DComplexToComplexImageFilter.hxx"
#endif

#endif
