/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNDIPassiveTool.h"

#include "mitkTestingMacros.h"

#include <iostream>

/**
 *  Simple example for a test for the (non-existent) class "ClassName".
 *  
 *  argc and argv are the command line parameters which were passed to 
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
int mitkNDIPassiveToolTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("NDIPassiveTool")

  // let's create an object of our class  
  mitk::NDIPassiveTool::Pointer myNDIPassiveTool = mitk::NDIPassiveTool::New();
  
  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myNDIPassiveTool.IsNotNull(),"Testing instantiation") 

  myNDIPassiveTool->SetTrackingPriority(mitk::Dynamic);
  MITK_TEST_CONDITION(myNDIPassiveTool->GetTrackingPriority()==mitk::Dynamic,"Testing Set/GetTrackingPriority() with 'Dynamic'");
  myNDIPassiveTool->SetTrackingPriority(mitk::ButtonBox);
  MITK_TEST_CONDITION(myNDIPassiveTool->GetTrackingPriority()==mitk::ButtonBox,"Testing Set/GetTrackingPriority() with 'ButtonBox'");
  myNDIPassiveTool->SetTrackingPriority(mitk::Static);
  MITK_TEST_CONDITION(myNDIPassiveTool->GetTrackingPriority()==mitk::Static,"Testing Set/GetTrackingPriority() with 'Static'");

  // write your own tests here and use the macros from mitkTestingMacros.h !!!
  // do not write to std::cout and do not return from this function yourself!
  
  // always end with this!
  MITK_TEST_END()
}

