/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date $
Version:   $Revision $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkInternalTrackingTool.h"

#include "mitkTestingMacros.h"

#include <iostream>
#include <time.h>

/**
 *  Simple example for a test for the (non-existent) class "ClassName".
 *  
 *  argc and argv are the command line parameters which were passed to 
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
int mitkInternalTrackingToolTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("InternalTrackingTool")

  // let's create an object of our class  
  mitk::InternalTrackingTool::Pointer internalTrackingTool = mitk::InternalTrackingTool::New();
  
  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(internalTrackingTool.IsNotNull(),"Testing instantiation")

    // test for Enable()
  internalTrackingTool->Enable();
  MITK_TEST_CONDITION((internalTrackingTool->IsEnabled()==true),"Testing of Enable()");

  srand(time(NULL));
  // generate a random position to test Set/GetPosition()
  mitk::Point3D position;
  position[0] = rand()%1000;
  position[1] = rand()%1000;
  position[2] = rand()%1000;
  internalTrackingTool->SetPosition(position);
  mitk::Point3D returnedPosition;
  returnedPosition.Fill(0);
  internalTrackingTool->GetPosition(returnedPosition);
  MITK_TEST_CONDITION((position==returnedPosition),"Testing of Set/GetPosition()");

  // generate a random orientation to test Set/GetOrientation()
  mitk::Quaternion orientation;
  orientation[0] = (rand()%1000)/1000.0;
  orientation[1] = (rand()%1000)/1000.0;
  orientation[2] = (rand()%1000)/1000.0;
  orientation[3] = (rand()%1000)/1000.0;
  internalTrackingTool->SetOrientation(orientation);
  mitk::Quaternion returnedOrientation(0,0,0,0);
  internalTrackingTool->GetOrientation(returnedOrientation);
  MITK_TEST_CONDITION((orientation==returnedOrientation),"Testing of Set/GetQuaternion()");

  // test Set/GetTrackingError()
  float trackingError = rand()%2;
  internalTrackingTool->SetTrackingError(trackingError);
  MITK_TEST_CONDITION((internalTrackingTool->GetTrackingError()==trackingError),"Testing of Set/GetTrackingError()");
  // test Set/GetDataValid()
  internalTrackingTool->SetDataValid(true);
  MITK_TEST_CONDITION((internalTrackingTool->IsDataValid()==true),"Testing of SetDataValid and IsDataValid() for parameter 'true'");
  internalTrackingTool->SetDataValid(false);
  MITK_TEST_CONDITION((internalTrackingTool->IsDataValid()==false),"Testing of SetDataValid and IsDataValid() for parameter 'false'");

  internalTrackingTool->Disable();
  MITK_TEST_CONDITION((internalTrackingTool->IsEnabled()==false),"Testing of Disable()");

  // write your own tests here and use the macros from mitkTestingMacros.h !!!
  // do not write to std::cout and do not return from this function yourself!
  
  // always end with this!
  MITK_TEST_END()
}

