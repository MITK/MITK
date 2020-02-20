/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
