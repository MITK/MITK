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

#include "mitkConnectomicsSyntheticNetworkGenerator.h"

#include <sstream>
#include <vector>

#include "mitkConnectomicsConstantsManager.h"

#include <vtkMatrix4x4.h>

//for random number generation
#include "vxl/core/vnl/vnl_random.h"
#include "vxl/core/vnl/vnl_math.h"

mitk::ConnectomicsSyntheticNetworkGenerator::ConnectomicsSyntheticNetworkGenerator()
: m_LastGenerationWasSuccess( false )
{
}

mitk::ConnectomicsSyntheticNetworkGenerator::~ConnectomicsSyntheticNetworkGenerator()
{
}

mitk::ConnectomicsNetwork::Pointer mitk::ConnectomicsSyntheticNetworkGenerator::CreateSyntheticNetwork(int networkTypeId, int paramterOne, double parameterTwo)
{
  mitk::ConnectomicsNetwork::Pointer network = mitk::ConnectomicsNetwork::New();

  m_LastGenerationWasSuccess = false;
  // give the network an artificial geometry
  network->SetGeometry( this->GenerateDefaultGeometry() );

  switch (networkTypeId) {
  case 0:
    GenerateSyntheticCubeNetwork( network, paramterOne, parameterTwo );
    break;
  case 1:
    GenerateSyntheticCenterToSurfaceNetwork( network, paramterOne, parameterTwo );
    break;
  case 2:
    GenerateSyntheticRandomNetwork( network, paramterOne, parameterTwo );
    break;
  case 3:
    //GenerateSyntheticScaleFreeNetwork( network, 1000 );
    break;
  case 4:
    //GenerateSyntheticSmallWorldNetwork( network, 1000 );
    break;
  default:
    MBI_ERROR << "Unrecognized Network ID";
  }

  network->UpdateBounds();

  return network;

}

mitk::Geometry3D::Pointer mitk::ConnectomicsSyntheticNetworkGenerator::GenerateDefaultGeometry()
{
  mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();

  double zero( 0.0 );
  double one( 1.0 );
  // origin = {0,0,0}
  mitk::Point3D origin;

  origin[0] = zero;
  origin[1] = zero;
  origin[2] = zero;
  geometry->SetOrigin(origin);

  // spacing = {1,1,1}
  float spacing[3];
  spacing[0] = one;
  spacing[1] = one;
  spacing[2] = one;
  geometry->SetSpacing(spacing);

  // transform
  vtkMatrix4x4* transformMatrix = vtkMatrix4x4::New();
  transformMatrix->SetElement(0,0,one);
  transformMatrix->SetElement(1,0,zero);
  transformMatrix->SetElement(2,0,zero);
  transformMatrix->SetElement(0,1,zero);
  transformMatrix->SetElement(1,1,one);
  transformMatrix->SetElement(2,1,zero);
  transformMatrix->SetElement(0,2,zero);
  transformMatrix->SetElement(1,2,zero);
  transformMatrix->SetElement(2,2,one);

  transformMatrix->SetElement(0,3,origin[0]);
  transformMatrix->SetElement(1,3,origin[1]);
  transformMatrix->SetElement(2,3,origin[2]);
  transformMatrix->SetElement(3,3,1);
  geometry->SetIndexToWorldTransformByVtkMatrix( transformMatrix );

  geometry->SetImageGeometry(true);

  return geometry;
}

