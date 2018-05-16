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
#ifndef itkSpectra1DSupportWindowToMaskImageFilter_h
#define itkSpectra1DSupportWindowToMaskImageFilter_h

#include "itkImageToImageFilter.h"

namespace itk
{

/** \class Spectra1DSupportWindowToMaskImageFilter
 * \brief Generate a mask image from the support window at a given index.
 *
 * \ingroup Ultrasound
 */
template< typename TInputImage, typename TOutputImage >
class Spectra1DSupportWindowToMaskImageFilter:
  public ImageToImageFilter< TInputImage,
                             TOutputImage >
{
public:
  itkStaticConstMacro( ImageDimension, unsigned int, TInputImage::ImageDimension );

  typedef TInputImage  InputImageType;
  typedef TOutputImage OutputImageType;

  typedef typename InputImageType::IndexType  IndexType;
  typedef typename OutputImageType::PixelType OutputPixelType;

  /** Standard class typedefs. */
  typedef Spectra1DSupportWindowToMaskImageFilter               Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType > Superclass;
  typedef SmartPointer< Self >                                  Pointer;
  typedef SmartPointer< const Self >                            ConstPointer;

  itkTypeMacro( Spectra1DSupportWindowToMaskImageFilter, ImageToImageFilter );
  itkNewMacro( Self );

  /** Set/Get the index of the support window to create the mask for. */
  itkGetConstReferenceMacro( MaskIndex, IndexType );
  itkSetMacro( MaskIndex, IndexType );

  /** Set/Get the value to consider as "background". Defaults to zero. */
  itkSetMacro( BackgroundValue, OutputPixelType );
  itkGetConstMacro( BackgroundValue, OutputPixelType );

  /** Set/Get the value in the image to consider as "foreground". Defaults to
   * maximum value of the OutputPixelType. */
  itkSetMacro( ForegroundValue, OutputPixelType );
  itkGetConstMacro( ForegroundValue, OutputPixelType );


protected:
  Spectra1DSupportWindowToMaskImageFilter();
  virtual ~Spectra1DSupportWindowToMaskImageFilter() {};

  virtual void GenerateData() ITK_OVERRIDE;

private:
  Spectra1DSupportWindowToMaskImageFilter( const Self & ); // purposely not implemented
  void operator=( const Self & ); // purposely not implemented

  IndexType m_MaskIndex;

  OutputPixelType m_BackgroundValue;
  OutputPixelType m_ForegroundValue;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkSpectra1DSupportWindowToMaskImageFilter.hxx"
#endif

#endif // itkSpectra1DSupportWindowToMaskImageFilter_h
