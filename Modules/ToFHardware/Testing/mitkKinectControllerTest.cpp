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

#include <mitkTestingMacros.h>
#include <mitkKinectController.h>

#include <XnCppWrapper.h>

/**Documentation
 *  test for the class "KinectController".
 */
int mitkKinectControllerTest(int /* argc */, char* /*argv*/[])
{

  MITK_TEST_BEGIN("KinectController");

  //xn::Context context;
  //context.InitFromXmlFile("C:/Temp/SamplesConfig.xml");
  //xn::DepthGenerator depthGenerator;
  //context.FindExistingNode(XN_NODE_TYPE_DEPTH, depthGenerator);
  //xn::DepthMetaData DepthMD;
  //depthGenerator.GetMetaData(DepthMD);
  //context.WaitAndUpdateAll();
  //const XnDepthPixel* DepthData = DepthMD.Data();
  //for (unsigned int i=0; i<640*480; i++)
  //{
  //  MITK_INFO<<DepthData[i];
  //}
  mitk::KinectController::Pointer testObject = mitk::KinectController::New();
  MITK_TEST_CONDITION_REQUIRED(!testObject.GetPointer()==NULL,"Testing initialzation!");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureHeight()== 480 ,"Testing initialization of CaptureHeight");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureWidth()== 640 ,"Testing initialization of CaptureWidth");
  MITK_TEST_CONDITION_REQUIRED(testObject->OpenCameraConnection(),"Testing opening of camera connection!");
  MITK_TEST_CONDITION_REQUIRED(testObject->UpdateCamera(),"Testing UpdateCamera()");
  //testObject->UpdateCamera();
  float* distances = new float[307200];
  testObject->GetDistances(distances);
  MITK_INFO<<distances[1000];
  MITK_INFO<<distances[1234];
  MITK_INFO<<distances[24556];
  MITK_INFO<<distances[31234];
  MITK_INFO<<distances[200344];
  MITK_TEST_CONDITION_REQUIRED(testObject->CloseCameraConnection(),"Testing closing of camera connection!");

  MITK_TEST_END();

}


