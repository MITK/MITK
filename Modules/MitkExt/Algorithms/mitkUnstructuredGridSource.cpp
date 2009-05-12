/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 10345 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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

mitk::UnstructuredGridSource::DataObjectPointer mitk::UnstructuredGridSource::MakeOutput(unsigned int /*idx*/)
{
  return static_cast<itk::DataObject*>(mitk::UnstructuredGrid::New().GetPointer());
}


mitk::UnstructuredGrid* mitk::UnstructuredGridSource::GetOutput()
{
  if (this->GetNumberOfOutputs() < 1)
  {
    return 0;
  }

  return static_cast<mitk::UnstructuredGrid*>
    (this->BaseProcess::GetOutput(0));
}

mitk::UnstructuredGrid* mitk::UnstructuredGridSource::GetOutput(unsigned int idx)
{
  return static_cast<mitk::UnstructuredGrid*>
    (this->ProcessObject::GetOutput(idx));
}

void mitk::UnstructuredGridSource::SetOutput(mitk::UnstructuredGrid* output)
{
  itkWarningMacro(<< "SetOutput(): This method is slated to be removed from ITK.  Please use GraftOutput() in possible combination with DisconnectPipeline() instead." );
  BaseProcess::SetNthOutput(0, output);
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

