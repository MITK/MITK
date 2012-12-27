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

#include "mitkTestingMacros.h"
#include "mitkTrackingTool.h"
#include "mitkTrackingTypes.h"

#include "mitkCommon.h"

#include <itkObject.h>
#include <itkObjectFactory.h>
/**
* Create new class and derive it from TrackingDevice
*/
class TrackingToolTestClass : public mitk::TrackingTool
{
public:

  mitkClassMacro(TrackingToolTestClass, mitk::TrackingTool);
  itkNewMacro(Self);

  virtual void GetPosition(mitk::Point3D & /*position*/) const {};
  virtual void GetOrientation(mitk::Quaternion&  /*orientation*/) const {};
  virtual void SetToolTip(mitk::Point3D toolTipPosition, mitk::Quaternion orientation) {};
  virtual bool Enable() {return true;}
  virtual bool Disable() {return true;}
  virtual bool IsEnabled() const {return true;}
  virtual bool IsDataValid() const {return true;}
  virtual float GetTrackingError() const {return 0.0;}

};

/**
* This function is testing the Class TrackingDevice. For most tests we would need the MicronTracker hardware, so only a few
* simple tests, which can run without the hardware are implemented yet (2009, January, 23rd). As soon as there is a working
* concept to test the tracking classes which are very close to the hardware on all systems more tests are needed here.
*/
int mitkTrackingToolTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("TrackingTool");

  // Test instantiation of TrackingTool
  TrackingToolTestClass::Pointer trackingToolTestClass = TrackingToolTestClass::New();
  MITK_TEST_CONDITION(trackingToolTestClass.IsNotNull(),"Test instatiation");

  // Test method GetToolName()
  MITK_TEST_CONDITION(!strcmp(trackingToolTestClass->GetToolName(),""),"Tool name should be empty");
  // Test method GetErrorMessage()
  MITK_TEST_CONDITION(!strcmp(trackingToolTestClass->GetErrorMessage(),""),"Error message should be empty");

  MITK_TEST_END();
}
