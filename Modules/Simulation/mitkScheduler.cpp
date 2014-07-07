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

#include "mitkRoundRobinSchedulingAlgorithm.h"
#include "mitkSchedulableProcess.h"
#include "mitkScheduler.h"
#include "mitkWeightedRoundRobinSchedulingAlgorithm.h"
#include <algorithm>
#include <cassert>
#include <vector>

struct mitk::Scheduler::Impl
{
  std::vector<SchedulableProcess*> processQueue;
  SchedulingAlgorithmBase* algorithm;
};

mitk::Scheduler::Scheduler(SchedulingAlgorithm::Enum algorithm)
  : m_Impl(new Impl)
{
  switch (algorithm)
  {
  case mitk::SchedulingAlgorithm::RoundRobin:
    m_Impl->algorithm = new mitk::RoundRobinSchedulingAlgorithm;
    break;

  case mitk::SchedulingAlgorithm::WeightedRoundRobin:
    m_Impl->algorithm = new mitk::WeightedRoundRobinSchedulingAlgorithm;
    break;

  default:
    assert(false && "Unknown scheduling algorithm!");
  }
}

mitk::Scheduler::~Scheduler()
{
  delete m_Impl->algorithm;
  delete m_Impl;
}

void mitk::Scheduler::AddProcess(SchedulableProcess* process)
{
  if (process == NULL)
    return;

  if (std::find(m_Impl->processQueue.begin(), m_Impl->processQueue.end(), process) == m_Impl->processQueue.end())
    m_Impl->processQueue.push_back(process);
}

void mitk::Scheduler::RemoveProcess(SchedulableProcess* process)
{
  if (process == NULL)
    return;

  std::vector<SchedulableProcess*>::iterator it = std::find(m_Impl->processQueue.begin(), m_Impl->processQueue.end(), process);

  if (it != m_Impl->processQueue.end())
    m_Impl->processQueue.erase(it);
}

bool mitk::Scheduler::IsEmpty() const
{
  return m_Impl->processQueue.empty();
}

mitk::SchedulableProcess* mitk::Scheduler::GetCurrentProcess()
{
  return !m_Impl->processQueue.empty()
    ? m_Impl->processQueue[0]
    : NULL;
}

mitk::SchedulableProcess* mitk::Scheduler::GetNextProcess()
{
  return m_Impl->algorithm->GetNextProcess(m_Impl->processQueue);
}
