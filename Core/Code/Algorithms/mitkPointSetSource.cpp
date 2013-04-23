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
  itk::DataObject::Pointer output = this->MakeOutput(0);
  Superclass::SetNumberOfRequiredInputs(0);
  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output);
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
  return OutputType::New().GetPointer();
}

mitkBaseDataSourceGetOutputDefinitions(mitk::PointSetSource)
