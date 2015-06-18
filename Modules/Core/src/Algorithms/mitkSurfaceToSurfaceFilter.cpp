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


#include "mitkSurfaceToSurfaceFilter.h"
#include "mitkSurface.h"

mitk::SurfaceToSurfaceFilter::SurfaceToSurfaceFilter()
: SurfaceSource()
{
}

mitk::SurfaceToSurfaceFilter::~SurfaceToSurfaceFilter()
{
}


void mitk::SurfaceToSurfaceFilter::SetInput( const mitk::Surface* surface )
{
  this->SetInput( 0, const_cast<mitk::Surface*>( surface ) );
}


void mitk::SurfaceToSurfaceFilter::SetInput( unsigned int idx, const mitk::Surface* surface )
{
  if ( this->GetInput(idx) != surface )
  {
    this->SetNthInput( idx, const_cast<mitk::Surface*>( surface ) );
    this->CreateOutputForInput(idx);
    this->Modified();
  }
}


const mitk::Surface* mitk::SurfaceToSurfaceFilter::GetInput()
{
  if (this->GetNumberOfInputs() < 1)
    return nullptr;

  return static_cast<const mitk::Surface*>(this->ProcessObject::GetInput(0));
}


const mitk::Surface* mitk::SurfaceToSurfaceFilter::GetInput( unsigned int idx)
{
  if (this->GetNumberOfInputs() < 1)
    return nullptr;

  return static_cast<const mitk::Surface*>(this->ProcessObject::GetInput(idx));
}

void mitk::SurfaceToSurfaceFilter::CreateOutputForInput(unsigned int idx)
{
  if (this->GetNumberOfIndexedInputs() < idx || this->GetInput(idx) == nullptr)
  {
    mitkThrow() << "Error creating output for input [" <<idx<< "]. Input does not exists!";
  }

  if (this->GetNumberOfIndexedOutputs() <= idx)
    this->SetNumberOfIndexedOutputs( idx+1 );

  if (this->GetOutput(idx) == nullptr)
  {
    DataObjectPointer newOutput = this->MakeOutput(idx);
    this->SetNthOutput(idx, newOutput);
  }
  this->GetOutput( idx )->Graft( this->GetInput( idx) );
  this->Modified();
}

void mitk::SurfaceToSurfaceFilter::CreateOutputsForAllInputs()
{
  this->SetNumberOfIndexedOutputs( this->GetNumberOfIndexedInputs() );
  for (unsigned int idx = 0; idx < this->GetNumberOfIndexedInputs(); ++idx)
  {
    if (this->GetOutput(idx) == nullptr)
    {
      DataObjectPointer newOutput = this->MakeOutput(idx);
      this->SetNthOutput(idx, newOutput);
    }
    this->GetOutput( idx )->Graft( this->GetInput( idx) );
  }
  this->Modified();
}

void mitk::SurfaceToSurfaceFilter::RemoveInputs( mitk::Surface* surface )
{
  for (unsigned int idx = 0; idx < this->GetNumberOfIndexedInputs(); ++idx)
  {
    if ( this->GetInput(idx) == surface )
    {
      this->RemoveOutput(idx);
    }
  }
}
