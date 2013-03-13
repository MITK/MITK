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

#include "mitkContourModelSource.h"


mitk::ContourModelSource::ContourModelSource()
{
  // Create the output.
  OutputType::Pointer output = dynamic_cast<OutputType*>(this->MakeOutput(0).GetPointer());
  Superclass::SetNumberOfRequiredInputs(0);
  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
}

mitk::ContourModelSource::~ContourModelSource()
{
}

itk::DataObject::Pointer mitk::ContourModelSource::MakeOutput ( DataObjectPointerArraySizeType /*idx*/ )
{
    return OutputType::New().GetPointer();
}


itk::DataObject::Pointer mitk::ContourModelSource::MakeOutput( const DataObjectIdentifierType & name )
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
    {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
    }
  return static_cast<itk::DataObject *>(OutputType::New().GetPointer());
}

void mitk::ContourModelSource::GraftOutput(OutputType *graft)
{
  this->GraftNthOutput(0, graft);
}

void mitk::ContourModelSource::GraftNthOutput(unsigned int /*idx*/, OutputType* /*graft*/)
{
  itkWarningMacro(<< "GraftNthOutput(): This method is not yet implemented for mitk. Implement it before using!!" );
  assert(false);
}

mitk::ContourModelSource::OutputType* mitk::ContourModelSource::GetOutput(const itk::ProcessObject::DataObjectIdentifierType &key)
{
  return static_cast<mitk::ContourModelSource::OutputType*>(Superclass::GetOutput(key));
}

const mitk::ContourModelSource::OutputType* mitk::ContourModelSource::GetOutput(const itk::ProcessObject::DataObjectIdentifierType &key) const
{
  return static_cast<const mitk::ContourModelSource::OutputType*>(Superclass::GetOutput(key));
}

 mitk::ContourModelSource::OutputType* mitk::ContourModelSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx)
{
  return static_cast<mitk::ContourModelSource::OutputType*>(Superclass::GetOutput(idx));
}

const  mitk::ContourModelSource::OutputType* mitk::ContourModelSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx) const
{
  return static_cast<const mitk::ContourModelSource::OutputType*>(Superclass::GetOutput(idx));
}
