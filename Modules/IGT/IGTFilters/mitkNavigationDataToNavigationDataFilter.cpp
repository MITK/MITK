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

#include "mitkNavigationDataToNavigationDataFilter.h"


mitk::NavigationDataToNavigationDataFilter::NavigationDataToNavigationDataFilter() 
: mitk::NavigationDataSource()
{
}


mitk::NavigationDataToNavigationDataFilter::~NavigationDataToNavigationDataFilter()
{
}


void mitk::NavigationDataToNavigationDataFilter::SetInput( const NavigationData* nd )
{
  this->SetInput(0, nd);
}


void mitk::NavigationDataToNavigationDataFilter::SetInput( unsigned int idx, const NavigationData* nd )
{
  if ((nd == NULL) && (idx == this->GetNumberOfInputs() - 1)) // if the last input is set to NULL, reduce the number of inputs by one
    this->SetNumberOfInputs(this->GetNumberOfInputs() - 1);
  else
    this->ProcessObject::SetNthInput(idx, const_cast<NavigationData*>(nd));   // Process object is not const-correct so the const_cast is required here

  this->CreateOutputsForAllInputs();
}


const mitk::NavigationData* mitk::NavigationDataToNavigationDataFilter::GetInput( void )
{
  return this->GetInput(0);
}


const mitk::NavigationData* mitk::NavigationDataToNavigationDataFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;

  return static_cast<const NavigationData*>(this->ProcessObject::GetInput(idx));
}


void mitk::NavigationDataToNavigationDataFilter::CreateOutputsForAllInputs()
{
  this->SetNumberOfOutputs(this->GetNumberOfInputs());  // create outputs for all inputs
  for (unsigned int idx = 0; idx < this->GetNumberOfOutputs(); ++idx)
    if (this->GetOutput(idx) == NULL)
    {
      DataObjectPointer newOutput = this->MakeOutput(idx);
      this->SetNthOutput(idx, newOutput);
    }
    this->Modified();
}
