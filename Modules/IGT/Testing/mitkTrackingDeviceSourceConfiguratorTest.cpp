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

#include "mitkTrackingDeviceSourceConfigurator.h"
#include <mitkClaronTrackingDevice.h>
#include <mitkNDITrackingDevice.h>
#include <mitkNavigationTool.h>
#include <mitkTestingMacros.h>
#include <mitkIGTConfig.h>

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include "mitkTrackingDeviceTypeCollection.h"
#include "mitkUnspecifiedTrackingTypeInformation.h"

//All Tracking devices, which should be avaiable by default
#include "mitkNDIAuroraTypeInformation.h"
#include "mitkNDIPolarisTypeInformation.h"
#include "mitkVirtualTrackerTypeInformation.h"
#include "mitkMicronTrackerTypeInformation.h"
#include "mitkNPOptitrackTrackingTypeInformation.h"
#include "mitkOpenIGTLinkTypeInformation.h"


static void TestInstantiation()
{
  // let's create an object of our class
  mitk::TrackingDeviceSourceConfigurator::Pointer testInstance;
  mitk::NavigationToolStorage::Pointer emptyStorage = mitk::NavigationToolStorage::New();
  mitk::TrackingDevice::Pointer dummyDevice = dynamic_cast<mitk::TrackingDevice*>(mitk::ClaronTrackingDevice::New().GetPointer());
  testInstance = mitk::TrackingDeviceSourceConfigurator::New(emptyStorage,dummyDevice);
  MITK_TEST_CONDITION_REQUIRED(testInstance.IsNotNull(),"Testing instantiation:");
}

static void TestInvalidClaronTrackingDevice()
{
  MITK_TEST_OUTPUT(<<"Testing simple claron tracking device with 2 invalid tools");

  mitk::TrackingDeviceSourceConfigurator::Pointer testInstance;

  mitk::NavigationToolStorage::Pointer claronStorage = mitk::NavigationToolStorage::New();

  //create invalid tool 1
  mitk::NavigationTool::Pointer firstTool = mitk::NavigationTool::New();
  firstTool->SetTrackingDeviceType(mitk::MicronTrackerTypeInformation::GetTrackingDeviceName());
  mitk::DataNode::Pointer firstNode = mitk::DataNode::New();
  firstNode->SetName("Tool1");
  firstTool->SetDataNode(firstNode);
  claronStorage->AddTool(firstTool);

  //create invalid tool 2
  mitk::NavigationTool::Pointer secondTool = mitk::NavigationTool::New();
  secondTool->SetTrackingDeviceType(mitk::MicronTrackerTypeInformation::GetTrackingDeviceName());
  mitk::DataNode::Pointer secondNode = mitk::DataNode::New();
  secondNode->SetName("Tool2");
  secondTool->SetDataNode(secondNode);
  claronStorage->AddTool(secondTool);

  mitk::TrackingDevice::Pointer testDevice = dynamic_cast<mitk::TrackingDevice*>(mitk::ClaronTrackingDevice::New().GetPointer());

  testInstance = mitk::TrackingDeviceSourceConfigurator::New(claronStorage,testDevice);

  mitk::TrackingDeviceSource::Pointer testSource = testInstance->CreateTrackingDeviceSource();

  MITK_TEST_CONDITION_REQUIRED(testSource.IsNull(),"..testing return value");
  MITK_TEST_CONDITION_REQUIRED(testInstance->GetErrorMessage().size()>1,"..testing if there is an error message");

  MITK_TEST_OUTPUT(<<"Testing simple claron tracking device with another 2 invalid tools");

  secondTool->SetTrackingDeviceType(mitk::NDIAuroraTypeInformation::GetTrackingDeviceName());
  claronStorage = mitk::NavigationToolStorage::New();
  claronStorage->AddTool(secondTool);

  testInstance = mitk::TrackingDeviceSourceConfigurator::New(claronStorage,testDevice);

  MITK_TEST_CONDITION_REQUIRED(!testInstance->IsCreateTrackingDeviceSourcePossible(),"..testing if factory class detects the invalid data");

  testSource = testInstance->CreateTrackingDeviceSource();

  MITK_TEST_CONDITION_REQUIRED(testSource.IsNull(),"..testing return value");
  MITK_TEST_CONDITION_REQUIRED(testInstance->GetErrorMessage().size()>1,"..testing if there is an error message");
  MITK_TEST_CONDITION_REQUIRED(testInstance->GetUpdatedNavigationToolStorage()->GetToolCount()==1,"..testing if navigation tool storage is still there");

  MITK_TEST_OUTPUT(<<"Testing other invalid test cases");
  testInstance = mitk::TrackingDeviceSourceConfigurator::New(claronStorage,nullptr);
  MITK_TEST_CONDITION_REQUIRED(!testInstance->IsCreateTrackingDeviceSourcePossible(),"..(1) testing if factory class detects the invalid data");
  testInstance = mitk::TrackingDeviceSourceConfigurator::New(nullptr,testDevice);
  MITK_TEST_CONDITION_REQUIRED(!testInstance->IsCreateTrackingDeviceSourcePossible(),"..(2) testing if factory class detects the invalid data");
}

