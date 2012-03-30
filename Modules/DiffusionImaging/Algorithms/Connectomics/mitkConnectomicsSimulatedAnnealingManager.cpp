/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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


//void mitk::ConnectomicsSimulatedAnnealingManager::Testing( mitk::ConnectomicsNetwork::Pointer network )
//{
//  // precursor to an actual test, this will assign a to module map which is only valid for a 
//  // specific network and whose modularity is known
//  ToModuleMapType threeModuleSolution;
//  std::vector< VertexDescriptorType > vertexVector = network->GetVectorOfAllVertexDescriptors();
//
//  threeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 0 ], 0 ) );
//  threeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 1 ], 0 ) );
//  threeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 2 ], 0 ) );
//  threeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 3 ], 0 ) );
//  threeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 4 ], 0 ) );
//  threeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 5 ], 1 ) );
//  threeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 6 ], 1 ) );
//  threeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 7 ], 1 ) );
//  threeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 8 ], 1 ) );
//  threeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 9 ], 2 ) );
//  threeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 10 ], 2 ) );
//  threeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 11 ], 2 ) );
//
//  std::cout << "Modularity is " << CalculateModularity( network, &threeModuleSolution ) << " and should be " << 0.4753 << std::endl;
//
//  std::cout << " Module 0 contains " << getNumberOfVerticesInModule( &threeModuleSolution, 0 ) << " nodes, should be 5.\n";
//  std::cout << " Module 1 contains " << getNumberOfVerticesInModule( &threeModuleSolution, 1 ) << " nodes, should be 4.\n";
//   std::cout << " Module 2 contains " << getNumberOfVerticesInModule( &threeModuleSolution, 2 ) << " nodes, should be 3.\n";
//
//  ToModuleMapType oneModuleSolution;
//  oneModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 0 ], 0 ) );
//  oneModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 1 ], 0 ) );
//  oneModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 2 ], 0 ) );
//  oneModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 3 ], 0 ) );
//  oneModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 4 ], 0 ) );
//  oneModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 5 ], 0 ) );
//  oneModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 6 ], 0 ) );
//  oneModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 7 ], 0 ) );
//  oneModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 8 ], 0 ) );
//  oneModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 9 ], 0 ) );
//  oneModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 10 ], 0 ) );
//  oneModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 11 ], 0 ) );
//
//  std::cout << "Modularity is " << CalculateModularity( network, &oneModuleSolution ) << " and should be " << 0.0 << std::endl;
//
//    std::cout << " Module 0 contains " << getNumberOfVerticesInModule( &oneModuleSolution, 0 ) << " nodes, should be 12.\n";
//
//  ToModuleMapType badTwoModuleSolution;
//  badTwoModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 0 ], 0 ) );
//  badTwoModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 1 ], 0 ) );
//  badTwoModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 2 ], 0 ) );
//  badTwoModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 3 ], 0 ) );
//  badTwoModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 4 ], 0 ) );
//  badTwoModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 5 ], 1 ) );
//  badTwoModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 6 ], 1 ) );
//  badTwoModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 7 ], 1 ) );
//  badTwoModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 8 ], 0 ) );
//  badTwoModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 9 ], 1 ) );
//  badTwoModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 10 ], 1 ) );
//  badTwoModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 11 ], 0 ) );
//
//  std::cout << "Modularity is " << CalculateModularity( network, &badTwoModuleSolution ) << " and should be " << 0.10 << std::endl;
//  std::cout << " Module 0 contains " << getNumberOfVerticesInModule( &badTwoModuleSolution, 0 ) << " nodes, should be 7.\n";
//  std::cout << " Module 1 contains " << getNumberOfVerticesInModule( &badTwoModuleSolution, 1 ) << " nodes, should be 5.\n";
//
//  ToModuleMapType noInternalLinksThreeModuleSolution;
//  noInternalLinksThreeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 0 ], 0 ) );
//  noInternalLinksThreeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 1 ], 2 ) );
//  noInternalLinksThreeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 2 ], 1 ) );
//  noInternalLinksThreeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 3 ], 0 ) );
//  noInternalLinksThreeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 4 ], 1 ) );
//  noInternalLinksThreeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 5 ], 2 ) );
//  noInternalLinksThreeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 6 ], 0 ) );
//  noInternalLinksThreeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 7 ], 0 ) );
//  noInternalLinksThreeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 8 ], 1 ) );
//  noInternalLinksThreeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 9 ], 2 ) );
//  noInternalLinksThreeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 10 ], 1 ) );
//  noInternalLinksThreeModuleSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ 11 ], 0 ) );
//
//  std::cout << "Modularity is " << CalculateModularity( network, &noInternalLinksThreeModuleSolution ) << " and should be " << -0.34 << std::endl;
//
//  std::cout << " Module 0 contains " << getNumberOfVerticesInModule( &noInternalLinksThreeModuleSolution, 0 ) << " nodes, should be 5.\n";
//  std::cout << " Module 1 contains " << getNumberOfVerticesInModule( &noInternalLinksThreeModuleSolution, 1 ) << " nodes, should be 4.\n";
//  std::cout << " Module 2 contains " << getNumberOfVerticesInModule( &noInternalLinksThreeModuleSolution, 2 ) << " nodes, should be 3.\n";
//
//}