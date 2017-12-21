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
        mitkNewMacro3Param(MonteCarloThreadHandler, long, bool, bool)

        long GetNextWorkPackage();

      void SetPackageSize(long sizeInMilliseconsOrNumberOfPhotons);

      itkGetMacro(NumberPhotonsToSimulate, long);
      itkGetMacro(NumberPhotonsRemaining, long);
      itkGetMacro(WorkPackageSize, long);
      itkGetMacro(SimulationTime, long);
      itkGetMacro(SimulateOnTimeBasis, bool);
      itkGetMacro(Verbose, bool);

    protected:
      long m_NumberPhotonsToSimulate;
      long m_NumberPhotonsRemaining;
      long m_WorkPackageSize;
      long m_SimulationTime;
      long m_Time;
      bool m_SimulateOnTimeBasis;
      bool m_Verbose;
      std::mutex m_MutexRemainingPhotonsManipulation;

      /**
       * @brief PhotoacousticThreadhandler
       * @param timInMilliseconsOrNumberofPhotons
       * @param simulateOnTimeBasis
       */
      MonteCarloThreadHandler(long timInMilliseconsOrNumberofPhotons, bool simulateOnTimeBasis);

      /**
      * @brief PhotoacousticThreadhandler
      * @param timInMilliseconsOrNumberofPhotons
      * @param simulateOnTimeBasis
      * @param verbose
      */
      MonteCarloThreadHandler(long timInMilliseconsOrNumberofPhotons, bool simulateOnTimeBasis, bool verbose);
      ~MonteCarloThreadHandler() override;
    };

    /**
    * @brief Equal A function comparing two thread handlers for beeing equal
    *
    * @param rightHandSide A object to be compared
    * @param leftHandSide A object to be compared
    * @param eps tolarence for comparison. You can use mitk::eps in most cases.
    * @param verbose flag indicating if the user wants detailed console output or not.
    * @return true, if all subsequent comparisons are true, false otherwise
    */
    MITKPHOTOACOUSTICSLIB_EXPORT bool Equal(const MonteCarloThreadHandler::Pointer leftHandSide,
      const MonteCarloThreadHandler::Pointer rightHandSide, double eps, bool verbose);
  }
}

#endif // MITKMONTECARLOTHREADHANDLER_H
