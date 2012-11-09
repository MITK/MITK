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

#include "mitkConnectomicsSimulatedAnnealingManager.h"

//for random number generation
#include "vxl/core/vnl/vnl_random.h"
#include "vxl/core/vnl/vnl_math.h"

mitk::ConnectomicsSimulatedAnnealingManager::ConnectomicsSimulatedAnnealingManager()
: m_Permutation( 0 )
{
}

mitk::ConnectomicsSimulatedAnnealingManager::~ConnectomicsSimulatedAnnealingManager()
{
}


bool mitk::ConnectomicsSimulatedAnnealingManager::AcceptChange( double costBefore, double costAfter, double temperature )
{
  if( costAfter <= costBefore )
  {// if cost is lower after
    return true;
  }

  //the random number generators
  vnl_random rng( (unsigned int) rand() );

  //randomly generate threshold
  const double threshold = rng.drand64( 0.0 , 1.0);

  //the likelihood of acceptance
  double likelihood = std::exp( - ( costAfter - costBefore ) / temperature );

  if( threshold < likelihood )
  {
    return true;
  }

  return false;
}

void mitk::ConnectomicsSimulatedAnnealingManager::SetPermutation( mitk::ConnectomicsSimulatedAnnealingPermutationBase::Pointer permutation )
{
  m_Permutation = permutation;
}

void mitk::ConnectomicsSimulatedAnnealingManager::RunSimulatedAnnealing(
  double temperature,
  double stepSize
  )
{
  if( m_Permutation.IsNull() )
  {
    MBI_ERROR << "Trying to run simulated annealing on empty permutation.";
    return;
  }

  if( !m_Permutation->HasCostFunction() )
  {
    MBI_ERROR << "Trying to run simulated annealing on empty cost function.";
    return;
  }

  // Initialize the associated permutation
  m_Permutation->Initialize();


  for( double currentTemperature( temperature );
    currentTemperature > 0.00001;
    currentTemperature = currentTemperature / stepSize )
  {
    // Run Permutations at the current temperature
    m_Permutation->Permutate( currentTemperature );

  }

  // Clean up result
  m_Permutation->CleanUp();

}
