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

#include "mitkConnectomicsSimulatedAnnealingPermutationModularity.h"
#include "mitkConnectomicsSimulatedAnnealingCostFunctionModularity.h"
#include "mitkConnectomicsSimulatedAnnealingManager.h"

//for random number generation
#include "vxl/core/vnl/vnl_random.h"
#include "vxl/core/vnl/vnl_math.h"

mitk::ConnectomicsSimulatedAnnealingPermutationModularity::ConnectomicsSimulatedAnnealingPermutationModularity()
{
}

mitk::ConnectomicsSimulatedAnnealingPermutationModularity::~ConnectomicsSimulatedAnnealingPermutationModularity()
{
}

void mitk::ConnectomicsSimulatedAnnealingPermutationModularity::SetNetwork(
  mitk::ConnectomicsNetwork::Pointer theNetwork )
{
  m_Network = theNetwork;
}

void mitk::ConnectomicsSimulatedAnnealingPermutationModularity::Initialize()
{
  // create entry for every vertex
  std::vector< VertexDescriptorType > vertexVector = m_Network->GetVectorOfAllVertexDescriptors();
  const int vectorSize = vertexVector.size();

  for( int index( 0 ); index < vectorSize; index++)
  {
    m_BestSolution.insert( std::pair<VertexDescriptorType, int>( vertexVector[ index ], 0 ) );
  }

  // initialize with random distribution of n modules
  int n( 5 );
  randomlyAssignNodesToModules( &m_BestSolution, n );

}

void mitk::ConnectomicsSimulatedAnnealingPermutationModularity::Permutate( double temperature )
{

  ToModuleMapType currentSolution = m_BestSolution;
  ToModuleMapType currentBestSolution = m_BestSolution;

  int factor = 1;
  int numberOfVertices = m_BestSolution.size();
  int singleNodeMaxNumber = factor * numberOfVertices * numberOfVertices;
  int moduleMaxNumber = factor  * numberOfVertices;
  double currentBestCost = Evaluate( &currentBestSolution );

  // do singleNodeMaxNumber node permutations and evaluate
  for(int loop( 0 ); loop < singleNodeMaxNumber; loop++)
  {
    permutateMappingSingleNodeShift( &currentSolution, m_Network );
    if( AcceptChange( currentBestCost, Evaluate( &currentSolution ), temperature ) )
    {
      currentBestSolution = currentSolution;
      currentBestCost = Evaluate( &currentBestSolution );
    }
  }

  // do moduleMaxNumber module permutations
  for(int loop( 0 ); loop < moduleMaxNumber; loop++)
  {
    permutateMappingModuleChange( &currentSolution, temperature, m_Network );
    if( AcceptChange( currentBestCost, Evaluate( &currentSolution ), temperature ) )
    {
      currentBestSolution = currentSolution;
      currentBestCost = Evaluate( &currentBestSolution );
    }
  }

  // store the best solution after the run
  m_BestSolution = currentBestSolution;
}

void mitk::ConnectomicsSimulatedAnnealingPermutationModularity::CleanUp()
{
  // delete empty modules, if any
  for( int loop( 0 ); loop < getNumberOfModules( &m_BestSolution ) ; loop++ )
  {
    if( getNumberOfVerticesInModule( &m_BestSolution, loop ) < 1 )
    {
      removeModule( &m_BestSolution, loop );
    }
  }
}

void mitk::ConnectomicsSimulatedAnnealingPermutationModularity::permutateMappingSingleNodeShift(
  ToModuleMapType *vertexToModuleMap, mitk::ConnectomicsNetwork::Pointer network )
{

  const int nodeCount = vertexToModuleMap->size();
  const int moduleCount = getNumberOfModules( vertexToModuleMap );

  // the random number generators
  vnl_random rng( (unsigned int) rand() );
  unsigned long randomNode = rng.lrand32( nodeCount - 1 );
  // move the node either to any existing module, or to its own
  //unsigned long randomModule = rng.lrand32( moduleCount );
  unsigned long randomModule = rng.lrand32( moduleCount - 1 );

  // do some sanity checks

  if ( nodeCount < 2 )
  {
    // no sense in doing anything
    return;
  }

  const std::vector< VertexDescriptorType > allNodesVector
    = network->GetVectorOfAllVertexDescriptors();

  ToModuleMapType::iterator iter = vertexToModuleMap->find( allNodesVector[ randomNode ] );
  const int previousModuleNumber = iter->second;

  // if we move the node to its own module, do nothing
  if( previousModuleNumber == randomModule )
  {
    return;
  }

  iter->second = randomModule;

  if( getNumberOfVerticesInModule( vertexToModuleMap, previousModuleNumber ) < 1 )
  {
    removeModule( vertexToModuleMap, previousModuleNumber );
  }
}

