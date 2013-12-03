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

#include <mitkTestingMacros.h>
#include <mitkKinect2Controller.h>

/**Documentation
 *  test for the class "Kinect2Controller".
 */
int mitkKinect2ControllerTest(int /* argc */, char* /*argv*/[])
{

  MITK_TEST_BEGIN("Kinect2ControllerTest");

  MITK_INFO << "HI";
  try
  {
  mitk::Kinect2Controller::Pointer kinectController = mitk::Kinect2Controller::New();
  MITK_INFO << "connected: " << kinectController->OpenCameraConnection();
  MITK_INFO << "update: " << kinectController->UpdateCamera();
  //try to work with a kinect controller

  }
  catch(std::exception &e)
  {
      MITK_INFO << e.what();
  }

  MITK_TEST_END();

}
