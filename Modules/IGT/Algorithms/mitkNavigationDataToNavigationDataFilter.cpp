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

#include "mitkNavigationDataToNavigationDataFilter.h"


mitk::NavigationDataToNavigationDataFilter::NavigationDataToNavigationDataFilter()
: mitk::NavigationDataSource()
{
mitk::NavigationData::Pointer output = mitk::NavigationData::New();
this->SetNumberOfRequiredOutputs(1);
this->SetNthOutput(0, output.GetPointer());
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
  if ( nd == NULL ) // if an input is set to NULL, remove it
    this->RemoveInput(idx);
  else
    this->ProcessObject::SetNthInput(idx, const_cast<NavigationData*>(nd));   // ProcessObject is not const-correct so a const_cast is required here
  this->CreateOutputsForAllInputs();
}


const mitk::NavigationData* mitk::NavigationDataToNavigationDataFilter::GetInput( void ) const
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;

  return static_cast<const NavigationData*>(this->ProcessObject::GetInput(0));
}


const mitk::NavigationData* mitk::NavigationDataToNavigationDataFilter::GetInput( unsigned int idx ) const
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;

  return static_cast<const NavigationData*>(this->ProcessObject::GetInput(idx));
}


const mitk::NavigationData* mitk::NavigationDataToNavigationDataFilter::GetInput(std::string navDataName) const
{
  const DataObjectPointerArray& inputs = const_cast<Self*>(this)->GetInputs();
  for (DataObjectPointerArray::const_iterator it = inputs.begin(); it != inputs.end(); ++it)
    if (std::string(navDataName) == (static_cast<NavigationData*>(it->GetPointer()))->GetName())
      return static_cast<NavigationData*>(it->GetPointer());
  return NULL;
}


itk::ProcessObject::DataObjectPointerArraySizeType mitk::NavigationDataToNavigationDataFilter::GetInputIndex( std::string navDataName )
{
  DataObjectPointerArray outputs = this->GetInputs();
  for (DataObjectPointerArray::size_type i = 0; i < outputs.size(); ++i)
    if (navDataName == (static_cast<NavigationData*>(outputs.at(i).GetPointer()))->GetName())
      return i;
  throw std::invalid_argument("output name does not exist");
}

void mitk::NavigationDataToNavigationDataFilter::ConnectTo(mitk::NavigationDataSource* UpstreamFilter)
{
  for (DataObjectPointerArraySizeType i = 0; i < UpstreamFilter->GetNumberOfOutputs(); i++)
  {
    this->SetInput(i, UpstreamFilter->GetOutput(i));
  }
}


void mitk::NavigationDataToNavigationDataFilter::CreateOutputsForAllInputs()
{
  this->SetNumberOfIndexedOutputs(this->GetNumberOfIndexedInputs());  // create outputs for all inputs
  bool isModified = false;
  for (unsigned int idx = 0; idx < this->GetNumberOfIndexedOutputs(); ++idx){
    if (this->GetOutput(idx) == NULL)
    {
      mitk::NavigationData::Pointer newOutput = mitk::NavigationData::New();
      this->SetNthOutput(idx, newOutput);
      isModified = true;
    }
  }

  if(isModified)
    this->Modified();
}
