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
