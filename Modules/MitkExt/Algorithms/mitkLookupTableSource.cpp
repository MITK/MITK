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
    // Create the output. We use static_cast<> here because we know the default
    // output must be of type TOutputImage
    OutputType::Pointer output = static_cast<OutputType*>( this->MakeOutput( 0 ).GetPointer() );

    if ( output.GetPointer() == NULL )
    {
        itkWarningMacro(<<"Output could not be created!");
    }

    this->ProcessObject::SetNumberOfRequiredOutputs( 1 );
    this->ProcessObject::SetNthOutput( 0, output.GetPointer() );
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
  return static_cast<itk::DataObject *>(OutputType::New().GetPointer());
}

void
mitk::LookupTableSource::GenerateInputRequestedRegion()
{
    this->ProcessObject::GenerateInputRequestedRegion();
}

void
mitk::LookupTableSource::GraftOutput( OutputType* graft )
{
    OutputType * output = this->GetOutput(0);

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

mitk::LookupTableSource::OutputType* mitk::LookupTableSource::GetOutput(const itk::ProcessObject::DataObjectIdentifierType &key)
{
  return static_cast<mitk::LookupTableSource::OutputType*>(Superclass::GetOutput(key));
}

const mitk::LookupTableSource::OutputType* mitk::LookupTableSource::GetOutput(const itk::ProcessObject::DataObjectIdentifierType &key) const
{
  return static_cast<const mitk::LookupTableSource::OutputType*>(Superclass::GetOutput(key));
}

mitk::LookupTableSource::OutputType* mitk::LookupTableSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx)
{
  return static_cast<mitk::LookupTableSource::OutputType*>(Superclass::GetOutput(idx));
}

const mitk::LookupTableSource::OutputType* mitk::LookupTableSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx) const
{
  return static_cast<const mitk::LookupTableSource::OutputType*>(Superclass::GetOutput(idx));
}
