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
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureHeight()== 0 ,"Testing initialization of CaptureHeight");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureWidth()== 0 ,"Testing initialization of CaptureWidth"); 
  MITK_TEST_CONDITION_REQUIRED(testObject->CloseCameraConnection(),"Testing closing of connection");

  MITK_TEST_END();
}


