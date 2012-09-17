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
