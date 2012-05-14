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

