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


#include "mitkLookupTableSource.h"

mitk::LookupTableSource::LookupTableSource()
{
    // Create the output. We use static_cast<> here because we know the default
    // output must be of type TOutputImage
    OutputType::Pointer output = static_cast<OutputType*>( this->MakeOutput( 0 ).GetPointer() );

    if ( output.GetPointer() == NULL )
    {
        itkWarningMacro(<<"Output could not be created!");
    }

    this->ProcessObject::SetNumberOfRequiredOutputs( 1 );
    this->ProcessObject::SetNthOutput( 0, output.GetPointer() );
}


mitk::LookupTableSource::~LookupTableSource()
{}




mitk::LookupTableSource::DataObjectPointer
mitk::LookupTableSource::MakeOutput ( unsigned int )
{
    return OutputType::New().GetPointer();
}




void
mitk::LookupTableSource::SetOutput( OutputType* output )
{
    itkWarningMacro( << "SetOutput(): This method is slated to be removed from ITK.  Please use GraftOutput() in possible combination with DisconnectPipeline() instead." );
    this->SetNthOutput( 0, output );
}




mitk::LookupTableSource::OutputType*
mitk::LookupTableSource::GetOutput()
{
    if ( this->GetNumberOfOutputs() < 1 )
    {
        return 0;
    }

    if ( static_cast<OutputType*> ( this->ProcessObject::GetOutput( 0 ) ) == NULL )
        itkWarningMacro(<<"Output is NULL!");
    return static_cast<OutputType*> ( this->ProcessObject::GetOutput( 0 ) );
}




mitk::LookupTableSource::OutputType*
mitk::LookupTableSource::GetOutput ( unsigned int idx )
{
    return static_cast<OutputType*> ( this->ProcessObject::GetOutput( idx ) );
}




void
mitk::LookupTableSource::GenerateInputRequestedRegion()
{
    this->ProcessObject::GenerateInputRequestedRegion();
}




void
mitk::LookupTableSource::GraftOutput( OutputType* graft )
{
    OutputType * output = this->GetOutput();

    if ( output && graft )
    {
        // grab a handle to the bulk data of the specified data object
        // output->SetPixelContainer( graft->GetPixelContainer() );

        // copy the region ivars of the specified data object
        // output->SetRequestedRegion( graft->GetRequestedRegion() );
        // output->SetLargestPossibleRegion( graft->GetLargestPossibleRegion() );
        // output->SetBufferedRegion( graft->GetBufferedRegion() );

        // copy the meta-information
        output->CopyInformation( graft );
    }
}
