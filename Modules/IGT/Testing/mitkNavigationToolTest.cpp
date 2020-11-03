/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNavigationTool.h"
#include "mitkCommon.h"
#include "mitkTestingMacros.h"
#include "mitkDataNode.h"
#include "mitkPointSet.h"
#include "mitkTrackingTool.h"

#include <itkSpatialObject.h>

#include "mitkNDIAuroraTypeInformation.h"

class mitkNavigationToolTestClass
  {
  public:

    static bool CompareQuaternions(mitk::Quaternion q1, mitk::Quaternion q2)
    {
      if ((q1.as_vector() - q2.as_vector()).one_norm() < 10e-3)
        return true;
      else
        return false;
    }
    static void TestInstantiation()
    {
    // let's create an object of our class
    mitk::NavigationTool::Pointer myNavigationTool = mitk::NavigationTool::New();
    MITK_TEST_CONDITION_REQUIRED(myNavigationTool.IsNotNull(),"Testing instantiation")
    }

    static void TestGetterAndSetter()
    {
    mitk::NavigationTool::Pointer myNavigationTool = mitk::NavigationTool::New();

    //initialize a few things
    mitk::DataNode::Pointer myNode = mitk::DataNode::New();
    myNode->SetName("TestNodeName");
    itk::SpatialObject<3>::Pointer mySpatialObject = itk::SpatialObject<3>::New();

    //set everything
    myNavigationTool->SetType(mitk::NavigationTool::Instrument);
    myNavigationTool->SetIdentifier("Tool#15");
    myNavigationTool->SetDataNode(myNode);
    myNavigationTool->SetSpatialObject(mySpatialObject);
    //notice: cannot test Get/SetTrackingTool because this class cannot be instantiated alone
    myNavigationTool->SetCalibrationFile("Test.srom");
    myNavigationTool->SetSerialNumber("0815");
    myNavigationTool->SetTrackingDeviceType(mitk::NDIAuroraTypeInformation::GetTrackingDeviceName());

    mitk::PointSet::Pointer CalLandmarks = mitk::PointSet::New();
    mitk::Point3D testPt1;
    mitk::FillVector3D(testPt1,1,2,3);
    CalLandmarks->SetPoint(0,testPt1);

    mitk::PointSet::Pointer RegLandmarks = mitk::PointSet::New();
    mitk::Point3D testPt2;
    mitk::FillVector3D(testPt2,4,5,6);
    RegLandmarks->SetPoint(0,testPt2);

    myNavigationTool->SetToolControlPoints(CalLandmarks);
    myNavigationTool->SetToolLandmarks(RegLandmarks);

    //test getter
    MITK_TEST_CONDITION(myNavigationTool->GetType()==mitk::NavigationTool::Instrument,"Testing getter and setter of type.");
    MITK_TEST_CONDITION(myNavigationTool->GetIdentifier()=="Tool#15","Testing getter and setter of identifier.");
    MITK_TEST_CONDITION(myNavigationTool->GetDataNode()==myNode,"Testing getter and setter of dataNode.");
    MITK_TEST_CONDITION(myNavigationTool->GetSpatialObject()==mySpatialObject,"Testing getter and setter of itk spatial object.");
    MITK_TEST_CONDITION(myNavigationTool->GetCalibrationFile()=="none","Testing getter and setter of calibration file."); //should be none, because file does not exist
    MITK_TEST_CONDITION(myNavigationTool->GetSerialNumber()=="0815","Testing getter and setter of serial number.");
    MITK_TEST_CONDITION(myNavigationTool->GetTrackingDeviceType() == mitk::NDIAuroraTypeInformation::GetTrackingDeviceName(), "Testing getter and setter of tracking device type.");
    MITK_TEST_CONDITION(myNavigationTool->GetToolName()=="TestNodeName","Testing method GetToolName().");
    MITK_TEST_CONDITION(myNavigationTool->GetToolControlPoints()->GetPoint(0)[0] == 1.0,"Testing method GetToolControlPoints()");
    MITK_TEST_CONDITION(myNavigationTool->GetToolLandmarks()->GetPoint(0)[0] == 4.0,"Testing method GetToolLandmarks()");
    }

    static void TestToolTipAndAxis()
    {
      // let's create an object of our class
      mitk::NavigationTool::Pointer myNavigationTool = mitk::NavigationTool::New();
      MITK_TEST_CONDITION_REQUIRED(myNavigationTool.IsNotNull(), "Testing instantiation");
      // define tool tip
      mitk::Point3D toolTip;
      mitk::FillVector3D(toolTip, 1.0, 3.4, 5.6);
      // test Set / GetToolTipPosition
      myNavigationTool->SetToolTipPosition(toolTip);
      MITK_TEST_CONDITION(myNavigationTool->GetToolTipPosition() == toolTip, "Testing Set / GetToolTipPosition");
      // define tool axis
      mitk::Point3D toolAxis;
      mitk::FillVector3D(toolAxis, 1.0, 0.0, 0.0);
      // This tool axis will result in the following transformation
      mitk::Quaternion resultingToolAxisOrientation(0, -sqrt(2)/2.0, 0, sqrt(2)/2.0);
      // test calculation of ToolAxisOrientation
      myNavigationTool->SetToolAxis(toolAxis);
      MITK_TEST_CONDITION(CompareQuaternions(myNavigationTool->GetToolAxisOrientation(),resultingToolAxisOrientation), "Testing caluclation of ToolAxisOrientation");
    }

  };
/** This function is testing the TrackingVolume class. */
int mitkNavigationToolTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationTool")

  mitkNavigationToolTestClass::TestInstantiation();
  mitkNavigationToolTestClass::TestGetterAndSetter();
  mitkNavigationToolTestClass::TestToolTipAndAxis();
  MITK_TEST_END()
}


