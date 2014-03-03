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


#include "mitkMeshSource.h"


mitk::MeshSource::MeshSource()
{
    // Create the output.
    OutputType::Pointer output = dynamic_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
    assert (output.IsNotNull());
    this->SetNumberOfRequiredInputs(0);
    this->SetNumberOfIndexedOutputs( 1 );
    this->SetOutput(0, output.GetPointer());
}

mitk::MeshSource::~MeshSource()
{
}

itk::DataObject::Pointer mitk::MeshSource::MakeOutput ( DataObjectPointerArraySizeType /*idx*/ )
{
    return OutputType::New().GetPointer();
}


itk::DataObject::Pointer mitk::MeshSource::MakeOutput( const DataObjectIdentifierType & name )
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
    {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
    }
  return static_cast<itk::DataObject *>(OutputType::New().GetPointer());
}

mitk::MeshSource::OutputType* mitk::MeshSource::GetOutput(const itk::ProcessObject::DataObjectIdentifierType &key)
{
  return static_cast<mitk::MeshSource::OutputType*>(Superclass::GetOutput(key));
}

const mitk::MeshSource::OutputType* mitk::MeshSource::GetOutput(const itk::ProcessObject::DataObjectIdentifierType &key) const
{
  return static_cast<const mitk::MeshSource::OutputType*>(Superclass::GetOutput(key));
}

mitk::MeshSource::OutputType* mitk::MeshSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx)
{
  return static_cast<mitk::MeshSource::OutputType*>(Superclass::GetOutput(idx));
}

const mitk::MeshSource::OutputType* mitk::MeshSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx) const
{
  return static_cast<const mitk::MeshSource::OutputType*>(Superclass::GetOutput(idx));
}
