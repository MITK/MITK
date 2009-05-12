/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkPointSetToGeometryDataFilter.h"
#include <itkProcessObject.h>

void mitk::PointSetToGeometryDataFilter::SetInput ( const mitk::PointSetToGeometryDataFilter::InputType* input )
{
  this->SetInput( 0, input );
}


void mitk::PointSetToGeometryDataFilter::SetInput ( const unsigned int& idx, const mitk::PointSetToGeometryDataFilter::InputType* input )
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


mitk::PointSetToGeometryDataFilter::InputType* mitk::PointSetToGeometryDataFilter::GetInput()
{
  return this->GetInput( 0 );
}


mitk::PointSetToGeometryDataFilter::InputType* mitk::PointSetToGeometryDataFilter::GetInput ( const unsigned int& idx )
{
  if ( this->GetNumberOfInputs() < 1 )
    return 0;
  
  return static_cast<InputType*> ( this->ProcessObject::GetInput ( idx ) );
}


mitk::PointSetToGeometryDataFilter::PointSetToGeometryDataFilter()
{}
  
mitk::PointSetToGeometryDataFilter::~PointSetToGeometryDataFilter()
{}

