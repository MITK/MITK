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

#ifndef mitkScheduler_h
#define mitkScheduler_h

#include <MitkSimulationExports.h>

namespace mitk
{
  class SchedulableProcess;
  class SchedulingAlgorithmBase;

  namespace SchedulingAlgorithm
  {
    enum Enum
    {
      RoundRobin,
      WeightedRoundRobin
    };
  }

  class MitkSimulation_EXPORT Scheduler
  {
  public:
    explicit Scheduler(SchedulingAlgorithm::Enum algorithm = SchedulingAlgorithm::RoundRobin);
    ~Scheduler();

    void AddProcess(SchedulableProcess* process);
    void RemoveProcess(SchedulableProcess* process);
    bool IsEmpty() const;
    SchedulableProcess* GetCurrentProcess();
    SchedulableProcess* GetNextProcess();

  private:
    Scheduler(const Scheduler&);
    Scheduler& operator=(const Scheduler&);

    struct Impl;
    Impl* m_Impl;
  };
}

#endif
