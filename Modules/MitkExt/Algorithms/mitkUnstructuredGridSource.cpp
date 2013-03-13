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


#include "mitkUnstructuredGridSource.h"
#include "mitkUnstructuredGrid.h"

mitk::UnstructuredGridSource::UnstructuredGridSource()
{
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type UnstructuredGrid
  mitk::UnstructuredGrid::Pointer output
    = static_cast<mitk::UnstructuredGrid*>(this->MakeOutput(0).GetPointer());

  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
}

mitk::UnstructuredGridSource::~UnstructuredGridSource()
{
}

itk::DataObject::Pointer mitk::UnstructuredGridSource::MakeOutput ( DataObjectPointerArraySizeType /*idx*/ )
{
    return OutputType::New().GetPointer();
}


itk::DataObject::Pointer mitk::UnstructuredGridSource::MakeOutput( const DataObjectIdentifierType & name )
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
    {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
    }
  return static_cast<DataObject *>(OutputType::New().GetPointer());
}


void mitk::UnstructuredGridSource::GraftOutput(mitk::UnstructuredGrid* graft)
{
  this->GraftNthOutput(0, graft);
}

void mitk::UnstructuredGridSource::GraftNthOutput(unsigned int idx, mitk::UnstructuredGrid *graft)
{
  if (idx < this->GetNumberOfOutputs())
  {
    mitk::UnstructuredGrid * output = this->GetOutput(idx);

    if (output && graft)
    {
      // grab a handle to the bulk data of the specified data object
      //      output->SetPixelContainer( graft->GetPixelContainer() ); @FIXME!!!!

      // copy the region ivars of the specified data object
      output->SetRequestedRegion( graft );//graft->GetRequestedRegion() );
      //      output->SetLargestPossibleRegion( graft->GetLargestPossibleRegion() ); @FIXME!!!!
      //      output->SetBufferedRegion( graft->GetBufferedRegion() ); @FIXME!!!!

      // copy the meta-information
      output->CopyInformation( graft );
    }
  }
}

mitk::UnstructuredGridSource::OutputType* mitk::UnstructuredGridSource::GetOutput(const itk::ProcessObject::DataObjectIdentifierType &key)
{
  return static_cast<mitk::UnstructuredGridSource::OutputType*>(Superclass::GetOutput(key));
}

const mitk::UnstructuredGridSource::OutputType* mitk::UnstructuredGridSource::GetOutput(const itk::ProcessObject::DataObjectIdentifierType &key) const
{
  return static_cast<const mitk::UnstructuredGridSource::OutputType*>(Superclass::GetOutput(key));
}

mitk::UnstructuredGridSource::OutputType* mitk::UnstructuredGridSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx)
{
  return static_cast<mitk::UnstructuredGridSource::OutputType*>(Superclass::GetOutput(idx));
}

const mitk::UnstructuredGridSource::OutputType* mitk::UnstructuredGridSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx) const
{
  return static_cast<const mitk::UnstructuredGridSource::OutputType*>(Superclass::GetOutput(idx));
}
