/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkClaronTool.h"
#include "mitkTestingMacros.h"

/**
 * This function tests the ClaronTool class.
 */
int mitkClaronToolTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("ClaronTool")
  {
    // let's create an object of our class  
    mitk::ClaronTool::Pointer myClaronTool = mitk::ClaronTool::New();

    // first test: did this work?
    // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
    // it makes no sense to continue without an object.
    MITK_TEST_CONDITION_REQUIRED(myClaronTool.IsNotNull(),"Testing instantiation:") 
  
      mitk::Point3D pos;
    pos[0] = 10;
    pos[1] = 20;
    pos[2] = 30;
    myClaronTool->SetPosition(pos);
    mitk::Point3D testPos;
    myClaronTool->GetPosition(testPos);
    MITK_TEST_CONDITION((testPos==pos),"Testing position update:")

      mitk::Quaternion orientation(10,20,30,40);
      myClaronTool->SetOrientation(orientation);
    mitk::Quaternion testOri;
    myClaronTool->GetOrientation(testOri);
    MITK_TEST_CONDITION((testOri==orientation),"Testing orientation update:")

    std::string name = "//testfilename";
    std::string name1 = "/testfilename";
    std::string name2 = "testfilename";
    MITK_TEST_CONDITION(myClaronTool->LoadFile(name) == true ,"Test LoadFile() with valid windows file name")
    MITK_TEST_CONDITION(myClaronTool->LoadFile(name1) == true ,"Test LoadFile() with valid Linux file name")
    MITK_TEST_CONDITION(myClaronTool->LoadFile(name2) == false, "Test LoadFile() with invalid file name")
    MITK_TEST_CONDITION(myClaronTool->GetCalibrationName() == name2 ,"Test GetCalibrationName() after setting with LoadFile()")
    myClaronTool->SetCalibrationName(name);
    MITK_TEST_CONDITION(myClaronTool->GetCalibrationName() == name ,"Test GetCalibrationName() after setting with SetCalibrationName()")

    myClaronTool->LoadFile(name);
    MITK_TEST_CONDITION(myClaronTool->GetFile() == name ,"Testing GetFile() after setting file name with LoadFile()")

  }
  // always end with this!
  MITK_TEST_END()
}

