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

#include "mitkSchedulableProcess.h"
#include "mitkRoundRobinSchedulingAlgorithm.h"

mitk::RoundRobinSchedulingAlgorithm::RoundRobinSchedulingAlgorithm(std::vector<SchedulableProcess*>* processQueue)
  : SchedulingAlgorithmBase(processQueue)
{
}

mitk::RoundRobinSchedulingAlgorithm::~RoundRobinSchedulingAlgorithm()
{
}

mitk::SchedulableProcess* mitk::RoundRobinSchedulingAlgorithm::GetNextProcess()
{
  if (m_ProcessQueue->size() > 1)
    std::swap(m_ProcessQueue->front(), m_ProcessQueue->back());

  return m_ProcessQueue->back();
}
