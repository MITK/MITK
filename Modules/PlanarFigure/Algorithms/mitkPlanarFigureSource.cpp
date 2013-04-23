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
    // Create the output. We use static_cast<> here because we know the default
    // output must be of type TOutputImage
    //OutputType::Pointer output = static_cast<OutputType*>( this->MakeOutput( 0 ).GetPointer() );

    //if ( output.GetPointer() == NULL )
    //{
    //    itkWarningMacro(<<"Output could not be created!");
    //}
    //this->ProcessObject::SetNumberOfRequiredOutputs( 1 );
    //this->ProcessObject::SetNthOutput( 0, output.GetPointer() );
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

void mitk::PlanarFigureSource::GraftOutput(itk::DataObject *graft)
{
  this->GraftNthOutput(0, graft);
}


void mitk::PlanarFigureSource::GraftNthOutput(unsigned int idx, itk::DataObject *graft)
{
  if ( idx >= this->GetNumberOfOutputs() )
  {
    itkExceptionMacro(<<"Requested to graft output " << idx <<
      " but this filter only has " << this->GetNumberOfOutputs() << " Outputs.");
  }

  if ( !graft )
  {
    itkExceptionMacro(<<"Requested to graft output with a NULL pointer object" );
  }

  itk::DataObject* output = this->GetOutput(idx);
  if ( !output )
  {
    itkExceptionMacro(<<"Requested to graft output that is a NULL pointer" );
  }
  // Call Graft on NavigationData to copy member data
  output->Graft( graft );
}

mitk::PlanarFigure* mitk::PlanarFigureSource::GetOutput(const itk::ProcessObject::DataObjectIdentifierType &key)
{
  return static_cast<mitk::PlanarFigure*>(Superclass::GetOutput(key));
}

const mitk::PlanarFigure* mitk::PlanarFigureSource::GetOutput(const itk::ProcessObject::DataObjectIdentifierType &key) const
{
  return static_cast<const mitk::PlanarFigure*>(Superclass::GetOutput(key));
}

 mitk::PlanarFigure* mitk::PlanarFigureSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx)
{
  return static_cast<mitk::PlanarFigure*>(Superclass::GetOutput(idx));
}

const  mitk::PlanarFigure* mitk::PlanarFigureSource::GetOutput(itk::ProcessObject::DataObjectPointerArraySizeType idx) const
{
  return static_cast<const mitk::PlanarFigure*>(Superclass::GetOutput(idx));
}
