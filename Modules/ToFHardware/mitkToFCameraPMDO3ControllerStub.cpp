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
#include "mitkToFCameraPMDO3Controller.h"
namespace mitk
{
  ToFCameraPMDO3Controller::ToFCameraPMDO3Controller()
  {
    MITK_WARN("ToF") << "Error: PMD O3 camera currently not available.";
  }

  ToFCameraPMDO3Controller::~ToFCameraPMDO3Controller()
  {
  }

  bool ToFCameraPMDO3Controller::OpenCameraConnection()
  {
    MITK_WARN("ToF") << "Error: PMD O3 camera currently not available.";
    return true;
  }
}