static void TestValidClaronTrackingDevice()
{
  MITK_TEST_OUTPUT(<<"Testing simple claron tracking device with 2 valid tools");

  std::string toolFileName(MITK_IGT_DATA_DIR);
  toolFileName.append("/ClaronTool");

  mitk::TrackingDeviceSourceConfigurator::Pointer testInstance;
  mitk::NavigationToolStorage::Pointer claronStorage = mitk::NavigationToolStorage::New();

  //create valid tool 1
  mitk::NavigationTool::Pointer firstTool = mitk::NavigationTool::New();
  firstTool->SetTrackingDeviceType(mitk::MicronTrackerTypeInformation::GetTrackingDeviceName());
  mitk::DataNode::Pointer firstNode = mitk::DataNode::New();
  firstNode->SetName("Tool1");
  firstTool->SetDataNode(firstNode);
  firstTool->SetCalibrationFile(toolFileName);
  firstTool->SetIdentifier("Tool#1");
  claronStorage->AddTool(firstTool);

  //create valid tool 2
  mitk::NavigationTool::Pointer secondTool = mitk::NavigationTool::New();
  secondTool->SetTrackingDeviceType(mitk::MicronTrackerTypeInformation::GetTrackingDeviceName());
  mitk::DataNode::Pointer secondNode = mitk::DataNode::New();
  secondNode->SetName("Tool2");
  secondTool->SetDataNode(secondNode);
  secondTool->SetCalibrationFile(toolFileName);
  secondTool->SetIdentifier("Tool#2");
  claronStorage->AddTool(secondTool);

  mitk::TrackingDevice::Pointer testDevice = dynamic_cast<mitk::TrackingDevice*>(mitk::ClaronTrackingDevice::New().GetPointer());

  testInstance = mitk::TrackingDeviceSourceConfigurator::New(claronStorage,testDevice);

  mitk::TrackingDeviceSource::Pointer testSource = testInstance->CreateTrackingDeviceSource();

  MITK_TEST_CONDITION_REQUIRED(testSource->GetNumberOfOutputs()==2,"testing number of outputs");
  MITK_TEST_CONDITION_REQUIRED(testSource->GetTrackingDevice()->GetToolCount()==2,"testing tracking device");
}

static void TestAdditionalMethods()
{
  MITK_TEST_OUTPUT(<<"Testing additional methods of TrackingDeviceSourceCOnfigurator");
  MITK_TEST_OUTPUT(<<"..using claron tracking device for testing");

  std::string toolFileName(MITK_IGT_DATA_DIR);
  toolFileName.append("/ClaronTool");

  mitk::TrackingDeviceSourceConfigurator::Pointer testInstance;

  mitk::NavigationToolStorage::Pointer claronStorage = mitk::NavigationToolStorage::New();

  //create valid tool 1
  mitk::NavigationTool::Pointer firstTool = mitk::NavigationTool::New();
  firstTool->SetTrackingDeviceType(mitk::MicronTrackerTypeInformation::GetTrackingDeviceName());
  mitk::DataNode::Pointer firstNode = mitk::DataNode::New();
  firstNode->SetName("Tool1");
  firstTool->SetDataNode(firstNode);
  firstTool->SetCalibrationFile(toolFileName);
  firstTool->SetIdentifier("Tool#1");
  claronStorage->AddTool(firstTool);

  //create valid tool 2
  mitk::NavigationTool::Pointer secondTool = mitk::NavigationTool::New();
  secondTool->SetTrackingDeviceType(mitk::MicronTrackerTypeInformation::GetTrackingDeviceName());
  mitk::DataNode::Pointer secondNode = mitk::DataNode::New();
  secondNode->SetName("Tool2");
  secondTool->SetDataNode(secondNode);
  secondTool->SetCalibrationFile(toolFileName);
  secondTool->SetIdentifier("Tool#2");
  claronStorage->AddTool(secondTool);

  mitk::TrackingDevice::Pointer testDevice = dynamic_cast<mitk::TrackingDevice*>(mitk::ClaronTrackingDevice::New().GetPointer());

  testInstance = mitk::TrackingDeviceSourceConfigurator::New(claronStorage,testDevice);

  mitk::TrackingDeviceSource::Pointer testSource = testInstance->CreateTrackingDeviceSource();

  //numbers must be the same in case of MicronTracker
  MITK_TEST_CONDITION_REQUIRED(testInstance->GetToolNumberInToolStorage(0)==0,"..testing method GetToolNumberInToolStorage()");
  MITK_TEST_CONDITION_REQUIRED(testInstance->GetToolIdentifierInToolStorage(0)=="Tool#1","..testing method GetToolIdentifierInToolStorage()");
  MITK_TEST_CONDITION_REQUIRED(testInstance->GetToolNumbersInToolStorage().at(0)==0,"..testing method GetToolNumbersInToolStorage()");
  MITK_TEST_CONDITION_REQUIRED(testInstance->GetToolIdentifiersInToolStorage().at(0)=="Tool#1","..testing method GetToolIdentifiersInToolStorage()");


}

int mitkTrackingDeviceSourceConfiguratorTest(int /* argc */, char* /*argv*/[])
{
  mitk::TrackingDeviceTypeCollection deviceTypeCollection;
  deviceTypeCollection.RegisterTrackingDeviceType(new mitk::NDIAuroraTypeInformation());
  deviceTypeCollection.RegisterAsMicroservice();

  deviceTypeCollection.RegisterTrackingDeviceType(new mitk::VirtualTrackerTypeInformation());
  deviceTypeCollection.RegisterTrackingDeviceType(new mitk::NDIPolarisTypeInformation());
  deviceTypeCollection.RegisterTrackingDeviceType(new mitk::MicronTrackerTypeInformation());

  MITK_TEST_BEGIN("TrackingDeviceConfigurator");

  TestInstantiation();
  TestInvalidClaronTrackingDevice();
  TestValidClaronTrackingDevice();
  TestAdditionalMethods();

  MITK_TEST_END();
}
