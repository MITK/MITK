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
#include "mitkToFCameraMESASR4000Device.h"
#include "mitkToFCameraMESASR4000Controller.h"
#include "mitkRealTimeClock.h"

#include "itkMultiThreader.h"


namespace mitk
{
  ToFCameraMESASR4000Device::ToFCameraMESASR4000Device()
  {
    m_Controller = ToFCameraMESASR4000Controller::New();
  }

  ToFCameraMESASR4000Device::~ToFCameraMESASR4000Device()
  {
  }

  void ToFCameraMESASR4000Device::SetRegionOfInterest(unsigned int leftUpperCornerX, unsigned int leftUpperCornerY, unsigned int width, unsigned int height)
  {
    //if (m_Controller.IsNotNull())
    //{
    //  dynamic_cast<ToFCameraMESASR4000Controller*>(m_Controller.GetPointer())->SetRegionOfInterest(leftUpperCornerX,leftUpperCornerY,width,height);
    //}
  }

  void ToFCameraMESASR4000Device::SetProperty( const char *propertyKey, BaseProperty* propertyValue )
  {
    ToFCameraMESADevice::SetProperty(propertyKey,propertyValue);
    this->m_PropertyList->SetProperty(propertyKey, propertyValue);

    ToFCameraMESASR4000Controller::Pointer myController = dynamic_cast<mitk::ToFCameraMESASR4000Controller*>(this->m_Controller.GetPointer());

    bool boolValue = false;
    GetBoolProperty(propertyKey, boolValue);
    if (strcmp(propertyKey, "SetFPN") == 0)
    {
      myController->SetFPN(boolValue);
    }
    else if (strcmp(propertyKey, "SetConvGray") == 0)
    {
      myController->SetConvGray(boolValue);
    }
    else if (strcmp(propertyKey, "SetMedian") == 0)
    {
      myController->SetMedian(boolValue);
    }
    else if (strcmp(propertyKey, "SetANF") == 0)
    {
      myController->SetANF(boolValue);
    }
  }

}
