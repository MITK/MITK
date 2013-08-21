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


#include "mitkLookupTableSource.h"

mitk::LookupTableSource::LookupTableSource()
{
  // Create the output.
  itk::DataObject::Pointer output = this->MakeOutput(0);
  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output);
}


mitk::LookupTableSource::~LookupTableSource()
{}

itk::DataObject::Pointer mitk::LookupTableSource::MakeOutput ( DataObjectPointerArraySizeType /*idx*/ )
{
  return OutputType::New().GetPointer();
}


itk::DataObject::Pointer mitk::LookupTableSource::MakeOutput( const DataObjectIdentifierType & name )
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
  {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
  }
  return OutputType::New().GetPointer();
}

void
mitk::LookupTableSource::GenerateInputRequestedRegion()
{
    this->ProcessObject::GenerateInputRequestedRegion();
}

void
mitk::LookupTableSource::GraftOutput( OutputType* graft )
{
    OutputType * output = this->GetOutput();

    if ( output && graft )
    {
        // grab a handle to the bulk data of the specified data object
        // output->SetPixelContainer( graft->GetPixelContainer() );

        // copy the region ivars of the specified data object
        // output->SetRequestedRegion( graft->GetRequestedRegion() );
        // output->SetLargestPossibleRegion( graft->GetLargestPossibleRegion() );
        // output->SetBufferedRegion( graft->GetBufferedRegion() );

        // copy the meta-information
        output->CopyInformation( graft );
    }
}

mitk::LookupTableSource::OutputType* mitk::LookupTableSource::GetOutput()
{
  return itkDynamicCastInDebugMode<OutputType*>( this->GetPrimaryOutput() );
}

const mitk::LookupTableSource::OutputType* mitk::LookupTableSource::GetOutput() const
{
  return itkDynamicCastInDebugMode<const OutputType*>( this->GetPrimaryOutput() );
}

mitk::LookupTableSource::OutputType* mitk::LookupTableSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx)
{
  OutputType* out = dynamic_cast<OutputType*>( this->ProcessObject::GetOutput(idx) );
  if ( out == NULL && this->ProcessObject::GetOutput(idx) != NULL )
  {
    itkWarningMacro (<< "Unable to convert output number " << idx << " to type " <<  typeid( OutputType ).name () );
  }
  return out;
}

const mitk::LookupTableSource::OutputType* mitk::LookupTableSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx) const
{
  const OutputType* out = dynamic_cast<const OutputType*>( this->ProcessObject::GetOutput(idx) );
  if ( out == NULL && this->ProcessObject::GetOutput(idx) != NULL )
  {
    itkWarningMacro (<< "Unable to convert output number " << idx << " to type " <<  typeid( OutputType ).name () );
  }
  return out;
}
