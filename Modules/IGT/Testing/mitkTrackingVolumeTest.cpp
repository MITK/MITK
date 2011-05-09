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

#include <mitkTrackingVolumeGenerator.h>
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
    mitk::TrackingVolumeGenerator::Pointer myTVGenerator = mitk::TrackingVolumeGenerator::New();
    MITK_TEST_CONDITION_REQUIRED(myTVGenerator.IsNotNull(),"Testing instantiation")

    }

    static void TestTrackingSystemNotSpecified()
    {
     MITK_TEST_OUTPUT(<<"---- Testing Trackingsystem not specified ----")
     mitk::TrackingVolumeGenerator::Pointer myTVGenerator = mitk::TrackingVolumeGenerator::New();
     MITK_TEST_CONDITION((myTVGenerator->GetTrackingDeviceType() == mitk::TrackingSystemNotSpecified), "Trackling System not specified:")
    }

    static void TestClaronTrackingVolume()
    {
    MITK_TEST_OUTPUT(<< "---- Testing MicronTracker 2 Tracking Volume ----")
    mitk::TrackingVolumeGenerator::Pointer myTVGenerator = mitk::TrackingVolumeGenerator::New ();
    myTVGenerator->SetTrackingDeviceType(mitk::ClaronMicron);
    MITK_TEST_CONDITION((myTVGenerator->GetTrackingDeviceType() == mitk::ClaronMicron),"loading MicronTracker Volume data:")
    }

    static void TestNDIAuroraTrackingVolume()
    {
    MITK_TEST_OUTPUT(<< "---- Testing NDI Aurora Tracking Volume ----")
    mitk::TrackingVolumeGenerator::Pointer myTVGenerator = mitk::TrackingVolumeGenerator::New ();
    myTVGenerator->SetTrackingDeviceType(mitk::NDIAurora);
    MITK_TEST_CONDITION((myTVGenerator->GetTrackingDeviceType() == mitk::NDIAurora),"loading Aurora Volume data:")
    }

    static void TestNDIPolarisTrackingVolume()
    {
    MITK_TEST_OUTPUT(<< "---- Testing NDI Polaris Tracking Volume ----")
    mitk::TrackingVolumeGenerator::Pointer myTVGenerator = mitk::TrackingVolumeGenerator::New ();
    myTVGenerator->SetTrackingDeviceType(mitk::NDIPolaris);
    MITK_TEST_CONDITION((myTVGenerator->GetTrackingDeviceType() == mitk::NDIPolaris),"loading Polaris Volume data:")
    }

    static void TestIntuitiveDaVinciTrackingVolume()
    {
    MITK_TEST_OUTPUT(<< "---- Testing Intuitive Da Vinci Tracking Volume ----")
    mitk::TrackingVolumeGenerator::Pointer myTVGenerator = mitk::TrackingVolumeGenerator::New ();
    myTVGenerator->SetTrackingDeviceType(mitk::IntuitiveDaVinci);
    MITK_TEST_CONDITION((myTVGenerator->GetTrackingDeviceType() == mitk::IntuitiveDaVinci),"loading Da Vinci Volume data:")
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
int mitkTrackingVolumeTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("TrackingVolume")

  mitkTrackingVolumeTestClass::TestInstantiation();
  mitkTrackingVolumeTestClass::TestTrackingSystemNotSpecified ();
  mitkTrackingVolumeTestClass::TestClaronTrackingVolume();
  mitkTrackingVolumeTestClass::TestNDIAuroraTrackingVolume();
  mitkTrackingVolumeTestClass::TestNDIPolarisTrackingVolume();
  mitkTrackingVolumeTestClass::TestIntuitiveDaVinciTrackingVolume();
  //mitkTrackingVolumeTestClass::TestIsInside(); Activate this code when method isInside() is implemented!

  MITK_TEST_END() 
}



