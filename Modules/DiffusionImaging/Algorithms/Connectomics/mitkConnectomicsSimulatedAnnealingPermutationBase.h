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

#ifndef mitkConnectomicsSimulatedAnnealingPermutationBase_h
#define mitkConnectomicsSimulatedAnnealingPermutationBase_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkMacro.h>

#include "mitkCommon.h"

#include "MitkDiffusionImagingExports.h"

#include "mitkConnectomicsSimulatedAnnealingCostFunctionBase.h"

namespace mitk
{

    /**
  * \brief Base class of a permutation to be used in simulated annealing */
  class MitkDiffusionImaging_EXPORT ConnectomicsSimulatedAnnealingPermutationBase : public itk::Object
  {
  public:

    /** Standard class typedefs. */
    /** Method for creation through the object factory. */

    mitkClassMacro(ConnectomicsSimulatedAnnealingPermutationBase, itk::Object);
    itkNewMacro(Self);

    // Set the cost function
    void SetCostFunction( mitk::ConnectomicsSimulatedAnnealingCostFunctionBase::Pointer costFunction );

    // Returns true if a cost function is assigned
    bool HasCostFunction( );

    // Initialize the permutation
    virtual void Initialize(){};

    // Do a permutation for a specific temperature
    virtual void Permutate( double temperature ){};

    // Do clean up necessary after a permutation
    virtual void CleanUp(){};

  protected:

    //////////////////// Functions ///////////////////////
    ConnectomicsSimulatedAnnealingPermutationBase();
    ~ConnectomicsSimulatedAnnealingPermutationBase();

    /////////////////////// Variables ////////////////////////
    // The cost function assigned to the permutation
    mitk::ConnectomicsSimulatedAnnealingCostFunctionBase::Pointer m_CostFunction;

  };

}// end namespace mitk

#endif // mitkConnectomicsSimulatedAnnealingPermutationBase_h
