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


#include "mitkDataNodeSource.h"


mitk::DataNodeSource::DataNodeSource()
{
    // Create the output.
    OutputType::Pointer output = dynamic_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
    assert (output.IsNotNull());
    this->SetNumberOfOutputs( 1 );
    this->SetOutput(0, output.GetPointer());
}




mitk::DataNodeSource::~DataNodeSource()
{
}




itk::DataObject::Pointer mitk::DataNodeSource::MakeOutput ( unsigned int /*idx*/ )
{
    return OutputType::New().GetPointer();
}




void mitk::DataNodeSource::SetOutput( OutputType* output )
{
    this->itk::ProcessObject::SetNthOutput( 0, output );
}




void mitk::DataNodeSource::SetOutput( unsigned int idx, OutputType* output )
{
    this->itk::ProcessObject::SetNthOutput(idx, output);    
}




mitk::DataNodeSource::OutputType* mitk::DataNodeSource::GetOutput()
{
    if ( this->GetNumberOfOutputs() < 1 )
    {
        return 0;
    }
    else
    {
        return dynamic_cast<OutputType*> ( this->GetOutput( 0 ) );
    }
}




mitk::DataNodeSource::OutputType* mitk::DataNodeSource::GetOutput ( unsigned int idx )
{
    return dynamic_cast<OutputType*> ( this->itk::ProcessObject::GetOutput( idx ) );
}


