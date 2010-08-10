/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkPointSetToPointSetFilter.h"


mitk::PointSetToPointSetFilter::PointSetToPointSetFilter()
{
  // Modify superclass default values, may be overridden by subclasses
  this->SetNumberOfRequiredInputs( 1 );
}


mitk::PointSetToPointSetFilter::~PointSetToPointSetFilter()
{}


void mitk::PointSetToPointSetFilter::SetInput( const mitk::PointSetToPointSetFilter::InputType* input )
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput( 0, const_cast< mitk::PointSetToPointSetFilter::InputType * >( input ) );
}


void mitk::PointSetToPointSetFilter::SetInput( const unsigned int& idx, const mitk::PointSetToPointSetFilter::InputType* input )
{
  if ( idx + 1 > this->GetNumberOfInputs() )
  {
    this->SetNumberOfRequiredInputs( idx + 1 );
  }
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput( idx, const_cast< mitk::PointSetToPointSetFilter::InputType *>( input ) );
}


const mitk::PointSetToPointSetFilter::InputType * mitk::PointSetToPointSetFilter::GetInput( void )
{
  if ( this->GetNumberOfInputs() < 1 )
    return 0;
  // Process object is not const-correct so the const_cast is required here
  return static_cast<const mitk::PointSetToPointSetFilter::InputType * >( this->ProcessObject::GetInput( 0 ) );
}


const mitk::PointSetToPointSetFilter::InputType * mitk::PointSetToPointSetFilter::GetInput( const unsigned int& idx )
{
  if (idx > this->GetNumberOfInputs() - 1 )
    return 0;
  // Process object is not const-correct so the const_cast is required here
  return static_cast< const mitk::PointSetToPointSetFilter::InputType * >(this->ProcessObject::GetInput(idx));
}


void mitk::PointSetToPointSetFilter::operator=( const mitk::PointSetToPointSetFilter::Self& )
{}


