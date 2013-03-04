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
