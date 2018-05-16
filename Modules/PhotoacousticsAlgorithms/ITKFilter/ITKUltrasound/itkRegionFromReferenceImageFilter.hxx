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
#ifndef itkRegionFromReferenceImageFilter_hxx
#define itkRegionFromReferenceImageFilter_hxx

#include "itkRegionFromReferenceImageFilter.h"

namespace itk
{

template <typename TInputImage, typename TOutputImage>
void
RegionFromReferenceImageFilter<TInputImage, TOutputImage>
::GenerateOutputInformation()
{
  if( !this->GetInput() || !this->GetReferenceImage() )
    {
    return;
    }

  // Superclass::Superclass::GenerateOutputInformation();
  this->SetExtractionRegion( this->GetReferenceImage()->GetLargestPossibleRegion() );
  Superclass::GenerateOutputInformation();
}


template <typename TInputImage, typename TOutputImage>
void
RegionFromReferenceImageFilter<TInputImage,TOutputImage>
::SetReferenceImage ( const ReferenceImageType *image )
{
  itkDebugMacro("setting input ReferenceImage to " << image);
  if( image != static_cast<const ReferenceImageType *>(this->GetInput( 1 )) )
    {
    this->ProcessObject::SetNthInput(1, const_cast< ReferenceImageType *>( image ) );
    this->Modified();
    }
}


template <typename TInputImage, typename TOutputImage>
const typename RegionFromReferenceImageFilter<TInputImage,TOutputImage>::ReferenceImageType *
RegionFromReferenceImageFilter<TInputImage,TOutputImage>
::GetReferenceImage() const
{
  Self * surrogate = const_cast< Self * >( this );

  const DataObject * input = surrogate->ProcessObject::GetInput(1);

  const ReferenceImageType * referenceImage = static_cast<const ReferenceImageType *>( input );

  return referenceImage;
}

} // end namespace itk

#endif
