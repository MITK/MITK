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
// mitk includes
#include <mitkTestingMacros.h>
#include <mitkToFCameraPMDController.h>
#include <mitkCommon.h>
#include <mitkToFConfig.h>

// itk includes
#include "itkObject.h"
#include "itkObjectFactory.h"

/**Documentation
 *  test for the class "ToFCameraPMDController".
 */
//creating a concrete test implementation of the abstract class
namespace mitk
{
/**
   * @brief The ToFCameraPMDControllerTest class
   * Special class to test the abstract class ToFCameraPMDController.
   */
  class ToFCameraPMDControllerTest : public ToFCameraPMDController
  {
  public:
    mitkClassMacro( ToFCameraPMDControllerTest, ToFCameraPMDController);
    itkNewMacro( ToFCameraPMDControllerTest );

    inline bool OpenCameraConnection(){return true;};

  protected:
    inline ToFCameraPMDControllerTest(){};
    inline ~ToFCameraPMDControllerTest(){};

  private:

  };
} // end namespace mitk

int mitkToFCameraPMDControllerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDController");

  // initialize test
  mitk::ToFCameraPMDControllerTest::Pointer testObject = mitk::ToFCameraPMDControllerTest::New();
  try
  {

  MITK_TEST_CONDITION_REQUIRED(!(testObject.GetPointer() == NULL) ,"Testing initialization class");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureHeight()== 200 ,"Testing initialization of CaptureHeight");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureWidth()== 200 ,"Testing initialization of CaptureWidth");
  int numberOfPixels = testObject->GetCaptureHeight()*testObject->GetCaptureWidth();
  float* dataArray = new float[numberOfPixels];
  char* sourceArray = new char[numberOfPixels];
  short* shortSource = new short[numberOfPixels];
  MITK_TEST_CONDITION_REQUIRED(testObject->OpenCameraConnection(),"Testing OpenCameraConnection()");
  MITK_TEST_CONDITION_REQUIRED(testObject->UpdateCamera(),"Testing UpdateCamera() with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetAmplitudes(dataArray),"Testing GetAmplitudes(float*) with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetAmplitudes(sourceArray,dataArray),"Testing GetAmplitudes(char*,float*) with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetIntensities(dataArray),"Testing GetIntensities(float*) with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetIntensities(sourceArray,dataArray),"Testing GetIntensities(char*,float*) with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetDistances(dataArray),"Testing GetDistances(float*) with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetDistances(sourceArray,dataArray),"Testing GetDistances(char*,float*) with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetSourceData(sourceArray),"Testing GetSourceData(char*) with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetShortSourceData(shortSource), "Testing GetShortSourceData(short*) with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->CloseCameraConnection(),"Testing closing of connection");
  delete [] dataArray;
  delete [] sourceArray;
  delete [] shortSource;
  }
  catch(std::exception &e)
  {
      MITK_INFO << e.what();
  }
  MITK_TEST_END();
}
