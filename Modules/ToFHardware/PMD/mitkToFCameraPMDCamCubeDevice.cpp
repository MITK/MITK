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
#include "mitkToFCameraPMDCamCubeDevice.h"
#include "mitkRealTimeClock.h"

#include "itkMultiThreader.h"


namespace mitk
{
  ToFCameraPMDCamCubeDevice::ToFCameraPMDCamCubeDevice()
  {
    m_Controller = ToFCameraPMDCamCubeController::New();
  }

  ToFCameraPMDCamCubeDevice::~ToFCameraPMDCamCubeDevice()
  {
  }

  void ToFCameraPMDCamCubeDevice::SetProperty( const char *propertyKey, BaseProperty* propertyValue )
  {
    ToFCameraPMDDevice::SetProperty(propertyKey,propertyValue);
    this->m_PropertyList->SetProperty(propertyKey, propertyValue);

    ToFCameraPMDCamCubeController::Pointer myController = dynamic_cast<mitk::ToFCameraPMDCamCubeController*>(this->m_Controller.GetPointer());

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
