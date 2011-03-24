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
#include <mitkToFImageGrabberCreator.h>

#include <mitkToFCameraMITKPlayerDevice.h>
#include <mitkToFCameraPMDCamCubeDevice.h>
#include <mitkToFCameraPMDCamBoardDevice.h>
#include <mitkToFCameraPMDPlayerDevice.h>
#include <mitkToFCameraPMDMITKPlayerDevice.h>
#include <mitkToFImageGrabber.h>

/**Documentation
 *  test for the class "ToFImageGrabberCreatorCreator".
 */
int mitkToFImageGrabberCreatorTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFImageGrabberCreator");

  MITK_TEST_CONDITION_REQUIRED(mitk::ToFImageGrabberCreator::GetInstance()!=NULL,"Test GetInstance()");
  mitk::ToFImageGrabber::Pointer imageGrabber = NULL;
  // ToFCameraMITKPlayer
  imageGrabber = mitk::ToFImageGrabberCreator::GetInstance()->GetMITKPlayerImageGrabber();
  MITK_TEST_CONDITION_REQUIRED(imageGrabber.IsNotNull(),"Test if MITKPlayerImageGrabber could be created");
  mitk::ToFCameraMITKPlayerDevice* mitkPlayerDevice
      = dynamic_cast<mitk::ToFCameraMITKPlayerDevice*>(imageGrabber->GetCameraDevice());
  MITK_TEST_CONDITION_REQUIRED(mitkPlayerDevice!=NULL,"Test if image grabber was initialized with ToFCameraMITKPlayerDevice");
  // ToFCameraPMDCamCube
  imageGrabber = mitk::ToFImageGrabberCreator::GetInstance()->GetPMDCamCubeImageGrabber();
  MITK_TEST_CONDITION_REQUIRED(imageGrabber.IsNotNull(),"Test if MITKPlayerImageGrabber could be created");
  mitk::ToFCameraPMDCamCubeDevice* camCubeDevice
      = dynamic_cast<mitk::ToFCameraPMDCamCubeDevice*>(imageGrabber->GetCameraDevice());
  MITK_TEST_CONDITION_REQUIRED(camCubeDevice!=NULL,"Test if image grabber was initialized with ToFCameraPMDCamCubeDevice");
  // ToFCameraPMDCamBoard
  imageGrabber = mitk::ToFImageGrabberCreator::GetInstance()->GetPMDCamBoardImageGrabber();
  MITK_TEST_CONDITION_REQUIRED(imageGrabber.IsNotNull(),"Test if MITKPlayerImageGrabber could be created");
  mitk::ToFCameraPMDCamBoardDevice* camBoardDevice
      = dynamic_cast<mitk::ToFCameraPMDCamBoardDevice*>(imageGrabber->GetCameraDevice());
  MITK_TEST_CONDITION_REQUIRED(camBoardDevice!=NULL,"Test if image grabber was initialized with ToFCameraPMDCamBoardDevice");
  // ToFCameraPMDPlayer
  imageGrabber = mitk::ToFImageGrabberCreator::GetInstance()->GetPMDPlayerImageGrabber();
  MITK_TEST_CONDITION_REQUIRED(imageGrabber.IsNotNull(),"Test if MITKPlayerImageGrabber could be created");
  mitk::ToFCameraPMDPlayerDevice* pmdPlayerDevice
      = dynamic_cast<mitk::ToFCameraPMDPlayerDevice*>(imageGrabber->GetCameraDevice());
  MITK_TEST_CONDITION_REQUIRED(pmdPlayerDevice!=NULL,"Test if image grabber was initialized with ToFCameraPMDPlayerDevice");
  // ToFCameraPMDMITKPlayer
  imageGrabber = mitk::ToFImageGrabberCreator::GetInstance()->GetPMDMITKPlayerImageGrabber();
  MITK_TEST_CONDITION_REQUIRED(imageGrabber.IsNotNull(),"Test if MITKPlayerImageGrabber could be created");
  mitk::ToFCameraPMDMITKPlayerDevice* pmdMITKPlayerDevice
      = dynamic_cast<mitk::ToFCameraPMDMITKPlayerDevice*>(imageGrabber->GetCameraDevice());
  MITK_TEST_CONDITION_REQUIRED(pmdMITKPlayerDevice!=NULL,"Test if image grabber was initialized with ToFCameraPMDMITKPlayerDevice");

  MITK_TEST_END();
}


