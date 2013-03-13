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
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type TOutputImage
  mitk::GeometryData::Pointer output
    = static_cast<mitk::GeometryData*>(this->MakeOutput(0).GetPointer());
  output->Initialize();

  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
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
  return static_cast<itk::DataObject *>(OutputType::New().GetPointer());
}

void mitk::GeometryDataSource::GraftOutput(mitk::GeometryData* graft)
{
  this->GraftNthOutput(0, graft);
}

void mitk::GeometryDataSource::GraftNthOutput(unsigned int idx, mitk::GeometryData *graft)
{
  if (idx < this->GetNumberOfOutputs())
  {
    mitk::GeometryData * output = this->GetOutput(idx);

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
