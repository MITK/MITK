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

  #include <mitkNavigationToolStorage.h>
  #include <mitkIOUtil.h>
  #include "mitkNavigationToolStorageTestHelper.h"
  #include "mitkNDIAuroraTypeInformation.h"
  #include "mitkMicronTrackerTypeInformation.h"

mitk::NavigationToolStorage::Pointer mitk::NavigationToolStorageTestHelper::CreateTestData_SimpleStorage()
    {
    //create Tool Storage
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();
    //first tool
    mitk::NavigationTool::Pointer myTool1 = mitk::NavigationTool::New();
    myTool1->SetIdentifier("001");
    myStorage->AddTool(myTool1);
    //second tool
    mitk::NavigationTool::Pointer myTool2 = mitk::NavigationTool::New();
    myTool2->SetIdentifier("002");
    myStorage->AddTool(myTool2);
    //third tool
    mitk::NavigationTool::Pointer myTool3 = mitk::NavigationTool::New();
    myTool3->SetIdentifier("003");
    myStorage->AddTool(myTool3);

    return myStorage;
    }

  mitk::NavigationToolStorage::Pointer mitk::NavigationToolStorageTestHelper::CreateTestData_StorageWithOneTool()
    {
    //create Tool Storage
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();

    //first tool
    mitk::NavigationTool::Pointer myTool1 = mitk::NavigationTool::New();
    myTool1->SetIdentifier("001");
    mitk::PointSet::Pointer CalLandmarks1 = mitk::PointSet::New();
    mitk::Point3D testPt1;
    mitk::FillVector3D(testPt1,1,2,3);
    CalLandmarks1->SetPoint(0,testPt1);
    mitk::PointSet::Pointer RegLandmarks1 = mitk::PointSet::New();
    mitk::Point3D testPt2;
    mitk::FillVector3D(testPt2,4,5,6);
    RegLandmarks1->SetPoint(5,testPt2);
    myTool1->SetToolCalibrationLandmarks(CalLandmarks1);
    myTool1->SetToolRegistrationLandmarks(RegLandmarks1);
    mitk::Point3D toolTipPos;
    mitk::FillVector3D(toolTipPos,1.3423,2.323,4.332);
    mitk::Quaternion toolTipRot = mitk::Quaternion(0.1,0.2,0.3,0.4);
    myTool1->SetToolTipPosition(toolTipPos);
    myTool1->SetToolTipOrientation(toolTipRot);
    myStorage->AddTool(myTool1);

    return myStorage;
    }

  mitk::NavigationToolStorage::Pointer mitk::NavigationToolStorageTestHelper::CreateTestData_ComplexStorage(std::string toolFilePath, std::string toolSurfacePath1, std::string toolSurfacePath2)
    {
    //create first tool
    mitk::NavigationTool::Pointer myNavigationTool = mitk::NavigationTool::New();
    myNavigationTool->SetCalibrationFile(toolFilePath);
    mitk::DataNode::Pointer myNode = mitk::DataNode::New();
    myNode->SetName("ClaronTool");
    myNode->SetData(mitk::IOUtil::LoadSurface(toolSurfacePath1)); //load an stl File
    myNavigationTool->SetDataNode(myNode);
    myNavigationTool->SetIdentifier("ClaronTool#1");
    myNavigationTool->SetSerialNumber("0815");
    myNavigationTool->SetTrackingDeviceType(mitk::MicronTrackerTypeInformation::GetTrackingDeviceName());
    myNavigationTool->SetType(mitk::NavigationTool::Fiducial);

    //create second tool
    mitk::NavigationTool::Pointer myNavigationTool2 = mitk::NavigationTool::New();
    mitk::Surface::Pointer testSurface2;

    mitk::DataNode::Pointer myNode2 = mitk::DataNode::New();
    myNode2->SetName("AuroraTool");

    //load an stl File
    testSurface2 = mitk::IOUtil::LoadSurface(toolSurfacePath2);
    myNode2->SetData(testSurface2);

    myNavigationTool2->SetDataNode(myNode2);
    myNavigationTool2->SetIdentifier("AuroraTool#1");
    myNavigationTool2->SetSerialNumber("0816");
    myNavigationTool2->SetTrackingDeviceType(mitk::NDIAuroraTypeInformation::GetTrackingDeviceName());
    myNavigationTool2->SetType(mitk::NavigationTool::Instrument);

    //create navigation tool storage
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();
    myStorage->AddTool(myNavigationTool);
    myStorage->AddTool(myNavigationTool2);

    return myStorage;
    }
