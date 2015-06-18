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

#include "mitkDataNodeSource.h"

mitk::DataNodeSource::DataNodeSource()
{
    // Create the output.
    OutputType::Pointer output = static_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
    this->SetNthOutput(0, output.GetPointer());
}

mitk::DataNodeSource::~DataNodeSource()
{
}

itk::ProcessObject::DataObjectPointer mitk::DataNodeSource::MakeOutput ( DataObjectPointerArraySizeType /*idx*/ )
{
    return OutputType::New().GetPointer();
}


itk::ProcessObject::DataObjectPointer mitk::DataNodeSource::MakeOutput( const DataObjectIdentifierType & name )
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
    {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
    }
  return static_cast<itk::DataObject*>(OutputType::New().GetPointer());
}

mitk::DataNodeSource::OutputType* mitk::DataNodeSource::GetOutput()
{
  return itkDynamicCastInDebugMode< OutputType * >( this->GetPrimaryOutput() );
}

const mitk::DataNodeSource::OutputType* mitk::DataNodeSource::GetOutput() const
{
  return itkDynamicCastInDebugMode< const OutputType * >( this->GetPrimaryOutput() );
}

mitk::DataNodeSource::OutputType* mitk::DataNodeSource::GetOutput(DataObjectPointerArraySizeType idx)
{
  return static_cast<OutputType*>(Superclass::GetOutput(idx));
}

const  mitk::DataNodeSource::OutputType* mitk::DataNodeSource::GetOutput(DataObjectPointerArraySizeType idx) const
{
  const OutputType *out = dynamic_cast< const OutputType * >
                      ( this->ProcessObject::GetOutput(idx) );

  if ( out == nullptr && this->ProcessObject::GetOutput(idx) != nullptr )
    {
    itkWarningMacro (<< "Unable to convert output number " << idx << " to type " <<  typeid( OutputType ).name () );
    }
  return out;
}
