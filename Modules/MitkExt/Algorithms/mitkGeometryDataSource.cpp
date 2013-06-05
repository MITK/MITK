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


#include "mitkGeometryDataSource.h"
#include "mitkGeometryData.h"

mitk::GeometryDataSource::GeometryDataSource()
{
  // Create the output.
  itk::DataObject::Pointer output = this->MakeOutput(0);
  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output);
}

mitk::GeometryDataSource::~GeometryDataSource()
{
}

itk::DataObject::Pointer mitk::GeometryDataSource::MakeOutput ( DataObjectPointerArraySizeType /*idx*/ )
{
  return OutputType::New().GetPointer();
}

itk::DataObject::Pointer mitk::GeometryDataSource::MakeOutput( const DataObjectIdentifierType & name )
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
  {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
  }
  return OutputType::New().GetPointer();
}

mitkBaseDataSourceGetOutputDefinitions(mitk::GeometryDataSource)
