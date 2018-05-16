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
#ifndef itkTimeGainCompensationImageFilter_h
#define itkTimeGainCompensationImageFilter_h

#include "itkImageToImageFilter.h"

#include "itkArray2D.h"

namespace itk
{

/**
 * \class TimeGainCompensationImageFilter
 * \brief Applies a linear piecewise time gain compensation.
 *
 * This filter applies a linear piecewise gain with depth.  The depth
 * direction is assumed to be the first direction (0th direction).
 *
 * \ingroup Ultrasound
 * */
template< typename TInputImage, typename TOutputImage = TInputImage >
class TimeGainCompensationImageFilter:
  public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef TInputImage                                           InputImageType;
  typedef TOutputImage                                          OutputImageType;

  typedef TimeGainCompensationImageFilter                       Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType > Superclass;

  typedef SmartPointer< Self >                                  Pointer;
  typedef SmartPointer< const Self >                            ConstPointer;

  itkTypeMacro( TimeGainCompensationImageFilter, ImageToImageFilter );
  itkNewMacro( Self );

  typedef Array2D< double > GainType;

  /** Set/Get the gain.  The first column specifies the depth. The second
   * column specifies the gain. */
  itkSetMacro( Gain, GainType );
  itkGetConstReferenceMacro( Gain, GainType );

protected:
  typedef typename OutputImageType::RegionType                  OutputImageRegionType;

  TimeGainCompensationImageFilter();
  virtual ~TimeGainCompensationImageFilter() {}

  void PrintSelf( std::ostream & os, Indent indent ) const ITK_OVERRIDE;

  virtual void BeforeThreadedGenerateData() ITK_OVERRIDE;
  virtual void ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, ThreadIdType threadId ) ITK_OVERRIDE;

private:
  TimeGainCompensationImageFilter( const Self& ); // purposely not implemented
  void operator=( const Self& ); // purposely not implemented

  GainType m_Gain;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTimeGainCompensationImageFilter.hxx"
#endif

#endif // itkTimeGainCompensationImageFilter_h
