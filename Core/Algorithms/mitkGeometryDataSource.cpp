/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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

mitk::GeometryDataSource::DataObjectPointer mitk::GeometryDataSource::MakeOutput(unsigned int /*idx*/)
{
  return static_cast<itk::DataObject*>(mitk::GeometryData::New().GetPointer());
}

mitk::GeometryData* mitk::GeometryDataSource::GetOutput()
{
  if (this->GetNumberOfOutputs() < 1)
  {
    return 0;
  }

  return static_cast<mitk::GeometryData*>
    (this->BaseProcess::GetOutput(0));
}

mitk::GeometryData* mitk::GeometryDataSource::GetOutput(unsigned int idx)
{
  return static_cast<mitk::GeometryData*>
    (this->itk::ProcessObject::GetOutput(idx));
}

void mitk::GeometryDataSource::SetOutput(mitk::GeometryData* output)
{
  itkWarningMacro(<< "SetOutput(): This method is slated to be removed from ITK.  Please use GraftOutput() in possible combination with DisconnectPipeline() instead." );
  BaseProcess::SetNthOutput(0, output);
}

void mitk::GeometryDataSource::GraftOutput(mitk::GeometryData* graft)
{
  this->GraftNthOutput(0, graft);
}

//##ModelId=3EF56C440197
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
