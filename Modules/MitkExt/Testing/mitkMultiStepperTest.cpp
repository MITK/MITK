/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-13 14:52:01 +0200 (Wed, 13 May 2009) $
Version:   $Revision: 17230 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
