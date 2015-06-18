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
#include <mitkTestFixture.h>
#include "mitkAbstractToFDeviceFactory.h"

class mitkAbstractToFDeviceFactoryTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkAbstractToFDeviceFactoryTestSuite);
  MITK_TEST(GetIntrinsics_DefaultFactory_ExpectedDefaultCalibrationParamters);
  CPPUNIT_TEST_SUITE_END();

private:

  /**
   * @brief The DummyFactory struct inherits from AbstractToFDeviceFactory
   * and overwrites all neccessary methods. Unfortunately, only GetCameraIntrinsics
   * can be testet without actually offering a device. The rest of the methods
   * is tested in the ToFCameraMITKPlayerDeviceTest and the
   * ToFCameraMITKPlayerDeviceFactoryTest
   */
  struct DummyFactory : public mitk::AbstractToFDeviceFactory {

    virtual std::string GetFactoryName() override{return "dummy factory";}

    virtual std::string GetDeviceNamePrefix() override{return "dummy device";}

    virtual std::string GetCurrentDeviceName() override{return "dummy device";}

    virtual mitk::ToFCameraDevice::Pointer CreateToFCameraDevice() override {return NULL;}

    /**
     * @brief GetIntrinsics Public wrapper to test intrinsics
     * @return Default camera intrinsic parameters.
     */
    mitk::CameraIntrinsics::Pointer GetIntrinsics()
    {
      return GetCameraIntrinsics();
    }
  };// End of internal dummy factory

public:

  void GetIntrinsics_DefaultFactory_ExpectedDefaultCalibrationParamters()
  {
    DummyFactory dummyFactory;
    mitk::CameraIntrinsics::Pointer result = dummyFactory.GetIntrinsics();
    mitk::CameraIntrinsics::Pointer expected = mitk::CameraIntrinsics::New();
    //Load default calibration parameters from MITK-Data
    expected->FromXMLFile(GetTestDataFilePath("ToF-Data/CalibrationFiles/Default_Parameters.xml"));

    //There is not Equal for mitk::CameraIntrinsics, thus we compare some paramters.
    CPPUNIT_ASSERT(mitk::Equal(result->GetFocalLengthX(), expected->GetFocalLengthX()));
    CPPUNIT_ASSERT(mitk::Equal(result->GetFocalLengthY(), expected->GetFocalLengthY()));

    CPPUNIT_ASSERT(mitk::Equal(result->GetPrincipalPointX(), expected->GetPrincipalPointX()));
    CPPUNIT_ASSERT(mitk::Equal(result->GetPrincipalPointY(), expected->GetPrincipalPointY()));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkAbstractToFDeviceFactory)
