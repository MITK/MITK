/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
  }
}