void mitk::ConnectomicsSyntheticNetworkGenerator::GenerateSyntheticCubeNetwork(
  mitk::ConnectomicsNetwork::Pointer network, int cubeExtent, double distance )
{
  // map for storing the conversion from indices to vertex descriptor
  std::map< int, mitk::ConnectomicsNetwork::VertexDescriptorType > idToVertexMap;

  int vertexID(0);
  for( int loopX( 0 ); loopX < cubeExtent; loopX++  )
  {
    for( int loopY( 0 ); loopY < cubeExtent; loopY++  )
    {
      for( int loopZ( 0 ); loopZ < cubeExtent; loopZ++  )
      {
        std::vector< float > position;
        std::string label;
        std::stringstream labelStream;
        labelStream << vertexID;
        label = labelStream.str();

        position.push_back( loopX * distance );
        position.push_back( loopY * distance );
        position.push_back( loopZ * distance );

        mitk::ConnectomicsNetwork::VertexDescriptorType newVertex = network->AddVertex( vertexID );
        network->SetLabel( newVertex, label );
        network->SetCoordinates( newVertex, position );

        if ( idToVertexMap.count( vertexID ) > 0 )
        {
          MITK_ERROR << "Aborting network creation, duplicate vertex ID generated.";
          m_LastGenerationWasSuccess = false;
          return;
        }
        idToVertexMap.insert( std::pair< int, mitk::ConnectomicsNetwork::VertexDescriptorType >( vertexID, newVertex) );

        vertexID++;
      }
    }
  }

  int edgeID(0), edgeSourceID(0), edgeTargetID(0);
  // uniform weight of one
  int edgeWeight(1);

  mitk::ConnectomicsNetwork::VertexDescriptorType source;
  mitk::ConnectomicsNetwork::VertexDescriptorType target;

  for( int loopX( 0 ); loopX < cubeExtent; loopX++  )
  {
    for( int loopY( 0 ); loopY < cubeExtent; loopY++  )
    {
      for( int loopZ( 0 ); loopZ < cubeExtent; loopZ++  )
      {
        // to avoid creating an edge twice (this being an undirected graph) we only generate
        // edges in three directions, the others will be supplied by the corresponding nodes
        if( loopX != 0 )
        {
          edgeTargetID = edgeSourceID - cubeExtent * cubeExtent;

          source = idToVertexMap.find( edgeSourceID )->second;
          target = idToVertexMap.find( edgeTargetID )->second;
          network->AddEdge( source, target, edgeSourceID, edgeTargetID, edgeWeight);

          edgeID++;
        }
        if( loopY != 0 )
        {
          edgeTargetID = edgeSourceID - cubeExtent;

          source = idToVertexMap.find( edgeSourceID )->second;
          target = idToVertexMap.find( edgeTargetID )->second;
          network->AddEdge( source, target, edgeSourceID, edgeTargetID, edgeWeight);

          edgeID++;
        }
        if( loopZ != 0 )
        {
          edgeTargetID = edgeSourceID - 1;

          source = idToVertexMap.find( edgeSourceID )->second;
          target = idToVertexMap.find( edgeTargetID )->second;
          network->AddEdge( source, target, edgeSourceID, edgeTargetID, edgeWeight);

          edgeID++;
        }

        edgeSourceID++;
      } // end for( int loopZ( 0 ); loopZ < cubeExtent; loopZ++  )
    } // end for( int loopY( 0 ); loopY < cubeExtent; loopY++  )
  } // end for( int loopX( 0 ); loopX < cubeExtent; loopX++  )
  m_LastGenerationWasSuccess = true;
}

void mitk::ConnectomicsSyntheticNetworkGenerator::GenerateSyntheticCenterToSurfaceNetwork(
  mitk::ConnectomicsNetwork::Pointer network, int numberOfPoints, double radius )
{
  //the random number generators
  unsigned int randomOne = (unsigned int) rand();
  unsigned int randomTwo = (unsigned int) rand();

  vnl_random rng( (unsigned int) rand() );
  vnl_random rng2( (unsigned int) rand() );

  mitk::ConnectomicsNetwork::VertexDescriptorType centerVertex;
  int vertexID(0);
  { //add center vertex
    std::vector< float > position;
    std::string label;
    std::stringstream labelStream;
    labelStream << vertexID;
    label = labelStream.str();

    position.push_back( 0 );
    position.push_back( 0 );
    position.push_back( 0 );

    centerVertex = network->AddVertex( vertexID );
    network->SetLabel( centerVertex, label );
    network->SetCoordinates( centerVertex, position );
  }//end add center vertex

  // uniform weight of one
  int edgeWeight(1);

  mitk::ConnectomicsNetwork::VertexDescriptorType source;
  mitk::ConnectomicsNetwork::VertexDescriptorType target;

  //add vertices on sphere surface
  for( int loopID( 1 ); loopID < numberOfPoints; loopID++  )
  {

    std::vector< float > position;
    std::string label;
    std::stringstream labelStream;
    labelStream << loopID;
    label = labelStream.str();

    //generate random, uniformly distributed points on a sphere surface
    const double uVariable = rng.drand64( 0.0 , 1.0);
    const double vVariable = rng.drand64( 0.0 , 1.0);
    const double phi = 2 * vnl_math::pi * uVariable;
    const double theta = std::acos( 2 * vVariable - 1 );

    double xpos = radius * std::cos( phi ) * std::sin( theta );
    double ypos = radius * std::sin( phi ) * std::sin( theta );
    double zpos = radius * std::cos( theta );

    position.push_back( xpos );
    position.push_back( ypos );
    position.push_back( zpos );

    mitk::ConnectomicsNetwork::VertexDescriptorType newVertex = network->AddVertex( loopID );
    network->SetLabel( newVertex, label );
    network->SetCoordinates( newVertex, position );

    network->AddEdge( newVertex, centerVertex, loopID, 0, edgeWeight);
  }
  m_LastGenerationWasSuccess = true;
}

