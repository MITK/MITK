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

#ifndef mitkRoundRobinSchedulingAlgorithm_h
#define mitkRoundRobinSchedulingAlgorithm_h

#include "mitkSchedulingAlgorithmBase.h"

namespace mitk
{
  class RoundRobinSchedulingAlgorithm : public SchedulingAlgorithmBase
  {
  public:
    RoundRobinSchedulingAlgorithm();
    ~RoundRobinSchedulingAlgorithm();

    SchedulableProcess* GetNextProcess(std::vector<SchedulableProcess*>& processQueue);
  };
}

#endif
