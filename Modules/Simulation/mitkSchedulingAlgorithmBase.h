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

#ifndef mitkSchedulingAlgorithmBase_h
#define mitkSchedulingAlgorithmBase_h

#include <vector>

namespace mitk
{
  class SchedulableProcess;

  class SchedulingAlgorithmBase
  {
  public:
    SchedulingAlgorithmBase();
    virtual ~SchedulingAlgorithmBase();

    virtual SchedulableProcess* GetNextProcess(std::vector<SchedulableProcess*>& processQueue) = 0;

  private:
    SchedulingAlgorithmBase(const SchedulingAlgorithmBase&);
    SchedulingAlgorithmBase& operator=(const SchedulingAlgorithmBase&);
  };
}

#endif
