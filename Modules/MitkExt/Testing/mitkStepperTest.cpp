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


#include "mitkStepper.h"
#include "mitkMultiStepper.h"

int mitkStepperTest(int /*argc*/, char* /*argv*/[])
{
  mitk::Stepper::Pointer stepperA;
  std::cout << "Testing mitk::Stepper::New(): ";
  stepperA = mitk::Stepper::New();
  if (stepperA.IsNull()) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
    std::cout<<"[PASSED]"<<std::endl;
  }
  
  mitk::Stepper::Pointer stepperB = mitk::Stepper::New();
  stepperA->SetSteps(4);
  //stepperA->PingPongOn();
  stepperB->SetSteps(6);
  // stepperB->PingPongOn();
  /* for (int i=0 ; i<10; i++) {
    std::cout << i << ": A: " << stepperA->GetPos() << " B:" << stepperB->GetPos() << std::endl; 
    stepperA->Next();
    stepperB->Next();
  }*/
  std::cout << "Multi Stepper Test" << std::endl;
  mitk::MultiStepper::Pointer multiStepper = mitk::MultiStepper::New();

  multiStepper->AddStepper(stepperA,2);
  multiStepper->AddStepper(stepperB);
  
  for (int i=0 ; i<10; i++) {
    std::cout << i << ": A: " << stepperA->GetPos() << " B:" << stepperB->GetPos() << std::endl; 
    multiStepper->Next();
  }
 
  return EXIT_SUCCESS;
}
