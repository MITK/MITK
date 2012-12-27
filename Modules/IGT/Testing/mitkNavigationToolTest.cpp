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

#include "mitkNavigationTool.h"
#include "mitkCommon.h"
#include "mitkTestingMacros.h"
#include "mitkDataNode.h"
#include "mitkPointSet.h"
#include "mitkTrackingTool.h"

#include <itkSpatialObject.h>

class mitkNavigationToolTestClass
  {
  public:

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
    myNavigationTool->SetTrackingDeviceType(mitk::NDIAurora);

    mitk::PointSet::Pointer CalLandmarks = mitk::PointSet::New();
    mitk::Point3D testPt1;
    mitk::FillVector3D(testPt1,1,2,3);
    CalLandmarks->SetPoint(0,testPt1);

    mitk::PointSet::Pointer RegLandmarks = mitk::PointSet::New();
    mitk::Point3D testPt2;
    mitk::FillVector3D(testPt2,4,5,6);
    RegLandmarks->SetPoint(0,testPt2);

    myNavigationTool->SetToolCalibrationLandmarks(CalLandmarks);
    myNavigationTool->SetToolRegistrationLandmarks(RegLandmarks);

    //test getter
    MITK_TEST_CONDITION(myNavigationTool->GetType()==mitk::NavigationTool::Instrument,"Testing getter and setter of type.");
    MITK_TEST_CONDITION(myNavigationTool->GetIdentifier()=="Tool#15","Testing getter and setter of identifier.");
    MITK_TEST_CONDITION(myNavigationTool->GetDataNode()==myNode,"Testing getter and setter of dataNode.");
    MITK_TEST_CONDITION(myNavigationTool->GetSpatialObject()==mySpatialObject,"Testing getter and setter of itk spatial object.");
    MITK_TEST_CONDITION(myNavigationTool->GetCalibrationFile()=="none","Testing getter and setter of calibration file."); //should be none, because file does not exist
    MITK_TEST_CONDITION(myNavigationTool->GetSerialNumber()=="0815","Testing getter and setter of serial number.");
    MITK_TEST_CONDITION(myNavigationTool->GetTrackingDeviceType()==mitk::NDIAurora,"Testing getter and setter of tracking device type.");
    MITK_TEST_CONDITION(myNavigationTool->GetToolName()=="TestNodeName","Testing method GetToolName().");
    MITK_TEST_CONDITION(myNavigationTool->GetToolCalibrationLandmarks()->GetPoint(0)[0] == 1.0,"Testing method GetToolCalibrationLandmarks()");
    MITK_TEST_CONDITION(myNavigationTool->GetToolRegistrationLandmarks()->GetPoint(0)[0] == 4.0,"Testing method GetToolRegistrationLandmarks()");
    }

  };
/** This function is testing the TrackingVolume class. */
int mitkNavigationToolTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationTool")

  mitkNavigationToolTestClass::TestInstantiation();
  mitkNavigationToolTestClass::TestGetterAndSetter();

  MITK_TEST_END()
}


