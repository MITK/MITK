/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 16011 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataEvaluationFilter.h"


mitk::NavigationDataEvaluationFilter::NavigationDataEvaluationFilter()
: mitk::NavigationDataToNavigationDataFilter()
{

}


mitk::NavigationDataEvaluationFilter::~NavigationDataEvaluationFilter()
{

}

void mitk::NavigationDataEvaluationFilter::GenerateData()
{

    this->CreateOutputsForAllInputs(); // make sure that we have the same number of outputs as inputs

    /* update outputs with tracking data from tools */
    for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)
    {
      mitk::NavigationData* output = this->GetOutput(i);
      assert(output);
      const mitk::NavigationData* input = this->GetInput(i);
      assert(input);

      if (input->IsDataValid() == false)
      {
        output->SetDataValid(false);
        continue;
      }

     

     // output->SetOrientation(quatOut);


    }
  }
}
