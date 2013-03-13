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


#include "mitkSurfaceSource.h"
#include "mitkSurface.h"

mitk::SurfaceSource::SurfaceSource()
{
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type TOutputImage
  mitk::Surface::Pointer output
    = static_cast<mitk::Surface*>(this->MakeOutput(0).GetPointer());

  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
}

mitk::SurfaceSource::~SurfaceSource()
{
}

itk::DataObject::Pointer mitk::SurfaceSource::MakeOutput ( DataObjectPointerArraySizeType /*idx*/ )
{
  return static_cast<itk::DataObject *>(itk::DataObject::New().GetPointer());
}


itk::DataObject::Pointer mitk::SurfaceSource::MakeOutput( const DataObjectIdentifierType & name )
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
    {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
    }
  return static_cast<itk::DataObject *>(itk::DataObject::New().GetPointer());
}


void mitk::SurfaceSource::GraftOutput(mitk::Surface* graft)
{
  this->GraftNthOutput(0, graft);
}

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



mitk::Surface *mitk::SurfaceSource::GetOutput(const itk::ProcessObject::DataObjectIdentifierType &key)
{
  return static_cast<mitk::Surface*>(Superclass::GetOutput(key));
}

const mitk::Surface *mitk::SurfaceSource::GetOutput(const itk::ProcessObject::DataObjectIdentifierType &key) const
{
  return static_cast<const mitk::Surface*>(Superclass::GetOutput(key));
}

mitk::Surface *mitk::SurfaceSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx)
{
  return static_cast<mitk::Surface*>(Superclass::GetOutput(idx));
}

const mitk::Surface *mitk::SurfaceSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx) const
{
  return static_cast<const mitk::Surface*>(Superclass::GetOutput(idx));
}
