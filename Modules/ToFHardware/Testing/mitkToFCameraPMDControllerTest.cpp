/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-12 14:05:50 +0100 (Fr, 12 Mrz 2010) $
Version:   $Revision: 16010 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
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
  MITK_TEST_CONDITION_REQUIRED(!(testObject.GetPointer() == NULL) ,"Testing initialization class");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureHeight()== 200 ,"Testing initialization of CaptureHeight");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureWidth()== 200 ,"Testing initialization of CaptureWidth"); 
  int numberOfPixels = testObject->GetCaptureHeight()*testObject->GetCaptureWidth();
  float* dataArray = new float[numberOfPixels];
  char* sourceArray = new char[numberOfPixels];
  MITK_TEST_CONDITION_REQUIRED(testObject->OpenCameraConnection(),"Testing OpenCameraConnection()");
  MITK_TEST_CONDITION_REQUIRED(testObject->UpdateCamera(),"Testing UpdateCamera() with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetAmplitudes(dataArray),"Testing GetAmplitudes(float*) with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetAmplitudes(sourceArray,dataArray),"Testing GetAmplitudes(char*,float*) with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetIntensities(dataArray),"Testing GetIntensities(float*) with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetIntensities(sourceArray,dataArray),"Testing GetIntensities(char*,float*) with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetDistances(dataArray),"Testing GetDistances(float*) with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetDistances(sourceArray,dataArray),"Testing GetDistances(char*,float*) with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetSourceData(sourceArray),"Testing GetSourceData(char*) with no camera connected");
  MITK_TEST_CONDITION_REQUIRED(testObject->CloseCameraConnection(),"Testing closing of connection");
  delete [] dataArray;
  delete [] sourceArray;
  MITK_TEST_END();
}


