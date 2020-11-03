/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