void mitk::ConnectomicsSimulatedAnnealingPermutationModularity::permutateMappingModuleChange(
  ToModuleMapType *vertexToModuleMap, double currentTemperature, mitk::ConnectomicsNetwork::Pointer network )
{
  //the random number generators
  vnl_random rng( (unsigned int) rand() );

  //randomly generate threshold
  const double threshold = rng.drand64( 0.0 , 1.0);

  //for deciding whether to join two modules or split one
  double splitThreshold = 0.5;

  //stores whether to join or two split
  bool joinModules( false );

  //select random module
  int numberOfModules = getNumberOfModules( vertexToModuleMap );
  unsigned long randomModuleA = rng.lrand32( numberOfModules - 1 );

  //select the second module to join, if joining
  unsigned long randomModuleB = rng.lrand32( numberOfModules - 1 );

  if( ( threshold < splitThreshold ) && ( randomModuleA != randomModuleB )  )
  {
    joinModules = true;
  }

  if( joinModules )
  {
    // this being an kommutative operation, we will always join B to A
    joinTwoModules( vertexToModuleMap, randomModuleA, randomModuleB );
    //eliminate the empty module
    removeModule( vertexToModuleMap, randomModuleB );

  }
  else
  {
    //split module
    splitModule( vertexToModuleMap, currentTemperature, network, randomModuleA );
  }


}

void mitk::ConnectomicsSimulatedAnnealingPermutationModularity::joinTwoModules(
  ToModuleMapType *vertexToModuleMap, int moduleA, int moduleB )
{

  ToModuleMapType::iterator iter = vertexToModuleMap->begin();
  ToModuleMapType::iterator end =  vertexToModuleMap->end();

  while( iter != end )
  {
    // if vertex belongs to module B move it to A
    if( iter->second == moduleB )
    {
      iter->second = moduleA;
    }

    iter++;
  }// end while( iter != end )
}

void mitk::ConnectomicsSimulatedAnnealingPermutationModularity::splitModule(
  ToModuleMapType *vertexToModuleMap, double currentTemperature, mitk::ConnectomicsNetwork::Pointer network, int moduleToSplit )
{

  if( m_Depth == 0 )
  {
    // do nothing
    return;
  }

  // if the module contains only one node, no more division is sensible
  if( getNumberOfVerticesInModule( vertexToModuleMap, moduleToSplit ) < 2 )
  {
    // do nothing
    return;
  }

  // create subgraph of the module, that is to be splitted

  mitk::ConnectomicsNetwork::Pointer subNetwork = mitk::ConnectomicsNetwork::New();
  VertexToVertexMapType graphToSubgraphVertexMap;
  VertexToVertexMapType subgraphToGraphVertexMap;


  extractModuleSubgraph( vertexToModuleMap, network, moduleToSplit, subNetwork, &graphToSubgraphVertexMap, &subgraphToGraphVertexMap );


  // The submap
  ToModuleMapType vertexToModuleSubMap;

  // copy vertices
  VertexToVertexMapType::iterator iter = graphToSubgraphVertexMap.begin();
  VertexToVertexMapType::iterator end =  graphToSubgraphVertexMap.end();

  // run simulated annealing on the subgraph to determine how the module should be split
  if( m_Depth > 0 && m_StepSize > 0 )
  {
    mitk::ConnectomicsSimulatedAnnealingManager::Pointer manager = mitk::ConnectomicsSimulatedAnnealingManager::New();
    mitk::ConnectomicsSimulatedAnnealingPermutationModularity::Pointer permutation = mitk::ConnectomicsSimulatedAnnealingPermutationModularity::New();
    mitk::ConnectomicsSimulatedAnnealingCostFunctionModularity::Pointer costFunction = mitk::ConnectomicsSimulatedAnnealingCostFunctionModularity::New();

    permutation->SetCostFunction( costFunction.GetPointer() );
    permutation->SetNetwork( subNetwork );
    permutation->SetDepth( m_Depth - 1 );
    permutation->SetStepSize( m_StepSize * 2 );

    manager->SetPermutation( permutation.GetPointer() );

    manager->RunSimulatedAnnealing( currentTemperature, m_StepSize * 2 );

    vertexToModuleSubMap = permutation->GetMapping();
  }

  // now carry the information over to the original map
  std::vector< int > moduleTranslationVector;
  moduleTranslationVector.resize( getNumberOfModules( &vertexToModuleSubMap ), 0 );
  int originalNumber = getNumberOfModules( vertexToModuleMap );

  // the new parts are added at the end
  for(int index( 0 ); index < moduleTranslationVector.size()  ; index++)
  {
    moduleTranslationVector[ index ] = originalNumber + index;
  }

  ToModuleMapType::iterator iter2 = vertexToModuleSubMap.begin();
  ToModuleMapType::iterator end2 =  vertexToModuleSubMap.end();

  while( iter2 != end2 )
  {
    // translate vertex descriptor from subgraph to graph
    VertexDescriptorType key = subgraphToGraphVertexMap.find( iter2->first )->second;
    // translate module number from subgraph to graph
    int value = moduleTranslationVector[ iter2->second ];
    // change the corresponding entry
    vertexToModuleMap->find( key )->second = value;

    iter2++;
  }

  // remove the now empty module, that was splitted
  removeModule( vertexToModuleMap, moduleToSplit );

}

