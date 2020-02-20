/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTrackingVolumeGenerator.h>
#include <mitkClaronTrackingDevice.h>
#include "mitkCommon.h"
#include "mitkTestingMacros.h"
#include "vtkPolyData.h"
#include <iostream>
#include <mitkSurface.h>

#include "mitkNDIAuroraTypeInformation.h"
#include "mitkMicronTrackerTypeInformation.h"
#include "mitkUnspecifiedTrackingTypeInformation.h"

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

class mitkTrackingVolumeGeneratorTestClass
{
public:

  static void TestInstantiation()
  {
    // let's create an object of our class
    mitk::TrackingVolumeGenerator::Pointer myTVGenerator = mitk::TrackingVolumeGenerator::New();
    MITK_TEST_CONDITION_REQUIRED(myTVGenerator.IsNotNull(),"Testing instantiation");

  }

  static void TestTrackingSystemNotSpecified()
  {
    MITK_TEST_OUTPUT(<<"---- Testing Trackingsystem not specified ----");
    mitk::TrackingVolumeGenerator::Pointer myTVGenerator = mitk::TrackingVolumeGenerator::New();
    MITK_TEST_CONDITION((myTVGenerator->GetTrackingDeviceType() == mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName()), "Tracking System not specified:");
    myTVGenerator->Update();
    mitk::Surface::Pointer volume = myTVGenerator->GetOutput();
    MITK_TEST_CONDITION((volume->IsEmptyTimeStep(0) == false),"Output contains data");
  }

  static void TestClaronTrackingVolume()
  {
    MITK_TEST_OUTPUT(<< "---- Testing MicronTracker 2 Tracking Volume ----");
    mitk::TrackingVolumeGenerator::Pointer myTVGenerator = mitk::TrackingVolumeGenerator::New ();
    myTVGenerator->SetTrackingDeviceType(mitk::MicronTrackerTypeInformation::GetTrackingDeviceName());
    MITK_TEST_CONDITION((myTVGenerator->GetTrackingDeviceType() == mitk::MicronTrackerTypeInformation::GetTrackingDeviceName()),"loading MicronTracker Volume data:");
    myTVGenerator->Update();
    mitk::Surface::Pointer volume = myTVGenerator->GetOutput();
    MITK_TEST_CONDITION((volume->IsEmptyTimeStep(0) == false),"Output contains data");
  }

  static void TestNDIAuroraTrackingVolume()
  {
    MITK_TEST_OUTPUT(<< "---- Testing NDI Aurora Tracking Volume ----");
    mitk::TrackingVolumeGenerator::Pointer myTVGenerator = mitk::TrackingVolumeGenerator::New ();
    myTVGenerator->SetTrackingDeviceType(mitk::NDIAuroraTypeInformation::GetTrackingDeviceName());
    MITK_TEST_CONDITION((myTVGenerator->GetTrackingDeviceType() == mitk::NDIAuroraTypeInformation::GetTrackingDeviceName()),"loading Aurora Volume data:");
    myTVGenerator->Update();
    mitk::Surface::Pointer volume = myTVGenerator->GetOutput();
    MITK_TEST_CONDITION((volume->IsEmptyTimeStep(0) == false),"Output contains data");
  }

  static void TestNDIPolarisTrackingVolume()
  {
    MITK_TEST_OUTPUT(<< "---- Testing NDI Polaris Tracking Volume ----");
    mitk::TrackingVolumeGenerator::Pointer myTVGenerator = mitk::TrackingVolumeGenerator::New ();
    myTVGenerator->SetTrackingDeviceType(mitk::NDIPolarisTypeInformation::GetTrackingDeviceName());
    MITK_TEST_CONDITION((myTVGenerator->GetTrackingDeviceType() == mitk::NDIPolarisTypeInformation::GetTrackingDeviceName()), "loading Polaris Volume data:");
    myTVGenerator->Update();
    mitk::Surface::Pointer volume = myTVGenerator->GetOutput();
    MITK_TEST_CONDITION((volume->IsEmptyTimeStep(0) == false),"Output contains data");
  }

  static void TestInvalidInputBehaviour()
  {
    MITK_TEST_OUTPUT(<< "---- Testing Invalid Inputs (errors should occure) ----");
    mitk::TrackingVolumeGenerator::Pointer myTVGenerator = mitk::TrackingVolumeGenerator::New ();
    myTVGenerator->SetTrackingDeviceType(mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName()); //MicroBird not implemented yet, so using as test dummy
    MITK_TEST_CONDITION((myTVGenerator->GetTrackingDeviceType() == mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName()), "testing device type");
    myTVGenerator->Update();
    MITK_TEST_CONDITION(myTVGenerator->GetOutput()->GetVtkPolyData()->GetNumberOfVerts()==0,"testing (invalid) output");
  }

