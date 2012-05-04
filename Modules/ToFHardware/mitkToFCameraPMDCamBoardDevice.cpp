/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "mitkToFCameraPMDCamBoardDevice.h"
#include "mitkToFCameraPMDCamBoardController.h"
#include "mitkRealTimeClock.h"

#include "itkMultiThreader.h"


namespace mitk
{
  ToFCameraPMDCamBoardDevice::ToFCameraPMDCamBoardDevice()
  {
    m_Controller = ToFCameraPMDCamBoardController::New();
  }

  ToFCameraPMDCamBoardDevice::~ToFCameraPMDCamBoardDevice()
  {
  }
  
  void ToFCameraPMDCamBoardDevice::SetRegionOfInterest(unsigned int leftUpperCornerX, unsigned int leftUpperCornerY, unsigned int width, unsigned int height)
  {
    if (m_Controller.IsNotNull())
    {
      dynamic_cast<ToFCameraPMDCamBoardController*>(m_Controller.GetPointer())->SetRegionOfInterest(leftUpperCornerX,leftUpperCornerY,width,height);
    }
  }

  void ToFCameraPMDCamBoardDevice::SetProperty( const char *propertyKey, BaseProperty* propertyValue )
  {
    this->m_PropertyList->SetProperty(propertyKey, propertyValue);

    ToFCameraPMDCamBoardController::Pointer myController = dynamic_cast<mitk::ToFCameraPMDCamBoardController*>(this->m_Controller.GetPointer());

    bool boolValue = false;
    GetBoolProperty(propertyKey, boolValue);
    if (strcmp(propertyKey, "SetFPNCalibration") == 0)
    {
      myController->SetFPNCalibration(boolValue);
    }
    else if (strcmp(propertyKey, "SetFPPNCalibration") == 0)
    {
      myController->SetFPPNCalibration(boolValue);
    }
    else if (strcmp(propertyKey, "SetLinearityCalibration") == 0)
    {
      myController->SetLinearityCalibration(boolValue);
    }
    else if (strcmp(propertyKey, "SetLensCalibration") == 0)
    {
      myController->SetLensCalibration(boolValue);
    }
    else if (strcmp(propertyKey, "SetExposureMode") == 0)
    {
      if (boolValue)
      {
        myController->SetExposureMode(1);
      }
      else
      {
        myController->SetExposureMode(0);
      }
    }
  }

}
