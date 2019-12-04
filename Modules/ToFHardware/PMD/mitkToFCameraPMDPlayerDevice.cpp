/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkToFCameraPMDPlayerDevice.h"
#include "mitkToFCameraPMDPlayerController.h"
#include "itkMultiThreader.h"


namespace mitk
{
  ToFCameraPMDPlayerDevice::ToFCameraPMDPlayerDevice()
  {
    m_Controller = ToFCameraPMDPlayerController::New();
    m_InputFileName = "";
  }

  ToFCameraPMDPlayerDevice::~ToFCameraPMDPlayerDevice()
  {
  }

  void ToFCameraPMDPlayerDevice::SetProperty(const char* propertyKey, BaseProperty* propertyValue)
  {
    this->m_PropertyList->SetProperty(propertyKey, propertyValue);

    ToFCameraPMDPlayerController::Pointer myController = dynamic_cast<mitk::ToFCameraPMDPlayerController*>(this->m_Controller.GetPointer());

    std::string strValue;
    GetStringProperty(propertyKey, strValue);
    if (strcmp(propertyKey, "PMDFileName") == 0)
    {
      myController->SetPMDFileName(strValue);
    }
  }
}
