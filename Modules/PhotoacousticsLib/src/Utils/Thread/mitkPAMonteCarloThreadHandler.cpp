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

mitk::pa::MonteCarloThreadHandler::MonteCarloThreadHandler(long timInMillisecondsOrNumberofPhotons, bool simulateOnTimeBasis) :
  MonteCarloThreadHandler(timInMillisecondsOrNumberofPhotons, simulateOnTimeBasis, true){}

mitk::pa::MonteCarloThreadHandler::MonteCarloThreadHandler(long timInMillisecondsOrNumberofPhotons, bool simulateOnTimeBasis, bool verbose)
{
  m_Verbose = verbose;
  m_SimulateOnTimeBasis = simulateOnTimeBasis;
  m_WorkPackageSize = 10000L;
  m_SimulationTime = 0;
  m_Time = 0;
  m_NumberPhotonsToSimulate = 0;
  m_NumberPhotonsRemaining = 0;

  if (m_SimulateOnTimeBasis)
  {
    m_SimulationTime = timInMillisecondsOrNumberofPhotons;
    m_Time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
  }
  else
  {
    m_NumberPhotonsToSimulate = timInMillisecondsOrNumberofPhotons;
    m_NumberPhotonsRemaining = timInMillisecondsOrNumberofPhotons;
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
      if (m_Verbose)
      {
        std::cout << "<filter-progress-text progress='" << ((double)(now - m_Time) / m_SimulationTime) << "'></filter-progress-text>" << std::endl;
      }
    }
  }
  else
  {
    m_MutexRemainingPhotonsManipulation.lock();

    if (m_NumberPhotonsRemaining < m_WorkPackageSize)
    {
      workPackageSize = m_NumberPhotonsRemaining;
    }
    else
    {
      workPackageSize = m_WorkPackageSize;
    }

    m_NumberPhotonsRemaining -= workPackageSize;
    m_MutexRemainingPhotonsManipulation.unlock();

    if (m_Verbose)
    {
      std::cout << "<filter-progress-text progress='" << 1.0 - ((double)m_NumberPhotonsRemaining / m_NumberPhotonsToSimulate) << "'></filter-progress-text>" << std::endl;
    }
  }

  return workPackageSize;
}

void mitk::pa::MonteCarloThreadHandler::SetPackageSize(long sizeInMilliseconsOrNumberOfPhotons)
{
  m_WorkPackageSize = sizeInMilliseconsOrNumberOfPhotons;
}

bool mitk::pa::Equal(const MonteCarloThreadHandler::Pointer leftHandSide,
  const MonteCarloThreadHandler::Pointer rightHandSide, double /*eps*/, bool verbose)
{
  if (rightHandSide->GetNumberPhotonsRemaining() != leftHandSide->GetNumberPhotonsRemaining())
  {
    MITK_INFO(verbose) << "Number of Photons remaining wasnt equal: lhs=" <<
      leftHandSide->GetNumberPhotonsRemaining() << " rhs=" << rightHandSide->GetNumberPhotonsRemaining();
    return false;
  }

  if (rightHandSide->GetNumberPhotonsToSimulate() != leftHandSide->GetNumberPhotonsToSimulate())
  {
    MITK_INFO(verbose) << "Number of Photons to simulate wasnt equal: lhs=" <<
      leftHandSide->GetNumberPhotonsToSimulate() << " rhs=" << rightHandSide->GetNumberPhotonsToSimulate();
    return false;
  }

  if (rightHandSide->GetWorkPackageSize() != leftHandSide->GetWorkPackageSize())
  {
    MITK_INFO(verbose) << "WorkPackageSize wasnt equal: lhs=" <<
      leftHandSide->GetWorkPackageSize() << " rhs=" << rightHandSide->GetWorkPackageSize();
    return false;
  }

  if (rightHandSide->GetSimulationTime() != leftHandSide->GetSimulationTime())
  {
    MITK_INFO(verbose) << "Simulationtime wasnt equal: lhs=" <<
      leftHandSide->GetSimulationTime() << " rhs=" << rightHandSide->GetSimulationTime();
    return false;
  }

  if (rightHandSide->GetSimulateOnTimeBasis() != leftHandSide->GetSimulateOnTimeBasis())
  {
    MITK_INFO(verbose) << "simulation on time basis wasnt equal: lhs=" <<
      leftHandSide->GetSimulateOnTimeBasis() << " rhs=" << rightHandSide->GetSimulateOnTimeBasis();
    return false;
  }

  if (rightHandSide->GetVerbose() != leftHandSide->GetVerbose())
  {
    MITK_INFO(verbose) << "Verbose wasnt equal: lhs=" <<
      leftHandSide->GetVerbose() << " rhs=" << rightHandSide->GetVerbose();
    return false;
  }

  return true;
}
