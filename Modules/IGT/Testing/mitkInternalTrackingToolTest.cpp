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

#include "mitkInternalTrackingTool.h"

#include "mitkTestingMacros.h"

#include <iostream>
#include <time.h>

/**Documentation
* NDIPassiveTool has a protected constructor and a protected itkFactorylessNewMacro
* so that only it's friend class NDITrackingDevice is able to instantiate
* tool objects. Therefore, we derive from NDIPassiveTool and add a
* public itkFactorylessNewMacro, so that we can instantiate and test the class
*/
class InternalTrackingToolTestClass : public mitk::InternalTrackingTool
{
public:
  mitkClassMacro(InternalTrackingToolTestClass, InternalTrackingTool);
  /** make a public constructor, so that the test is able
  *   to instantiate NDIPassiveTool
  */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
protected:
  InternalTrackingToolTestClass() : mitk::InternalTrackingTool()
  {
  }

public: //these static methods are only to structure the test
        //please see them seperated from the upper part of the class

static void TestBasicFunctionality()
  {
  // let's create an object of our class
  mitk::InternalTrackingTool::Pointer internalTrackingTool = InternalTrackingToolTestClass::New().GetPointer();

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(internalTrackingTool.IsNotNull(),"Testing instantiation");

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
  }

static void TestTooltipFunctionality()
  {
  mitk::InternalTrackingTool::Pointer internalTrackingTool = InternalTrackingToolTestClass::New().GetPointer();
  mitk::Point3D toolTipPos; mitk::FillVector3D(toolTipPos,1,1,1);
  mitk::Quaternion toolTipQuat = mitk::Quaternion(0,0,0,1);
  internalTrackingTool->SetToolTip(toolTipPos,toolTipQuat);

  mitk::Point3D positionInput; mitk::FillVector3D(positionInput,5,6,7);

  internalTrackingTool->SetPosition(positionInput);

  mitk::Point3D positionOutput;

  internalTrackingTool->GetPosition(positionOutput);

  MITK_TEST_CONDITION(((positionOutput[0] == 6)&&
                       (positionOutput[0] == 6)&&
                       (positionOutput[0] == 6)&&
                       (positionOutput[0] == 6)),
                       "Testing tooltip definition."
                     );


  }

static void TestModiciationTimeCorrectness()
{
  mitk::InternalTrackingTool::Pointer tool = InternalTrackingToolTestClass::New().GetPointer();
  unsigned long mTime1 = tool->GetMTime();

  mitk::Point3D position1;
  mitk::FillVector3D(position1, 1.1, 2.2, 3.3);
  tool->SetPosition(position1);
  MITK_TEST_CONDITION( mTime1 < tool->GetMTime(),
                       "Testing MTime updated after initial position set" );

  mitk::Quaternion quat1 = mitk::Quaternion(0,0,0.70710678118654757,0.70710678118654757);
  tool->SetOrientation(quat1);
  MITK_TEST_CONDITION( mTime1 < tool->GetMTime(),
                       "Testing MTime updated after initial orientation set" );

  unsigned long mTime2 = tool->GetMTime();

  mitk::Point3D position2;
  mitk::FillVector3D(position2, 1.10001, 2.2, 3.3);
  tool->SetPosition(position2);
  MITK_TEST_CONDITION( mTime2 < tool->GetMTime(),
                       "Testing MTime updated after new position set" );

  unsigned long mTime3 = tool->GetMTime();

  mitk::Quaternion quat2 = mitk::Quaternion(0.0,
                                            0.0,
                                            0.70710678118654757,
                                            0.70710678118654757 + 0.00001);
  tool->SetOrientation(quat2);
  MITK_TEST_CONDITION( mTime3 < tool->GetMTime(),
                       "Testing MTime updated after new orientation set" );

  unsigned long mTime4 = tool->GetMTime();

  mitk::Point3D position3;
  mitk::FillVector3D(position3, 1.10002, 2.2, 3.3);
}
};

/**
 *  Simple example for a test for the class "InternalTrackingTool".
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

  InternalTrackingToolTestClass::TestBasicFunctionality();
  InternalTrackingToolTestClass::TestTooltipFunctionality();
  InternalTrackingToolTestClass::TestModiciationTimeCorrectness();


  // always end with this!
  MITK_TEST_END();
}

