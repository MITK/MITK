/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTrackingDevice.h"
#include "mitkTestingMacros.h"
#include "mitkTrackingTool.h"
#include "mitkTrackingTypes.h"

#include "mitkCommon.h"

#include <itkObject.h>
#include <itkObjectFactory.h>
/**
* Create new class and derive it from TrackingDevice
*/
class TrackingDeviceTestClass : public mitk::TrackingDevice
{
public:

  mitkClassMacro(TrackingDeviceTestClass, mitk::TrackingDevice);
  itkNewMacro(Self);

  virtual bool OpenConnection(){return true;};
  virtual bool CloseConnection(){return true;};
  virtual bool StartTracking(){this->SetState(Tracking); this->m_TrackingFinishedMutex->Unlock(); return true;};
  virtual mitk::TrackingTool* GetTool(unsigned int toolNumber) const {return NULL;};
  virtual unsigned int GetToolCount() const {return 1;};
};

/**
* This function is testing the Class TrackingDevice. For most tests we would need the MicronTracker hardware, so only a few
* simple tests, which can run without the hardware are implemented yet (2009, January, 23rd). As soon as there is a working
* concept to test the tracking classes which are very close to the hardware on all systems more tests are needed here.
*/
int mitkTrackingDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("TrackingDevice");

  // Test instantiation of TrackingDevice
  TrackingDeviceTestClass::Pointer trackingDeviceTestClass = TrackingDeviceTestClass::New();
  MITK_TEST_CONDITION(trackingDeviceTestClass.IsNotNull(),"Test instatiation");

  // Test method GetErrorMessage()
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetErrorMessage()==std::string(""),"Error message should be empty");
  
  // Test method GetState()
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetState()==mitk::TrackingDevice::Setup,"Mode should be initialized to SETUP");

  // Test method SetType()
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetType()==mitk::TrackingSystemNotSpecified,"Type should be initialized to 'not specified'");
  trackingDeviceTestClass->SetType( mitk::NDIAurora );
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetType()==mitk::NDIAurora,"Type should be NDIAurora, as it has just been set");
  trackingDeviceTestClass->SetType( mitk::NDIPolaris );
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetType()==mitk::NDIPolaris,"Type should be NDIPolaris, as it has just been set");
  trackingDeviceTestClass->SetType( mitk::ClaronMicron );
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetType()==mitk::ClaronMicron,"Type should be ClaronMicron, as it has just been set");
  trackingDeviceTestClass->SetType( mitk::AscensionMicroBird );
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetType()==mitk::AscensionMicroBird,"Type should be AscensionMicroBird, as it has just been set");
  trackingDeviceTestClass->SetType( mitk::VirtualTracker);
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetType()==mitk::VirtualTrackingDevice,"Type should be VirtualTrackingDevice, as it has just been set");
  trackingDeviceTestClass->SetType( mitk::IntuitiveDaVinci );
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetType()==mitk::IntuitiveDaVinci,"Type should be IntuitiveDaVinci, as it has just been set");

  // Test method StopTracking()
  trackingDeviceTestClass->StartTracking();
  trackingDeviceTestClass->StopTracking();
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetState()== mitk::TrackingDevice::Ready,"Type should be NDIAurora, as it has just been set");

  MITK_TEST_END();
}
