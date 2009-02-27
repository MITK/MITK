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

#include "mitkTrackingVolume.h"
#include "mitkCommon.h"
#include "mitkTestingMacros.h"
#include "vtkPolyData.h"
#include <iostream>

class mitkTrackingVolumeTestClass 
  { 
  public:

    static void TestInstantiation()
    {
    // let's create an object of our class  
    mitk::TrackingVolume::Pointer myTrackingVolume = mitk::TrackingVolume::New();
    MITK_TEST_CONDITION_REQUIRED(myTrackingVolume.IsNotNull(),"Testing instantiation")
    }

    static void TestClaronTrackingVolume()
    {
    MITK_TEST_OUTPUT(<< "---- Testing MicronTracker 2 Tracking Volume ----")
    mitk::TrackingVolume::Pointer myTrackingVolume = mitk::TrackingVolume::New();
    MITK_TEST_CONDITION(myTrackingVolume->SetTrackingDeviceType(mitk::ClaronMicron),"loading MicronTracker Volume data:")
    }

    static void TestNDIAuroraTrackingVolume()
    {
    MITK_TEST_OUTPUT(<< "---- Testing NDI Aurora Tracking Volume ----")
    mitk::TrackingVolume::Pointer myTrackingVolume = mitk::TrackingVolume::New();
    MITK_TEST_CONDITION(myTrackingVolume->SetTrackingDeviceType(mitk::NDIAurora),"loading Aurora Volume data:")
    }

    static void TestNDIPolarisTrackingVolume()
    {
    MITK_TEST_OUTPUT(<< "---- Testing NDI Polaris Tracking Volume ----")
    mitk::TrackingVolume::Pointer myTrackingVolume = mitk::TrackingVolume::New();
    MITK_TEST_CONDITION(myTrackingVolume->SetTrackingDeviceType(mitk::NDIPolaris),"loading Polaris Volume data:")
    }

    static void TestIntuitiveDaVinciTrackingVolume()
    {
    MITK_TEST_OUTPUT(<< "---- Testing Intuitive Da Vinci Tracking Volume ----")
    mitk::TrackingVolume::Pointer myTrackingVolume = mitk::TrackingVolume::New();
    MITK_TEST_CONDITION(myTrackingVolume->SetTrackingDeviceType(mitk::IntuitiveDaVinci),"loading Da Vinci Volume data:")
    }

    static void TestIsInside()
    {
    MITK_TEST_OUTPUT(<< "---- Testing IsInside-Method ----")
    mitk::TrackingVolume::Pointer myTrackingVolume = mitk::TrackingVolume::New();
    /* The isInside() method is not implemented so far. As soon as this is done, we could load the different Trackingvolumens (Polaris,
     *  MicronTracker, etc) and test different points inside and outside in this method. 
     */
    mitk::Point3D p1;
    mitk::FillVector3D(p1,(float)0,(float)0,(float)0);
    MITK_TEST_CONDITION(myTrackingVolume->IsInside(p1)==false,"... successfull")
    }

    static void TestManualVolume()
    {
    MITK_TEST_OUTPUT(<< "---- Testing Manual Volume definition ----")
    mitk::TrackingVolume::Pointer myTrackingVolume = mitk::TrackingVolume::New();
    try
      {
      vtkPolyData* myPolyData = vtkPolyData::New();
      myTrackingVolume->SetManualVolume(myPolyData);
      }
    catch(...)
      {
      MITK_TEST_FAILED_MSG(<< "Failed loading manual volume!");
      }
    
    }

  };

/** This function is testing the TrackingVolume class. */
int mitkTrackingVolumeTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("TrackingVolume")

  mitkTrackingVolumeTestClass::TestInstantiation();
  mitkTrackingVolumeTestClass::TestClaronTrackingVolume();
  mitkTrackingVolumeTestClass::TestNDIAuroraTrackingVolume();
  mitkTrackingVolumeTestClass::TestNDIPolarisTrackingVolume();
  mitkTrackingVolumeTestClass::TestIntuitiveDaVinciTrackingVolume();
  mitkTrackingVolumeTestClass::TestIsInside();
  mitkTrackingVolumeTestClass::TestManualVolume();
    
  MITK_TEST_END() 
}



