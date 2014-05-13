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

#include "mitkVirtualTrackingDevice.h"

#include "itksys/SystemTools.hxx"
#include "mitkTestingMacros.h"
#include "mitkTrackingTool.h"
#include <iomanip>


int mitkVirtualTrackingDeviceTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("VirtualTrackingDevice");

  // let's create an object of our class
  mitk::VirtualTrackingDevice::Pointer tracker = mitk::VirtualTrackingDevice::New();
  tracker->SetRefreshRate(10);

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(tracker.IsNotNull(),"Testing instantiation\n");


  MITK_TEST_CONDITION_REQUIRED(tracker->GetState() == mitk::TrackingDevice::Setup ,"Checking tracking device state == setup.\n");

  //CloseConnection
  MITK_TEST_CONDITION( (tracker->CloseConnection()), "Testing behavior of method CloseConnection().");

  //StartTracking
  MITK_TEST_CONDITION( tracker->StartTracking() == false, "Testing behavior of method StartTracking().");

  tracker->SetRefreshRate(43);
  MITK_TEST_CONDITION( tracker->GetRefreshRate() == 43, "Testing Set-/GetRefreshRate()");

  MITK_TEST_CONDITION( tracker->GetToolCount() == 0, "Testing GetToolCount() before AddTool()");

  MITK_TEST_CONDITION( tracker->AddTool("Tool0"), "Testing AddTool() for tool 0.");
  MITK_TEST_CONDITION( tracker->GetToolCount() == 1, "Testing GetToolCount() after AddTool()");

  mitk::TrackingTool::Pointer tool = tracker->GetTool(0);
  MITK_TEST_CONDITION_REQUIRED( tool.IsNotNull(), "Testing GetTool() for tool 0.");

  MITK_TEST_CONDITION( tracker->GetToolByName("Tool0") == tool.GetPointer(), "Testing GetTool() equals GetToolByName() for tool 0.");

  mitk::ScalarType bounds[6] = {0.0, 10.0, 1.0, 20.0, 3.0, 30.0};
  tracker->SetBounds(bounds);
  MITK_TEST_CONDITION( tracker->GetBounds()[0] == bounds[0]
                    && tracker->GetBounds()[1] == bounds[1]
                    && tracker->GetBounds()[2] == bounds[2]
                    && tracker->GetBounds()[3] == bounds[3]
                    && tracker->GetBounds()[4] == bounds[4]
                    && tracker->GetBounds()[5] == bounds[5]
                    , "Testing Set-/GetBounds()");
  MITK_TEST_CONDITION( tracker->AddTool("Tool1"), "Testing AddTool() for tool 1.");
  MITK_TEST_CONDITION( tracker->GetToolCount() == 2, "Testing GetToolCount() after AddTool()");

  tracker->SetToolSpeed(0, 0.1); // no exception expected
  tracker->SetToolSpeed(1, 0.1); // no exception expected
  MITK_TEST_FOR_EXCEPTION(std::invalid_argument, tracker->SetToolSpeed(2, 0.1));  // exception expected

  mitk::ScalarType lengthBefore = tracker->GetSplineChordLength(0); // no exception expected
  MITK_TEST_FOR_EXCEPTION(std::invalid_argument, tracker->GetSplineChordLength(2));  // exception expected


  MITK_TEST_CONDITION( tracker->OpenConnection() == true, "Testing OpenConnection().");
  MITK_TEST_CONDITION( tracker->GetSplineChordLength(0)  == lengthBefore, "Testing GetSplineChordLength() after initalization");

  //StartTracking
  mitk::Point3D posBefore0;
  tool->GetPosition(posBefore0);
  unsigned long tmpMTimeBefore0 = tool->GetMTime();
  mitk::Point3D posBefore1;
  tracker->GetToolByName("Tool1")->GetPosition(posBefore1);
  unsigned long tmpMTimeBefore1 = tracker->GetToolByName("Tool1")->GetMTime();

  mitk::Point3D posAfter0;
  tool->GetPosition(posAfter0);
  MITK_TEST_CONDITION( mitk::Equal(posBefore0, posAfter0) == true, "Testing if position value is constant before StartTracking()");


  MITK_TEST_CONDITION( tracker->StartTracking() == true, "Testing behavior of method StartTracking().");
  itksys::SystemTools::Delay(500); // wait for tracking thread to start generating positions

  tool->GetPosition(posAfter0);
  if(tracker->GetToolByName("Tool0")->GetMTime() == tmpMTimeBefore0) //tool not modified yet
  {
    MITK_TEST_CONDITION( mitk::Equal(posBefore0, posAfter0) == true, "Testing if tracking is producing new position values in tool 0.");
  }
  else
  {
    MITK_TEST_CONDITION( mitk::Equal(posBefore0, posAfter0) == false, "Testing if tracking is producing new position values in tool 0.");
  }

  mitk::Point3D posAfter1;
  tracker->GetToolByName("Tool1")->GetPosition(posAfter1);
  if(tracker->GetToolByName("Tool1")->GetMTime() == tmpMTimeBefore1) //tool not modified yet
  {
    MITK_TEST_CONDITION( mitk::Equal(posBefore1, posAfter1) == true, "Testing if tracking is producing new position values in tool 1.");
  }
  else
  {
    MITK_TEST_CONDITION( mitk::Equal(posBefore1, posAfter1) == false, "Testing if tracking is producing new position values in tool 1.");
  }

  // temporarly deactivated the test due to bug 17791
  /*
  // add tool while tracking is in progress
  tracker->AddTool("while Running");

  itksys::SystemTools::Delay(100); // wait for tracking thread to start generating positions
  tracker->GetToolByName("while Running")->GetPosition(posBefore0);
  unsigned long tmpBeforeMTime = tracker->GetToolByName("while Running")->GetMTime();
  itksys::SystemTools::Delay(100); // wait for tracking thread to start generating positions
  tracker->GetToolByName("while Running")->GetPosition(posAfter0);
  unsigned long tmpAfterMTime = tracker->GetToolByName("while Running")->GetMTime();
  MITK_INFO << "If this test fails, please reopen bug 8033 and commit this output: ";
  MITK_INFO << std::setprecision(16) << "Value of posBefore0 " << posBefore0;
  MITK_INFO << std::setprecision(16) << "Value of posAfter0 " << posAfter0;
  MITK_INFO << std::setprecision(16) << "tmpTime " << tmpBeforeMTime;
  MITK_INFO << std::setprecision(16) << "current time " << tmpAfterMTime;
  if(tmpAfterMTime == tmpBeforeMTime) //tool not modified yet
  {
    //hence the tool was not modified, the position has to be equal
    MITK_TEST_CONDITION( mitk::Equal(posBefore0, posAfter0) == true, "Testing if the position values for the 'while running' tool remain the same.");
  }
  else //tool was modified => position should have changed
  {
    MITK_TEST_CONDITION( mitk::Equal(posBefore0, posAfter0) == false, "Testing if tracking is producing new position values for 'while running' tool.");
  }
  */

  // always end with this!
  MITK_TEST_END();
}
