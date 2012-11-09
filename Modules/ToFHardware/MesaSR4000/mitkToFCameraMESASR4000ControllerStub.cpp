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
#include "mitkToFCameraMESASR4000Controller.h"
#include "mitkToFConfig.h"

namespace mitk
{
  ToFCameraMESASR4000Controller::ToFCameraMESASR4000Controller()
  {
    MITK_WARN("ToF") << "Error: MESA SR4000 currently not available";
  }

  ToFCameraMESASR4000Controller::~ToFCameraMESASR4000Controller()
  {
    MITK_WARN("ToF") << "Error: MESA SR4000 currently not available";
  }

  bool ToFCameraMESASR4000Controller::OpenCameraConnection()
  {
    MITK_WARN("ToF") << "Error: MESA SR4000 currently not available";
    return true;
  }

  int ToFCameraMESASR4000Controller::SetIntegrationTime(unsigned int integrationTime)
  {
    MITK_WARN("ToF") << "Error: MESA SR4000 currently not available";
    return 0;
  }

  int ToFCameraMESASR4000Controller::GetIntegrationTime()
  {
    MITK_WARN("ToF") << "Error: MESA SR4000 currently not available";
    return 0;
  }

  int ToFCameraMESASR4000Controller::SetModulationFrequency(unsigned int modulationFrequency)
  {
    MITK_WARN("ToF") << "Error: MESA SR4000 currently not available";
    return 0;
  }

  int ToFCameraMESASR4000Controller::GetModulationFrequency()
  {
    MITK_WARN("ToF") << "Error: MESA SR4000 currently not available";
    return 0;
  }

  void ToFCameraMESASR4000Controller::SetFPN( bool fpn )
  {
    MITK_WARN("ToF") << "Error: MESA SR4000 currently not available";
  }

  void ToFCameraMESASR4000Controller::SetConvGray( bool ConvGray )
  {
    MITK_WARN("ToF") << "Error: MESA SR4000 currently not available";
  }

  void ToFCameraMESASR4000Controller::SetMedian( bool median )
  {
    MITK_WARN("ToF") << "Error: MESA SR4000 currently not available";
  }

  void ToFCameraMESASR4000Controller::SetANF( bool anf )
  {
    MITK_WARN("ToF") << "Error: MESA SR4000 currently not available";
  }
}
