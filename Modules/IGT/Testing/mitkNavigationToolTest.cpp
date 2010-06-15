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

#include "mitkNavigationTool.h"
#include "mitkCommon.h"
#include "mitkTestingMacros.h"
#include "mitkDataNode.h"
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

    //test getter
    MITK_TEST_CONDITION(myNavigationTool->GetType()==mitk::NavigationTool::Instrument,"Testing getter and setter of type.");
    MITK_TEST_CONDITION(myNavigationTool->GetIdentifier()=="Tool#15","Testing getter and setter of identifier.");
    MITK_TEST_CONDITION(myNavigationTool->GetDataNode()==myNode,"Testing getter and setter of dataTreeNode.");
    MITK_TEST_CONDITION(myNavigationTool->GetSpatialObject()==mySpatialObject,"Testing getter and setter of itk spatial object.");
    MITK_TEST_CONDITION(myNavigationTool->GetCalibrationFile()=="Test.srom","Testing getter and setter of calibration file.");
    MITK_TEST_CONDITION(myNavigationTool->GetSerialNumber()=="0815","Testing getter and setter of serial number.");
    MITK_TEST_CONDITION(myNavigationTool->GetTrackingDeviceType()==mitk::NDIAurora,"Testing getter and setter of tracking device type.");

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