void mitk::ConnectomicsSimulatedAnnealingPermutationModularity::extractModuleSubgraph(
  ToModuleMapType *vertexToModuleMap,
  mitk::ConnectomicsNetwork::Pointer network,
  int moduleToSplit,
  mitk::ConnectomicsNetwork::Pointer subNetwork,
  VertexToVertexMapType* graphToSubgraphVertexMap,
  VertexToVertexMapType* subgraphToGraphVertexMap )
{
  const std::vector< VertexDescriptorType > allNodesVector = network->GetVectorOfAllVertexDescriptors();

  // add vertices to subgraph
  for( int nodeNumber( 0 ); nodeNumber < allNodesVector.size() ; nodeNumber++)
  {
    int correspondingModule = vertexToModuleMap->find( allNodesVector[ nodeNumber ] )->second;

    if( moduleToSplit == vertexToModuleMap->find( allNodesVector[ nodeNumber ] )->second )
    {
      int id = network->GetNode( allNodesVector[ nodeNumber ] ).id;
      VertexDescriptorType newVertex = subNetwork->AddVertex( id );

      graphToSubgraphVertexMap->insert(
        std::pair<VertexDescriptorType, VertexDescriptorType>(
        allNodesVector[ nodeNumber ], newVertex
        )
        );
      subgraphToGraphVertexMap->insert(
        std::pair<VertexDescriptorType, VertexDescriptorType>(
        newVertex, allNodesVector[ nodeNumber ]
      )
        );
    }
  }

  // add edges to subgraph
  VertexToVertexMapType::iterator iter = graphToSubgraphVertexMap->begin();
  VertexToVertexMapType::iterator end =  graphToSubgraphVertexMap->end();

  while( iter != end )
  {
    const std::vector< VertexDescriptorType > adjacentNodexVector
      = network->GetVectorOfAdjacentNodes( iter->first );

    for( int adjacentNodeNumber( 0 ); adjacentNodeNumber < adjacentNodexVector.size() ; adjacentNodeNumber++)
    {
      // if the adjacent vertex is part of the subgraph,
      // add edge, if it does not exist yet, else do nothing

      VertexDescriptorType adjacentVertex = adjacentNodexVector[ adjacentNodeNumber ];
      if( graphToSubgraphVertexMap->count( adjacentVertex ) > 0 )
      {
        if( !subNetwork->EdgeExists( iter->second, graphToSubgraphVertexMap->find( adjacentVertex )->second ) )
        { //edge exists in parent network, but not yet in sub network
          const VertexDescriptorType vertexA = iter->second;
          const VertexDescriptorType vertexB = graphToSubgraphVertexMap->find( adjacentVertex )->second;
          const int sourceID = network->GetNode( vertexA ).id;
          const int targetID = network->GetNode( vertexB ).id;
          const int weight = network->GetEdge( iter->first, graphToSubgraphVertexMap->find( adjacentVertex )->first ).weight;
          subNetwork->AddEdge( vertexA , vertexB, sourceID, targetID, weight );
        }
      }
    }
    iter++;
  }// end while( iter != end )
}

int mitk::ConnectomicsSimulatedAnnealingPermutationModularity::getNumberOfModules(
  ToModuleMapType *vertexToModuleMap ) const
{
  int maxModule( 0 );
  ToModuleMapType::iterator iter = vertexToModuleMap->begin();
  ToModuleMapType::iterator end =  vertexToModuleMap->end();
  while( iter != end )
  {
    if( iter->second > maxModule )
    {
      maxModule = iter->second;
    }

    iter++;
  }

  return maxModule + 1;
}

