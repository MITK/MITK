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

#ifndef _mitk_DiffusionImageToDiffusionImageFilter_cpp
#define _mitk_DiffusionImageToDiffusionImageFilter_cpp

#include "mitkDiffusionImageToDiffusionImageFilter.h"

template <typename DiffusionPixelType>
mitk::DiffusionImageToDiffusionImageFilter<DiffusionPixelType>
::DiffusionImageToDiffusionImageFilter()
{
  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfRequiredOutputs(1);
}

template <typename DiffusionPixelType>
void mitk::DiffusionImageToDiffusionImageFilter<DiffusionPixelType>
::SetInput(const InputImageType *input)
{
  // call to the more general method
  this->SetInput(0, input);
}

template <typename DiffusionPixelType>
void mitk::DiffusionImageToDiffusionImageFilter<DiffusionPixelType>
::SetInput(unsigned int index, const InputImageType *input)
{
  if( index+1 > this->GetNumberOfInputs() )
  {
    this->SetNumberOfRequiredInputs( index + 1 );
  }
  // Process object is not const-correct so the const_cast is required here
  this->itk::ProcessObject::SetNthInput(index,
    const_cast< typename Self::InputImageType *>( input ) );
}


template <typename DiffusionPixelType>
const typename mitk::DiffusionImageToDiffusionImageFilter<DiffusionPixelType>::InputImageType*
mitk::DiffusionImageToDiffusionImageFilter<DiffusionPixelType>
::GetInput()
{
  // call to the more general method
  return this->GetInput(0);
}

template <typename DiffusionPixelType>
const typename mitk::DiffusionImageToDiffusionImageFilter<DiffusionPixelType>::InputImageType*
mitk::DiffusionImageToDiffusionImageFilter<DiffusionPixelType>
::GetInput(unsigned int idx)
{
  return static_cast< const typename Self::InputImageType * >
    (this->itk::ProcessObject::GetInput(idx));
}




#endif
