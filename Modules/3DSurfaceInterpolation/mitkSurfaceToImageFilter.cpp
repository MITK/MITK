/*=========================================================================
Program: Medical Imaging & Interaction Toolkit
Module: $RCSfile$
Language: C++
Date: $Date: $
Version: $Revision: $
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notices for more information.
=========================================================================*/

#include "mitkSurfaceToImageFilter.h"

mitk::SurfaceToImageFilter::SurfaceToImageFilter()
{
}

mitk::SurfaceToImageFilter::~SurfaceToImageFilter()
{
}

void mitk::SurfaceToImageFilter::SetInput( const mitk::Surface* surface )
{
  this->SetInput( 0, const_cast<mitk::Surface*>( surface ) );
}


void mitk::SurfaceToImageFilter::SetInput( unsigned int idx, const mitk::Surface* surface )
{
  if ( this->GetInput(idx) != surface )
  {
    this->SetNthInput( idx, const_cast<mitk::Surface*>( surface ) );
    this->Modified();
  }
}


const mitk::Surface* mitk::SurfaceToImageFilter::GetInput()
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;

  return static_cast<const mitk::Surface*>(this->ProcessObject::GetInput(0));
}


const mitk::Surface* mitk::SurfaceToImageFilter::GetInput( unsigned int idx)
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;

  return static_cast<const mitk::Surface*>(this->ProcessObject::GetInput(idx));
}

void mitk::SurfaceToImageFilter::RemoveInputs(mitk::Surface* input)
{
  this->RemoveInput(input);
}
