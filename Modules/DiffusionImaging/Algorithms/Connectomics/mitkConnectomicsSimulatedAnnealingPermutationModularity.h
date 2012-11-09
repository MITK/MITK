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

#ifndef mitkConnectomicsSimulatedAnnealingPermutationModularity_h
#define mitkConnectomicsSimulatedAnnealingPermutationModularity_h

#include "mitkConnectomicsSimulatedAnnealingPermutationBase.h"

#include "mitkConnectomicsNetwork.h"

namespace mitk
{
  /**
  * \brief A class providing permutations for the calculation of modularity using simulated annealing */
  class MitkDiffusionImaging_EXPORT ConnectomicsSimulatedAnnealingPermutationModularity : public mitk::ConnectomicsSimulatedAnnealingPermutationBase
  {
  public:

    typedef mitk::ConnectomicsNetwork::VertexDescriptorType VertexDescriptorType;
    typedef std::map< VertexDescriptorType, int > ToModuleMapType;
    typedef std::map< VertexDescriptorType, VertexDescriptorType > VertexToVertexMapType;

    /** Standard class typedefs. */
    /** Method for creation through the object factory. */

    mitkClassMacro(ConnectomicsSimulatedAnnealingPermutationModularity, itk::Object);
    itkNewMacro(Self);

    // Initialize the permutation
    virtual void Initialize();

    // Do a permutation for a specific temperature
    virtual void Permutate( double temperature );

    // Do clean up necessary after a permutation
    virtual void CleanUp();

    // set the network permutation is to be run upon
    void SetNetwork( mitk::ConnectomicsNetwork::Pointer theNetwork );

    // Get the number of modules the graph has ( highest number - 1 )
    int getNumberOfModules( ToModuleMapType *vertexToModuleMap ) const;

    // Get the number of vertices belonging to a given module
    int getNumberOfVerticesInModule( ToModuleMapType *vertexToModuleMap, int module ) const;

    // Set the mapping
    void SetMapping( ToModuleMapType mapping );

    // Get the mapping
    ToModuleMapType GetMapping();

    // Set depth
    void SetDepth( int depth );

    // Set stepSize
    void SetStepSize( double size );

  protected:

    //////////////////// Functions ///////////////////////
    ConnectomicsSimulatedAnnealingPermutationModularity();
    ~ConnectomicsSimulatedAnnealingPermutationModularity();

    // This function moves one single node from a module to another
        void permutateMappingSingleNodeShift(
      ToModuleMapType *vertexToModuleMap,
      mitk::ConnectomicsNetwork::Pointer network );

        // This function splits and joins modules
    void permutateMappingModuleChange(
      ToModuleMapType *vertexToModuleMap,
      double currentTemperature,
      mitk::ConnectomicsNetwork::Pointer network );

    // join the two given modules to a single one
    void joinTwoModules( ToModuleMapType *vertexToModuleMap, int moduleA, int moduleB );

    // split the given module recursively up to a certain level
    // first randomly assigns nodes and then starts another simulated annealing
    // on the sub network, as long as depthOfModuleChange > 0
    void splitModule(
      ToModuleMapType *vertexToModuleMap,
      double currentTemperature,
      mitk::ConnectomicsNetwork::Pointer network,
      int moduleToSplit );

    // Extract the subgraph of a network containing all nodes
    // of a given module and the egdes between them
    void extractModuleSubgraph(
      ToModuleMapType *vertexToModuleMap,
      mitk::ConnectomicsNetwork::Pointer network,
      int moduleToSplit,
      mitk::ConnectomicsNetwork::Pointer subNetwork,
      VertexToVertexMapType* graphToSubgraphVertexMap,
      VertexToVertexMapType* subgraphToGraphVertexMap );

    // Remove empty modules by moving all nodes of the highest module to the given module
    void removeModule( ToModuleMapType *vertexToModuleMap, int module );

    // Randomly assign nodes to modules, this makes sure each module contains at least one node
    // as long as numberOfIntendedModules < number of nodes
    void randomlyAssignNodesToModules(ToModuleMapType *vertexToModuleMap, int numberOfIntendedModules );

    // Evaluate mapping using a modularity cost function
    double Evaluate( ToModuleMapType* mapping ) const;

    // Whether to accept the permutation
    bool AcceptChange( double costBefore, double costAfter, double temperature ) const;

    // the current best solution
    ToModuleMapType m_BestSolution;

    // the network
    mitk::ConnectomicsNetwork::Pointer m_Network;

    // How many levels of recursive calls can be gone down
    int m_Depth;

    // The step size for recursive configuring of simulated annealing manager
    double m_StepSize;
  };

}// end namespace mitk

#endif // mitkConnectomicsSimulatedAnnealingPermutationModularity_h
