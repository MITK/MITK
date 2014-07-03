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
#include <algorithm>
#include <cassert>

mitk::Scheduler::Scheduler(SchedulingAlgorithm::Enum algorithm)
{
  switch (algorithm)
  {
  case mitk::SchedulingAlgorithm::RoundRobin:
    m_Algorithm = new mitk::RoundRobinSchedulingAlgorithm(&m_ProcessQueue);

  default:
    assert(false);
  }
}

mitk::Scheduler::~Scheduler()
{
  delete m_Algorithm;
}

void mitk::Scheduler::AddProcess(SchedulableProcess* process)
{
  if (process == NULL)
    return;

  if (std::find(m_ProcessQueue.begin(), m_ProcessQueue.end(), process) == m_ProcessQueue.end())
    m_ProcessQueue.push_back(process);
}

void mitk::Scheduler::RemoveProcess(SchedulableProcess* process)
{
  if (process == NULL)
    return;

  std::vector<SchedulableProcess*>::iterator it = std::find(m_ProcessQueue.begin(), m_ProcessQueue.end(), process);

  if (it != m_ProcessQueue.end())
    m_ProcessQueue.erase(it);
}

mitk::SchedulableProcess* mitk::Scheduler::GetNextProcess()
{
  return m_Algorithm->GetNextProcess();
}
