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

mitk::RoundRobinSchedulingAlgorithm::RoundRobinSchedulingAlgorithm()
{
}

mitk::RoundRobinSchedulingAlgorithm::~RoundRobinSchedulingAlgorithm()
{
}

mitk::SchedulableProcess* mitk::RoundRobinSchedulingAlgorithm::GetNextProcess(std::vector<SchedulableProcess*>& processQueue)
{
  size_t numProcesses = processQueue.size();

  if (numProcesses == 0)
    return NULL;

  mitk::SchedulableProcess* process = processQueue[0];

  if (numProcesses > 1)
  {
    processQueue.erase(processQueue.begin());
    processQueue.push_back(process);
  }

  return process;
}
