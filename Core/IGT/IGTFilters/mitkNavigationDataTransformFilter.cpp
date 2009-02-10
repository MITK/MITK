/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkNavigationDataTransformFilter.h"


mitk::NavigationDataTransformFilter::NavigationDataTransformFilter() 
: mitk::NavigationDataToNavigationDataFilter()
{}


mitk::NavigationDataTransformFilter::~NavigationDataTransformFilter()
{}


void mitk::NavigationDataTransformFilter::GenerateData()
{

  //this->CreateOutputsForAllInputs(); // make sure that we have the same number of outputs as inputs
  // \warning This could erase outputs if inputs have been removed
  
  // only update data if m_Transform was set
  if(m_Transform)
  {
    /* update outputs with tracking data from tools */
    for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)
    {
      mitk::NavigationData* output = this->GetOutput(i);
      assert(output);
      const mitk::NavigationData* input = this->GetInput(i);
      assert(input);

      if (input->IsDataValid() == false)
      {
        continue;
      }

      mitk::NavigationData::PositionType tempCoordinateIn, tempCoordinateOut;
      float tempPositionIn[3], tempPositionOut[3];

      //convert to vtk
      tempCoordinateIn = input->GetPosition();
      tempPositionIn[0] = tempCoordinateIn[0]; 
      tempPositionIn[1] = tempCoordinateIn[1]; 
      tempPositionIn[2] = tempCoordinateIn[2]; 
      
      //do the transform
      m_Transform->Update();
      m_Transform->InternalTransformPoint( tempPositionIn, tempPositionOut );  
      //InternalTransformPoint don't call the update of the Transform

      //convert back to mitk
      tempCoordinateOut[0] = tempPositionOut[0]; 
      tempCoordinateOut[1] = tempPositionOut[1]; 
      tempCoordinateOut[2] = tempPositionOut[2]; 

      output->Graft(input); // First, copy all information from input to output
      output->SetPosition(tempCoordinateOut);// Then change the member(s): add new position of navigation data after tranformation
      output->SetDataValid(true); // operation was successful, therefore data of output is valid.
    }
  }
}