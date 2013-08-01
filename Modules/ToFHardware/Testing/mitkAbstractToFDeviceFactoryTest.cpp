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
#include <mitkAbstractToFDeviceFactory.h>


struct DummyFactory : public mitk::AbstractToFDeviceFactory {

  virtual std::string GetFactoryName(){return "dummy";}

  virtual std::string GetCurrentDeviceName(){return "dummy";}

  virtual mitk::ToFCameraDevice::Pointer CreateToFCameraDevice() {return NULL;}

  // Public wrapper to test intrinsics
  mitk::CameraIntrinsics::Pointer GetIntrinsics()
  {
    return GetCameraIntrinsics();
  }

}; // End of internal dummy reader


/**Documentation
 *  test for the class "ToFImageWriter".
 */
int mitkAbstractToFDeviceFactoryTest(int  argc , char* argv[])
{
  MITK_TEST_BEGIN("ToFImageWriter");

  MITK_TEST_CONDITION_REQUIRED(argc > 0, "Testing if enough arguments are set.");
    std::string calibrationFilePath(argv[2]);

  //testing initialization of object
  DummyFactory factory;
  mitk::CameraIntrinsics::Pointer result = factory.GetIntrinsics();
  mitk::CameraIntrinsics::Pointer expected = mitk::CameraIntrinsics::New();
  expected->FromXMLFile(calibrationFilePath);

  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(result->GetFocalLengthX(), expected->GetFocalLengthX())  , "Testing correctness of CamerIntrinsics: FocalLengthX");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(result->GetFocalLengthY(), expected->GetFocalLengthY())  , "Testing correctness of CamerIntrinsics: FocalLengthY");

  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(result->GetPrincipalPointX(), expected->GetPrincipalPointX())  , "Testing correctness of CamerIntrinsics: PrincipalPointX");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(result->GetPrincipalPointY(), expected->GetPrincipalPointY())  , "Testing correctness of CamerIntrinsics: PrincipalPointY");


  MITK_TEST_END();
}
