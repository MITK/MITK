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


#include "mitkMeshSource.h"


mitk::MeshSource::MeshSource()
{
    // Create the output.
    OutputType::Pointer output = dynamic_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
    assert (output.IsNotNull());
    this->SetNumberOfRequiredInputs(0);
    this->SetNumberOfOutputs( 1 );
    this->SetOutput(0, output.GetPointer());  
}




mitk::MeshSource::~MeshSource()
{
}




itk::DataObject::Pointer mitk::MeshSource::MakeOutput ( unsigned int /*idx */)
{
    return OutputType::New().GetPointer();
}




void mitk::MeshSource::SetOutput( OutputType* output )
{
    this->ProcessObject::SetNthOutput( 0, output );
}




void mitk::MeshSource::SetOutput( unsigned int idx, OutputType* output )
{
    this->ProcessObject::SetNthOutput(idx, output);    
}




mitk::MeshSource::OutputType* mitk::MeshSource::GetOutput()
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




mitk::MeshSource::OutputType* mitk::MeshSource::GetOutput ( unsigned int idx )
{
    return dynamic_cast<OutputType*> ( this->ProcessObject::GetOutput( idx ) );
}


