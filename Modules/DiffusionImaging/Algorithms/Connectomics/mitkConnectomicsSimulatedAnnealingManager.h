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

#ifndef mitkConnectomicsSimulatedAnnealingManager_h
#define mitkConnectomicsSimulatedAnnealingManager_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkMacro.h>

#include "mitkCommon.h"

#include "MitkDiffusionImagingExports.h"

#include "mitkConnectomicsSimulatedAnnealingPermutationBase.h"

namespace mitk
{
  /**
  * \brief A class allow generic simulated annealing by using classes derived from ConnectomicsSimulatedAnnealingPermutationBase */
  class MitkDiffusionImaging_EXPORT ConnectomicsSimulatedAnnealingManager : public itk::Object
  {
  public:

    /** Standard class typedefs. */
    /** Method for creation through the object factory. */

    mitkClassMacro(ConnectomicsSimulatedAnnealingManager, itk::Object);
    itkNewMacro(Self);

    // Decide whether to accept the change or not
    bool AcceptChange( double costBefore, double costAfter, double temperature );

    // Run the permutations at different temperatures, where t_n = t_n-1 / stepSize
    void RunSimulatedAnnealing( double temperature, double stepSize );

    // Set the permutation to be used
    void SetPermutation( mitk::ConnectomicsSimulatedAnnealingPermutationBase::Pointer permutation );

  protected:

    //////////////////// Functions ///////////////////////
    ConnectomicsSimulatedAnnealingManager();
    ~ConnectomicsSimulatedAnnealingManager();

    /////////////////////// Variables ////////////////////////
    // The permutation assigned to the simulated annealing manager
    mitk::ConnectomicsSimulatedAnnealingPermutationBase::Pointer m_Permutation;

  };

}// end namespace mitk

#endif // mitkConnectomicsSimulatedAnnealingManager_h
