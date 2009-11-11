/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11. Dez 2007) $
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlanarFigureSource.h"

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


mitk::PlanarFigureSource::DataObjectPointer mitk::PlanarFigureSource::MakeOutput ( unsigned int )
{
  return NULL;
}


void mitk::PlanarFigureSource::SetOutput( OutputType* output )
{
    itkWarningMacro( << "SetOutput(): This method is slated to be removed from ITK.  Please use GraftOutput() in possible combination with DisconnectPipeline() instead." );
    this->SetNthOutput( 0, output );
}


mitk::PlanarFigureSource::OutputType* mitk::PlanarFigureSource::GetOutput()
{
    if ( this->GetNumberOfOutputs() < 1 )
    {
        return 0;
    }

    if ( static_cast<OutputType*> ( this->ProcessObject::GetOutput( 0 ) ) == NULL )
        itkWarningMacro(<<"Output is NULL!");
    return static_cast<OutputType*> ( this->ProcessObject::GetOutput( 0 ) );
}


mitk::PlanarFigureSource::OutputType* mitk::PlanarFigureSource::GetOutput ( unsigned int idx )
{
    return static_cast<OutputType*> ( this->ProcessObject::GetOutput( idx ) );
}

void mitk::PlanarFigureSource::GenerateInputRequestedRegion()
{
    this->ProcessObject::GenerateInputRequestedRegion();
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