  static void TestSetTrackingDevice()
  {
    MITK_TEST_OUTPUT(<< "---- Testing method SetTrackingDevice() ----");
    mitk::ClaronTrackingDevice::Pointer testTrackingDevice = mitk::ClaronTrackingDevice::New();
    mitk::TrackingVolumeGenerator::Pointer myTVGenerator = mitk::TrackingVolumeGenerator::New ();
    myTVGenerator->SetTrackingDevice(dynamic_cast<mitk::TrackingDevice*>(testTrackingDevice.GetPointer()));
    MITK_TEST_CONDITION((myTVGenerator->GetTrackingDeviceType() == mitk::MicronTrackerTypeInformation::GetTrackingDeviceName()), "testing SetTrackingDevice()");

  }

  static void TestSetTrackingDeviceData()
  {
    MITK_TEST_OUTPUT(<< "---- Testing method SetTrackingDeviceData() ----");
    mitk::TrackingVolumeGenerator::Pointer myTVGenerator = mitk::TrackingVolumeGenerator::New ();
    myTVGenerator->SetTrackingDeviceData(mitk::NDIAuroraTypeInformation::GetDeviceDataAuroraPlanarCube());
    MITK_TEST_CONDITION((myTVGenerator->GetTrackingDeviceType() == mitk::NDIAuroraTypeInformation::GetTrackingDeviceName()), "testing if data was set correctly");
  }

  static void TestGetTrackingDeviceData()
  {
    MITK_TEST_OUTPUT(<< "---- Testing method GetTrackingDeviceData() ----");
    mitk::TrackingVolumeGenerator::Pointer myTVGenerator = mitk::TrackingVolumeGenerator::New ();
    myTVGenerator->SetTrackingDeviceData(mitk::UnspecifiedTrackingTypeInformation::GetDeviceDataInvalid());
    MITK_TEST_CONDITION((myTVGenerator->GetTrackingDeviceType() == mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName()), "setting device to invalid");
    myTVGenerator->SetTrackingDeviceData(mitk::MicronTrackerTypeInformation::GetDeviceDataMicronTrackerH40());
    mitk::TrackingDeviceData testData = myTVGenerator->GetTrackingDeviceData();
    MITK_TEST_CONDITION(( (testData.Line == mitk::MicronTrackerTypeInformation::GetDeviceDataMicronTrackerH40().Line) &&
      (testData.Model == mitk::MicronTrackerTypeInformation::GetDeviceDataMicronTrackerH40().Model) &&
      (testData.VolumeModelLocation == mitk::MicronTrackerTypeInformation::GetDeviceDataMicronTrackerH40().VolumeModelLocation)
      ),"changing device and testing getter");
  }


  /* The isInside() method is not implemented so far. So please activate is as soon as this is done. Then we could load
  * the different Trackingvolumens (Polaris, MicronTracker, etc) and test different points inside and outside in this method.
  static void TestIsInside()
  {
  MITK_TEST_OUTPUT(<< "---- Testing IsInside-Method ----")
  mitk::TrackingVolume::Pointer myTrackingVolume = mitk::TrackingVolume::New();

  mitk::Point3D p1;
  mitk::FillVector3D(p1,(float)0,(float)0,(float)0);
  MITK_TEST_CONDITION(myTrackingVolume->IsInside(p1)==false,"... successfull")
  }
  */


};

/** This function is testing the TrackingVolume class. */
int mitkTrackingVolumeGeneratorTest(int /* argc */, char* /*argv*/[])
{
  mitk::TrackingDeviceTypeCollection deviceTypeCollection;
  deviceTypeCollection.RegisterTrackingDeviceType(new mitk::NDIAuroraTypeInformation());
  deviceTypeCollection.RegisterAsMicroservice();

  deviceTypeCollection.RegisterTrackingDeviceType(new mitk::VirtualTrackerTypeInformation());
  deviceTypeCollection.RegisterTrackingDeviceType(new mitk::NDIPolarisTypeInformation());
  deviceTypeCollection.RegisterTrackingDeviceType(new mitk::MicronTrackerTypeInformation());

  MITK_TEST_BEGIN("TrackingVolumeGenerator");

  mitkTrackingVolumeGeneratorTestClass::TestInstantiation();
  mitkTrackingVolumeGeneratorTestClass::TestTrackingSystemNotSpecified ();
  mitkTrackingVolumeGeneratorTestClass::TestClaronTrackingVolume();
  mitkTrackingVolumeGeneratorTestClass::TestNDIAuroraTrackingVolume();
  mitkTrackingVolumeGeneratorTestClass::TestNDIPolarisTrackingVolume();
  mitkTrackingVolumeGeneratorTestClass::TestInvalidInputBehaviour();
  mitkTrackingVolumeGeneratorTestClass::TestSetTrackingDevice();
  mitkTrackingVolumeGeneratorTestClass::TestSetTrackingDeviceData();
  mitkTrackingVolumeGeneratorTestClass::TestGetTrackingDeviceData();
  //mitkTrackingVolumeTestClass::TestIsInside(); Activate this code when method isInside() is implemented!

  MITK_TEST_END() ;
}



