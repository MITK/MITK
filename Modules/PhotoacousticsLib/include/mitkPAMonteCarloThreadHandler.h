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

#ifndef MITKMONTECARLOTHREADHANDLER_H
#define MITKMONTECARLOTHREADHANDLER_H

#include <mitkCommon.h>
#include <MitkPhotoacousticsLibExports.h>
#include <mutex>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

namespace mitk {
  namespace pa {
    /**
     * @brief The PhotoacousticStatefulObject class
     * Designed for inheritence. Provides a state member variable and convenience methods to check
     * for the state.
     */
    class MITKPHOTOACOUSTICSLIB_EXPORT MonteCarloThreadHandler : public itk::LightObject
    {
    public:

      mitkClassMacroItkParent(MonteCarloThreadHandler, itk::LightObject)
        mitkNewMacro2Param(MonteCarloThreadHandler, long, bool)

        long GetNextWorkPackage();

      void SetPackageSize(long sizeInMilliseconsOrNumberOfPhotons);

    protected:
      long m_NumberPhotonsToSimulate;
      long m_NumberPhotonsRemaining;
      long m_WorkPackageSize;
      long m_SimulationTime;
      long m_Time;
      bool m_SimulateOnTimeBasis;
      std::mutex m_MutexRemainingPhotonsManipulation;

      /**
       * @brief PhotoacousticThreadhandler
       * @param timInMilliseconsOrNumberofPhotons
       * @param simulateOnTimeBasis
       */
      MonteCarloThreadHandler(long timInMilliseconsOrNumberofPhotons, bool simulateOnTimeBasis);
      virtual ~MonteCarloThreadHandler();
    };
  }
}

#endif // MITKMONTECARLOTHREADHANDLER_H
