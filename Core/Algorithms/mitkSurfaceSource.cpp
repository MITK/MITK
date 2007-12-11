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


#include "mitkSurfaceSource.h"
#include "mitkSurface.h"

//##ModelId=3EF4A4A3001E
mitk::SurfaceSource::SurfaceSource()
{
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type TOutputImage
  mitk::Surface::Pointer output
    = static_cast<mitk::Surface*>(this->MakeOutput(0).GetPointer());

  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
}

//##ModelId=3EF4A4A3003C
mitk::SurfaceSource::~SurfaceSource()
{
}

//##ModelId=3EF56B0703A4
mitk::SurfaceSource::DataObjectPointer mitk::SurfaceSource::MakeOutput(unsigned int /*idx*/)
{
  return static_cast<itk::DataObject*>(mitk::Surface::New().GetPointer());
}



//##ModelId=3EF56B16028D
mitk::Surface* mitk::SurfaceSource::GetOutput()
{
  if (this->GetNumberOfOutputs() < 1)
  {
    return 0;
  }

  return static_cast<mitk::Surface*>
    (this->BaseProcess::GetOutput(0));
}

//##ModelId=3EF56B1A0257
mitk::Surface* mitk::SurfaceSource::GetOutput(unsigned int idx)
{
  return static_cast<mitk::Surface*>
    (this->ProcessObject::GetOutput(idx));
}

//##ModelId=3EF56B1101F0
void mitk::SurfaceSource::SetOutput(mitk::Surface* output)
{
  itkWarningMacro(<< "SetOutput(): This method is slated to be removed from ITK.  Please use GraftOutput() in possible combination with DisconnectPipeline() instead." );
  BaseProcess::SetNthOutput(0, output);
}

//##ModelId=3EF56B1303D3
void mitk::SurfaceSource::GraftOutput(mitk::Surface* graft)
{
  this->GraftNthOutput(0, graft);
}

//##ModelId=3EF56C440197
void mitk::SurfaceSource::GraftNthOutput(unsigned int idx, mitk::Surface *graft)
{
  if (idx < this->GetNumberOfOutputs())
  {
    mitk::Surface * output = this->GetOutput(idx);

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

