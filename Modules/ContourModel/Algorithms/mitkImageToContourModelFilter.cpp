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

#include "mitkImageToContourModelFilter.h"



mitk::ImageToContourModelFilter::ImageToContourModelFilter()
{
}



mitk::ImageToContourModelFilter::~ImageToContourModelFilter()
{

}



void mitk::ImageToContourModelFilter::SetInput ( const mitk::ImageToContourModelFilter::InputType* input )
{
  this->SetInput( 0, input );
}



void mitk::ImageToContourModelFilter::SetInput ( unsigned int idx, const mitk::ImageToContourModelFilter::InputType* input )
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



const mitk::ImageToContourModelFilter::InputType* mitk::ImageToContourModelFilter::GetInput( void )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ImageToContourModelFilter::InputType*>(this->ProcessObject::GetInput(0));
}



const mitk::ImageToContourModelFilter::InputType* mitk::ImageToContourModelFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ImageToContourModelFilter::InputType*>(this->ProcessObject::GetInput(idx));
}


void mitk::ImageToContourModelFilter::GenerateData()
{

}
