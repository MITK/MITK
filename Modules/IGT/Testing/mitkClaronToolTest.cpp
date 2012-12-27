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

#include "mitkClaronTool.h"
#include "mitkTestingMacros.h"

/**Documentation
* ClaronTool has a protected constructor and a protected itkNewMacro
* so that only it's friend class ClaronTrackingDevice is able to instantiate
* tool objects. Therefore, we derive from ClaronTool and add a
* public itkNewMacro, so that we can instantiate and test the class
*/
class ClaronToolTestClass : public mitk::ClaronTool
{
public:
  mitkClassMacro(ClaronToolTestClass, ClaronTool);
  /** make a public constructor, so that the test is able
  *   to instantiate NDIPassiveTool
  */
  itkNewMacro(Self);
protected:
  ClaronToolTestClass() : mitk::ClaronTool()
  {
  }
};

/**
 * This function tests the ClaronTool class.
 */
int mitkClaronToolTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("ClaronTool")
  {
    // let's create an object of our class
    mitk::ClaronTool::Pointer myClaronTool = ClaronToolTestClass::New().GetPointer();

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
    MITK_TEST_CONDITION(myClaronTool->GetFile() == name ,"Testing GetFile() after setting file name with LoadFile()");
    MITK_TEST_CONDITION(myClaronTool->LoadFile(NULL) == false ,"Test LoadFile() with NULL as parameter.")
    MITK_TEST_CONDITION(myClaronTool->LoadFile(std::string("")) == false ,"Test LoadFile() with empty string as parameter.")

    myClaronTool->SetToolHandle(011022);
    MITK_TEST_CONDITION(myClaronTool->GetToolHandle() == 011022 ,"Test SetToolHandle() and GetToolHandle().")

  }
  // always end with this!
  MITK_TEST_END()
}

