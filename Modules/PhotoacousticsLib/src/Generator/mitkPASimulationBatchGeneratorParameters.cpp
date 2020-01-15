/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPASimulationBatchGeneratorParameters.h"

mitk::pa::SimulationBatchGeneratorParameters::SimulationBatchGeneratorParameters()
{
  m_BinaryPath = "";
  m_NrrdFilePath = "";
  m_NumberOfPhotons = 0;
  m_TissueName = "";
  m_VolumeIndex = 0;
  m_YOffsetLowerThresholdInCentimeters = 0;
  m_YOffsetUpperThresholdInCentimeters = 0;
  m_YOffsetStepInCentimeters = 0;
}

mitk::pa::SimulationBatchGeneratorParameters::~SimulationBatchGeneratorParameters()
{
}
