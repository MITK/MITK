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

#include "mitkNavigationDataPassThroughFilter.h"

mitk::NavigationDataPassThroughFilter::NavigationDataPassThroughFilter()
{
}

mitk::NavigationDataPassThroughFilter::~NavigationDataPassThroughFilter()
{
}

void mitk::NavigationDataPassThroughFilter::GenerateData()
{
  // get each input and transfer the data
  DataObjectPointerArray inputs = this->GetInputs(); //get all inputs
  for ( unsigned int index = 0; index < inputs.size(); ++index )
  {
    // get the needed variables (input and output)
    const mitk::NavigationData* nd = this->GetInput(index);
    mitk::NavigationData* output = this->GetOutput(index);

    if ( ! nd || ! output )
    {
      MITK_ERROR("NavigationDataToNavigationDataFilter")("NavigationDataPassThroughFilter")
          << "Input and output must not be null.";
      mitkThrow() << "Input and output must not be null.";
    }

    output->Graft(nd); // copy all information from input to output
    output->SetDataValid(nd->IsDataValid());
  }
}
