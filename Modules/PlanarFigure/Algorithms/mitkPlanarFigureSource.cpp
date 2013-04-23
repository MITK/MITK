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


#include "mitkPlanarFigureSource.h"

#include "mitkPlanarCircle.h"

mitk::PlanarFigureSource::PlanarFigureSource()
{
  // Create the output.
  itk::DataObject::Pointer output = this->MakeOutput(0);
  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output);
}


mitk::PlanarFigureSource::~PlanarFigureSource()
{}

void mitk::PlanarFigureSource::GenerateInputRequestedRegion()
{
    this->ProcessObject::GenerateInputRequestedRegion();
}

itk::DataObject::Pointer mitk::PlanarFigureSource::MakeOutput ( DataObjectPointerArraySizeType /*idx*/ )
{
    return static_cast<itk::DataObject *>(PlanarCircle::New().GetPointer());
}


itk::DataObject::Pointer mitk::PlanarFigureSource::MakeOutput( const DataObjectIdentifierType & name )
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
    {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
    }
  return static_cast<itk::DataObject *>(PlanarCircle::New().GetPointer());
}

mitkBaseDataSourceGetOutputDefinitions(mitk::PlanarFigureSource)
