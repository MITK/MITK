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

#include "mitkDelayFilter.h"


mitk::DelayFilter::DataObjectPointer mitk::DelayFilter::MakeOutput ( DataObjectPointerArraySizeType )
{
    mitkThrow() << "Unsupported Operation. Please override method in sublclass if you need this functionality.";
}


mitk::DelayFilter::DataObjectPointer mitk::DelayFilter::MakeOutput( const DataObjectIdentifierType& )
{
    mitkThrow() << "Unsupported Operation. Please override method in sublclass if you need this functionality.";
}

void mitk::DelayFilter::GenerateData()
{
    //get each input and transfer the data
    DataObjectPointerArray inputs = this->GetInputs(); //get all inputs
    for (unsigned int index=0; index < inputs.size(); index++)
    {
      //get the needed variables
      //const mitk::BaseData* input = this->GetInput(index);
      //assert(input);

      //mitk::BaseData* output = this->GetOutput(index);
      //assert(output);

      // TODO: this should be delayed for this filter
      //output->Graft(input); // copy all information from input to output
    }
}
