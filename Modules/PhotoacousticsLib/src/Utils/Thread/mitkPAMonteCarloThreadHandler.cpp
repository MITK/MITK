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

#include "mitkPAMonteCarloThreadHandler.h"
#include "mitkCommon.h"

mitk::pa::MonteCarloThreadHandler::MonteCarloThreadHandler(long timInMilliseconsOrNumberofPhotons, bool simulateOnTimeBasis)
{
  m_SimulateOnTimeBasis = simulateOnTimeBasis;
  if (m_SimulateOnTimeBasis)
  {
    m_SimulationTime = timInMilliseconsOrNumberofPhotons;
    m_Time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    m_WorkPackageSize = 10000L;
  }
  else
  {
    m_NumberPhotonsToSimulate = timInMilliseconsOrNumberofPhotons;
    m_NumberPhotonsRemaining = timInMilliseconsOrNumberofPhotons;
    m_WorkPackageSize = 10000L;
  }
}

mitk::pa::MonteCarloThreadHandler::~MonteCarloThreadHandler()
{
}

long mitk::pa::MonteCarloThreadHandler::GetNextWorkPackage()
{
  long workPackageSize = 0;
  if (m_SimulateOnTimeBasis)
  {
    long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    if (now - m_Time <= m_SimulationTime)
    {
      workPackageSize = m_WorkPackageSize;
      std::cout << "<filter-progress-text progress='" << ((double)(now - m_Time) / m_SimulationTime) << "'></filter-progress-text>" << std::endl;
    }
  }
  else
  {
    m_MutexRemainingPhotonsManipulation.lock();

    if (m_NumberPhotonsRemaining < m_WorkPackageSize)
      workPackageSize = m_NumberPhotonsRemaining;
    else
      workPackageSize = m_WorkPackageSize;

    m_NumberPhotonsRemaining -= workPackageSize;
    m_MutexRemainingPhotonsManipulation.unlock();

    std::cout << "<filter-progress-text progress='" << 1.0 - ((double)m_NumberPhotonsRemaining / m_NumberPhotonsToSimulate) << "'></filter-progress-text>" << std::endl;
  }

  return workPackageSize;
}

void mitk::pa::MonteCarloThreadHandler::SetPackageSize(long sizeInMilliseconsOrNumberOfPhotons)
{
  m_WorkPackageSize = sizeInMilliseconsOrNumberOfPhotons;
}