void mitk::ConnectomicsSyntheticNetworkGenerator::GenerateSyntheticRandomNetwork(
  mitk::ConnectomicsNetwork::Pointer network, int numberOfPoints, double threshold )
{
  // as the surface is proportional to the square of the radius the density stays the same
  double radius = 5 * std::sqrt( (float) numberOfPoints );

  //the random number generators
  unsigned int randomOne = (unsigned int) rand();
  unsigned int randomTwo = (unsigned int) rand();

  vnl_random rng( (unsigned int) rand() );
  vnl_random rng2( (unsigned int) rand() );

  // map for storing the conversion from indices to vertex descriptor
  std::map< int, mitk::ConnectomicsNetwork::VertexDescriptorType > idToVertexMap;

  //add vertices on sphere surface
  for( int loopID( 0 ); loopID < numberOfPoints; loopID++  )
  {

    std::vector< float > position;
    std::string label;
    std::stringstream labelStream;
    labelStream << loopID;
    label = labelStream.str();

    //generate random, uniformly distributed points on a sphere surface
    const double uVariable = rng.drand64( 0.0 , 1.0);
    const double vVariable = rng.drand64( 0.0 , 1.0);
    const double phi = 2 * vnl_math::pi * uVariable;
    const double theta = std::acos( 2 * vVariable - 1 );

    double xpos = radius * std::cos( phi ) * std::sin( theta );
    double ypos = radius * std::sin( phi ) * std::sin( theta );
    double zpos = radius * std::cos( theta );

    position.push_back( xpos );
    position.push_back( ypos );
    position.push_back( zpos );

    mitk::ConnectomicsNetwork::VertexDescriptorType newVertex = network->AddVertex( loopID );
    network->SetLabel( newVertex, label );
    network->SetCoordinates( newVertex, position );

    if ( idToVertexMap.count( loopID ) > 0 )
    {
      MITK_ERROR << "Aborting network creation, duplicate vertex ID generated.";
      m_LastGenerationWasSuccess = false;
      return;
    }
    idToVertexMap.insert( std::pair< int, mitk::ConnectomicsNetwork::VertexDescriptorType >( loopID, newVertex) );
  }

  int edgeID(0);
  // uniform weight of one
  int edgeWeight(1);

  mitk::ConnectomicsNetwork::VertexDescriptorType source;
  mitk::ConnectomicsNetwork::VertexDescriptorType target;

  for( int loopID( 0 ); loopID < numberOfPoints; loopID++  )
  {
    // to avoid creating an edge twice (this being an undirected graph) we only
    // potentially generate edges with all nodes with a bigger ID
    for( int innerLoopID( loopID ); innerLoopID < numberOfPoints; innerLoopID++  )
    {
      if( rng.drand64( 0.0 , 1.0) > threshold)
      {
        // do nothing
      }
      else
      {
        source = idToVertexMap.find( loopID )->second;
        target = idToVertexMap.find( innerLoopID )->second;
        network->AddEdge( source, target, loopID, innerLoopID, edgeWeight);

        edgeID++;
      }
    } // end for( int innerLoopID( loopID ); innerLoopID < numberOfPoints; innerLoopID++  )
  } // end for( int loopID( 0 ); loopID < numberOfPoints; loopID++  )
  m_LastGenerationWasSuccess = true;
}

bool mitk::ConnectomicsSyntheticNetworkGenerator::WasGenerationSuccessfull()
{
  return m_LastGenerationWasSuccess;
}
