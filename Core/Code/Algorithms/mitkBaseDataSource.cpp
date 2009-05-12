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


#include "mitkBaseDataSource.h"


mitk::BaseDataSource::BaseDataSource()
{
    // Pure virtual functions may not be called in the constructor...
    // see ( see Bjarne Stroustrup's C++ PL 3rd ed section 15.4.3 )
    //OutputType::Pointer output = static_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
    //Superclass::SetNumberOfRequiredOutputs( 1 );
    //Superclass::SetNthOutput( 0, output.GetPointer() );
}



mitk::BaseDataSource::~BaseDataSource()
{
}



void mitk::BaseDataSource::SetOutput( OutputType* output )
{
    this->SetNthOutput( 0, output );
}



void mitk::BaseDataSource::SetOutput( unsigned int idx, OutputType* output )
{
    this->SetNthOutput(idx, output);    
}



mitk::BaseDataSource::OutputType* mitk::BaseDataSource::GetOutput()
{
    if ( this->GetNumberOfOutputs() < 1 )
    {
        return 0;
    }
    return static_cast<OutputType*> ( Superclass::GetOutput( 0 ) );
}



mitk::BaseDataSource::OutputType* mitk::BaseDataSource::GetOutput ( unsigned int idx )
{
    return static_cast<OutputType*> ( Superclass::GetOutput( idx ) );
}


