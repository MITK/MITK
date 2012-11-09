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
#include "mitkToFCameraPMDO3Device.h"
#include "mitkToFCameraPMDO3Controller.h"
#include "mitkRealTimeClock.h"

#include "itkMultiThreader.h"
#include <itksys/SystemTools.hxx>


namespace mitk
{
  ToFCameraPMDO3Device::ToFCameraPMDO3Device()
  {
    m_Controller = ToFCameraPMDO3Controller::New();
  }

  ToFCameraPMDO3Device::~ToFCameraPMDO3Device()
  {
  }
}
