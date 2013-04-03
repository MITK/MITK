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
    OutputType::Pointer output = dynamic_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
    assert (output.IsNotNull());
//    this->SetNumberOfIndexedOutputs( 1 );
    this->SetPrimaryOutput(output.GetPointer());
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

 mitk::DataNodeSource::OutputType* mitk::DataNodeSource::GetOutput(const itk::ProcessObject::DataObjectIdentifierType &key)
{
  return static_cast<mitk::DataNodeSource::OutputType*>(Superclass::GetOutput(key));
}

const  mitk::DataNodeSource::OutputType* mitk::DataNodeSource::GetOutput(const itk::ProcessObject::DataObjectIdentifierType &key) const
{
  return static_cast<const mitk::DataNodeSource::OutputType*>(Superclass::GetOutput(key));
}

 mitk::DataNodeSource::OutputType* mitk::DataNodeSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx)
{
  return static_cast<mitk::DataNodeSource::OutputType*>(Superclass::GetOutput(idx));
}

const   mitk::DataNodeSource::OutputType* mitk::DataNodeSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx) const
{
  return static_cast<const mitk::DataNodeSource::OutputType*>(Superclass::GetOutput(idx));
}
