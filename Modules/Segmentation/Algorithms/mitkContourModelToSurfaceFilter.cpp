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
#include <mitkContourModelToSurfaceFilter.h>

mitk::ContourModelToSurfaceFilter::ContourModelToSurfaceFilter()
{

}
mitk::ContourModelToSurfaceFilter::~ContourModelToSurfaceFilter()
{

}

void mitk::ContourModelToSurfaceFilter::SetInput ( const mitk::ContourModelToSurfaceFilter::InputType* input )
{
  this->SetInput( 0, input );
}

void mitk::ContourModelToSurfaceFilter::SetInput ( unsigned int idx, const mitk::ContourModelToSurfaceFilter::InputType* input )
{
  if ( idx + 1 > this->GetNumberOfInputs() )
  {
    this->SetNumberOfRequiredInputs(idx + 1);
  }
  if ( input != static_cast<InputType*> ( this->ProcessObject::GetInput ( idx ) ) )
  {
    this->ProcessObject::SetNthInput ( idx, const_cast<InputType*> ( input ) );
    this->Modified();
  }
}




void mitk::ContourModelToSurfaceFilter::GenerateData()
{

}
