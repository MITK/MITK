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
#include "mitkWeightedRoundRobinSchedulingAlgorithm.h"
#include <algorithm>

static bool CompareElapsedTime(mitk::SchedulableProcess* lhs, mitk::SchedulableProcess* rhs)
{
  return lhs->GetElapsedTime() < rhs->GetElapsedTime();
}

mitk::WeightedRoundRobinSchedulingAlgorithm::WeightedRoundRobinSchedulingAlgorithm()
{
}

mitk::WeightedRoundRobinSchedulingAlgorithm::~WeightedRoundRobinSchedulingAlgorithm()
{
}

mitk::SchedulableProcess* mitk::WeightedRoundRobinSchedulingAlgorithm::GetNextProcess(std::vector<SchedulableProcess*>& processQueue)
{
  const double threshold = 1.5;
  size_t numProcesses = processQueue.size();

  if (numProcesses == 0)
    return NULL;

  boost::chrono::nanoseconds maxElapsedTime = (*std::max_element(processQueue.begin(), processQueue.end(), CompareElapsedTime))->GetElapsedTime();
  mitk::SchedulableProcess* process = processQueue[0];

  if (numProcesses > 1)
  {
    boost::chrono::nanoseconds totalElapsedTime = process->GetTotalElapsedTime();
    boost::chrono::nanoseconds elapsedTime = process->GetElapsedTime();

    if (totalElapsedTime >= maxElapsedTime || totalElapsedTime + elapsedTime >= maxElapsedTime * threshold)
    {
      process->ResetTotalElapsedTime();

      processQueue.erase(processQueue.begin());
      processQueue.push_back(process);
    }
  }

  return process;
}
