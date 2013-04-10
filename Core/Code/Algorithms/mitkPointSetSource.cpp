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


#include "mitkPointSetSource.h"


mitk::PointSetSource::PointSetSource()
{
  // Create the output.
  OutputType::Pointer output = static_cast<OutputType*>(this->MakeOutput(0).GetPointer());
  Superclass::SetNumberOfRequiredInputs(0);
  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
}

mitk::PointSetSource::~PointSetSource()
{
}

itk::DataObject::Pointer mitk::PointSetSource::MakeOutput ( DataObjectPointerArraySizeType /*idx*/ )
{
    return OutputType::New().GetPointer();
}

itk::DataObject::Pointer mitk::PointSetSource::MakeOutput( const DataObjectIdentifierType & name )
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
    {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
    }
  return static_cast<itk::DataObject *>(OutputType::New().GetPointer());
}

mitk::PointSetSource::OutputType* mitk::PointSetSource::GetOutput()
{
  return itkDynamicCastInDebugMode< OutputType * >( this->GetPrimaryOutput() );
}

const mitk::PointSetSource::OutputType* mitk::PointSetSource::GetOutput() const
{
  return itkDynamicCastInDebugMode< const OutputType * >( this->GetPrimaryOutput() );
}

mitk::PointSetSource::OutputType* mitk::PointSetSource::GetOutput(DataObjectPointerArraySizeType idx)
{
  return static_cast<OutputType*>(Superclass::GetOutput(idx));
}

const  mitk::PointSetSource::OutputType* mitk::PointSetSource::GetOutput(DataObjectPointerArraySizeType idx) const
{
  const OutputType *out = dynamic_cast< const OutputType * >
                      ( this->ProcessObject::GetOutput(idx) );

  if ( out == NULL && this->ProcessObject::GetOutput(idx) != NULL )
    {
    itkWarningMacro (<< "Unable to convert output number " << idx << " to type " <<  typeid( OutputType ).name () );
    }
  return out;
}

void mitk::PointSetSource::GraftOutput(OutputType *graft)
{
  this->GraftNthOutput(0, graft);
}

void mitk::PointSetSource::GraftOutput(const itk::ProcessObject::DataObjectIdentifierType& key, OutputType* graft)
{
  if ( !graft )
    {
    itkExceptionMacro(<< "Requested to graft output that is a NULL pointer");
    }

  itkExceptionMacro(<< "GraftOutput(): This method is not yet functional in MITK. Implement mitk::PointSet::Graft() before using!!" );

  // we use the process object method since all out output may not be
  // of the same type
  itk::DataObject *output = this->ProcessObject::GetOutput(key);

  // Call GraftImage to copy meta-information, regions, and the pixel container
  output->Graft(graft);
}

void mitk::PointSetSource::GraftNthOutput(DataObjectPointerArraySizeType idx, OutputType* graft)
{
  if ( idx >= this->GetNumberOfIndexedOutputs() )
    {
    itkExceptionMacro(<< "Requested to graft output " << idx
                      << " but this filter only has " << this->GetNumberOfIndexedOutputs() << " indexed Outputs.");
    }
  this->GraftOutput( this->MakeNameFromOutputIndex(idx), graft );
}
