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


#include "mitkStepper.h"
#include "mitkMultiStepper.h"
#include "mitkTestingMacros.h"

int mitkMultiStepperTest(int /*argc*/, char* /*argv*/[])
{
  mitk::Stepper::Pointer stepperA;
  stepperA = mitk::Stepper::New();

  mitk::Stepper::Pointer stepperB = mitk::Stepper::New();
  stepperA->SetSteps(4);
  stepperB->SetSteps(6);
  mitk::MultiStepper::Pointer multiStepper = mitk::MultiStepper::New();

  multiStepper->AddStepper(stepperA,2);
  multiStepper->AddStepper(stepperB);

  for (int i=0 ; i<10; i++) {
    std::cout << i << ": A: " << stepperA->GetPos() << " B:" << stepperB->GetPos() << std::endl;
    multiStepper->Next();
  }

  return EXIT_SUCCESS;
}
