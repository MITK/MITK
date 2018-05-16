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
#ifndef itkSpectra1DSupportWindowImageFilter_h
#define itkSpectra1DSupportWindowImageFilter_h

#include <deque>

#include "itkImageToImageFilter.h"

namespace itk
{

/** \class Spectra1DSupportWindowImageFilter
 * \brief Generate an image of local spectra computation support windows.
 *
 * The information from the input image is used to determine the output image
 * information. The pixel value of the input image is used to specify the
 * nominal number of lines on either side of the central FFT line to add to
 * the window. The nominal size of the 1D FFT is specified with SetFFTSize()
 *
 * The overlap between windows is specified with SetStep(). By default, the
 * Step is only one sample.
 *
 * \ingroup Ultrasound
 */
template< typename TInputImage >
class Spectra1DSupportWindowImageFilter:
  public ImageToImageFilter< TInputImage,
                             Image< std::deque< typename TInputImage::IndexType >, TInputImage::ImageDimension > >
{
public:
  itkStaticConstMacro( ImageDimension, unsigned int, TInputImage::ImageDimension );

  typedef TInputImage                              InputImageType;
  typedef typename InputImageType::IndexType       IndexType;

  typedef std::deque< IndexType >                  OutputPixelType;
  typedef Image< OutputPixelType, ImageDimension > OutputImageType;

  typedef unsigned int                             FFT1DSizeType;

  /** Standard class typedefs. */
  typedef Spectra1DSupportWindowImageFilter                     Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType > Superclass;
  typedef SmartPointer< Self >                                  Pointer;
  typedef SmartPointer< const Self >                            ConstPointer;

  itkTypeMacro( Spectra1DSupportWindowImageFilter, ImageToImageFilter );
  itkNewMacro( Self );

  /** Set/Get the nominal size of the FFT.  This will be truncated at the
   * boundary of image. */
  itkGetConstMacro( FFT1DSize, FFT1DSizeType );
  itkSetMacro( FFT1DSize, FFT1DSizeType );

  /** Set/Get the number of samples between windows -- defaults to 1. */
  itkGetConstMacro( Step, SizeValueType );
  itkSetMacro( Step, SizeValueType );

protected:
  Spectra1DSupportWindowImageFilter();
  virtual ~Spectra1DSupportWindowImageFilter() {};

  typedef typename OutputImageType::RegionType OutputImageRegionType;

  virtual void GenerateOutputInformation() ITK_OVERRIDE;

  virtual void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId ) ITK_OVERRIDE;
  virtual void AfterThreadedGenerateData() ITK_OVERRIDE;

  virtual void PrintSelf( std::ostream & os, Indent indent ) const ITK_OVERRIDE;

private:
  Spectra1DSupportWindowImageFilter( const Self & ); // purposely not implemented
  void operator=( const Self & ); // purposely not implemented

  FFT1DSizeType m_FFT1DSize;
  SizeValueType m_Step;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkSpectra1DSupportWindowImageFilter.hxx"
#endif

#endif // itkSpectra1DSupportWindowImageFilter_h
