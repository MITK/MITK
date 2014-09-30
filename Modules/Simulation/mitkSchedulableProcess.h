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

#ifndef mitkSchedulableProcess_h
#define mitkSchedulableProcess_h

#include <boost/chrono.hpp>
#include <MitkSimulationExports.h>

namespace mitk
{
  class MitkSimulation_EXPORT SchedulableProcess
  {
  public:
    explicit SchedulableProcess(int priority = 0);
    virtual ~SchedulableProcess();

    int GetPriority() const;
    boost::chrono::nanoseconds GetTotalElapsedTime() const;
    void ResetTotalElapsedTime(boost::chrono::nanoseconds carryover = boost::chrono::nanoseconds::zero());
    boost::chrono::nanoseconds GetElapsedTime() const;

  protected:
    void SetElapsedTime(boost::chrono::nanoseconds elapsedTime);

  private:
    SchedulableProcess(const SchedulableProcess&);
    SchedulableProcess& operator=(const SchedulableProcess&);

    int m_Priority;
    boost::chrono::nanoseconds m_TotalElapsedTime;
    boost::chrono::nanoseconds m_ElapsedTime;
  };
}

#endif