int mitk::ConnectomicsSimulatedAnnealingPermutationModularity::getNumberOfVerticesInModule(
  ToModuleMapType *vertexToModuleMap, int module ) const
{
  int number( 0 );
  ToModuleMapType::iterator iter = vertexToModuleMap->begin();
  ToModuleMapType::iterator end =  vertexToModuleMap->end();
  while( iter != end )
  {
    if( iter->second == module )
    {
      number++;
    }

    iter++;
  }

  return number;
}

void mitk::ConnectomicsSimulatedAnnealingPermutationModularity::removeModule(
  ToModuleMapType *vertexToModuleMap, int module )
{
  int lastModuleNumber = getNumberOfModules( vertexToModuleMap ) - 1;
  if( module == lastModuleNumber )
  {
    // no need to do anything, the last module is deleted "automatically"
    return;
  }

  if( getNumberOfVerticesInModule( vertexToModuleMap, module ) > 0 )
  {
    MBI_WARN << "Trying to remove non-empty module";
    return;
  }

  ToModuleMapType::iterator iter = vertexToModuleMap->begin();
  ToModuleMapType::iterator end =  vertexToModuleMap->end();
  while( iter != end )
  {
    if( iter->second == lastModuleNumber )
    {
      // renumber last module to to-be-deleted module
      iter->second = module;
    }

    iter++;
  }
}

void mitk::ConnectomicsSimulatedAnnealingPermutationModularity::randomlyAssignNodesToModules(
  ToModuleMapType *vertexToModuleMap, int numberOfIntendedModules )
{
  // we make sure that each intended module contains *at least* one node
  // thus if more modules are asked for than node exists we will only generate
  // as many modules as there are nodes
  if( numberOfIntendedModules > vertexToModuleMap->size() )
  {
    MBI_ERROR << "Tried to generate more modules than vertices were provided";
    numberOfIntendedModules = vertexToModuleMap->size();
  }

  //the random number generators
  vnl_random rng( (unsigned int) rand() );

  std::vector< int > histogram;
  std::vector< int > nodeList;

  histogram.resize( numberOfIntendedModules, 0 );
  nodeList.resize( vertexToModuleMap->size(), 0 );

  int numberOfVertices = vertexToModuleMap->size();

  //randomly distribute nodes to modules
  for( int nodeIndex( 0 ); nodeIndex < nodeList.size(); nodeIndex++ )
  {
    //select random module
    nodeList[ nodeIndex ] = rng.lrand32( numberOfIntendedModules - 1 );

    histogram[ nodeList[ nodeIndex ] ]++;

  }

  // make sure no module contains no node, if one does assign it one of a random module
  // that does contain at least two
  for( int moduleIndex( 0 ); moduleIndex < histogram.size(); moduleIndex++ )
  {
    while( histogram[ moduleIndex ] == 0 )
    {
      int randomNodeIndex = rng.lrand32( numberOfVertices - 1 );
      if( histogram[ nodeList[ randomNodeIndex ] ] > 1 )
      {
        histogram[ moduleIndex ]++;
        histogram[ nodeList[ randomNodeIndex ] ]--;
        nodeList[ randomNodeIndex ] = moduleIndex;
      }
    }
  }

  ToModuleMapType::iterator iter = vertexToModuleMap->begin();
  ToModuleMapType::iterator end =  vertexToModuleMap->end();

  for( int index( 0 ); ( iter != end ) && ( index < nodeList.size() ); index++, iter++ )
  {
    iter->second = nodeList[ index ] ;
  }
}

void mitk::ConnectomicsSimulatedAnnealingPermutationModularity::SetMapping( ToModuleMapType mapping )
{
  m_BestSolution = mapping;
}

mitk::ConnectomicsSimulatedAnnealingPermutationModularity::ToModuleMapType
mitk::ConnectomicsSimulatedAnnealingPermutationModularity::GetMapping()
{
  return m_BestSolution;
}

double mitk::ConnectomicsSimulatedAnnealingPermutationModularity::Evaluate( ToModuleMapType* mapping ) const
{
  mitk::ConnectomicsSimulatedAnnealingCostFunctionModularity* costMapping =
    dynamic_cast<mitk::ConnectomicsSimulatedAnnealingCostFunctionModularity*>( m_CostFunction.GetPointer() );
  if( costMapping )
  {
    return  costMapping->Evaluate( m_Network, mapping );
  }
  else
  {
    return 0;
  }
}

bool mitk::ConnectomicsSimulatedAnnealingPermutationModularity::AcceptChange( double costBefore, double costAfter, double temperature ) const
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


void mitk::ConnectomicsSimulatedAnnealingPermutationModularity::SetDepth( int depth )
{
  m_Depth = depth;
}

void mitk::ConnectomicsSimulatedAnnealingPermutationModularity::SetStepSize( double size )
{
  m_StepSize = size;
}
