/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMultiStepper.h"
#include "mitkStepper.h"
#include "mitkTestingMacros.h"

int mitkMultiStepperTest(int /*argc*/, char * /*argv*/ [])
{
  mitk::Stepper::Pointer stepperA;
  stepperA = mitk::Stepper::New();

  mitk::Stepper::Pointer stepperB = mitk::Stepper::New();
  stepperA->SetSteps(4);
  stepperB->SetSteps(6);
  mitk::MultiStepper::Pointer multiStepper = mitk::MultiStepper::New();

  multiStepper->AddStepper(stepperA, 2);
  multiStepper->AddStepper(stepperB);

  for (int i = 0; i < 10; i++)
  {
    std::cout << i << ": A: " << stepperA->GetPos() << " B:" << stepperB->GetPos() << std::endl;
    multiStepper->Next();
  }

  return EXIT_SUCCESS;
}
