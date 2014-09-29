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

#include "mitkConnectomicsNetworkThresholder.h"
#include <mitkConnectomicsStatisticsCalculator.h>
#include "vnl/vnl_random.h"

mitk::ConnectomicsNetworkThresholder::ConnectomicsNetworkThresholder()
  : m_Network( 0 )
  , m_ThresholdingScheme( mitk::ConnectomicsNetworkThresholder::ThresholdBased )
  , m_TargetThreshold( 0.0 )
  , m_TargetDensity( 1.0 )
{
}

mitk::ConnectomicsNetworkThresholder::~ConnectomicsNetworkThresholder()
{
}

mitk::ConnectomicsNetwork::Pointer mitk::ConnectomicsNetworkThresholder::GetThresholdedNetwork()
{
  mitk::ConnectomicsNetwork::Pointer result;

  if( ! CheckValidity() )
  {
    MITK_ERROR << "Aborting";
    return m_Network;
  }

  switch(m_ThresholdingScheme)
  {
  case RandomRemovalOfWeakest :
    {
      result = ThresholdByRandomRemoval( m_Network, m_TargetDensity );
      break;
    }
  case LargestLowerThanDensity :
    {
      result = ThresholdBelowDensity( m_Network, m_TargetDensity );
      break;
    }
  case ThresholdBased :
    {
      result = Threshold( m_Network, m_TargetThreshold );
      break;
    }
  default :
    {
      MITK_ERROR << "Specified unknown Thresholding Scheme";
      result = m_Network;
    }
  }

  return result;
}

bool mitk::ConnectomicsNetworkThresholder::CheckValidity()
{
  bool valid(true);

  if( m_Network.IsNull() )
  {
    valid = false;
    MITK_ERROR << "Network is NULL.";
  }

  switch(m_ThresholdingScheme)
  {
  case RandomRemovalOfWeakest :
  case LargestLowerThanDensity :
    {
      if( m_TargetDensity < 0.0 )
      {
        valid = false;
        MITK_ERROR << "Target density is negative.";
      }
      if( m_TargetDensity > 1.0 )
      {
        valid = false;
        MITK_ERROR << "Target density is larger than 1.";
      }
      break;
    }
  case ThresholdBased :
    {
      if( m_TargetThreshold < 0 )
      {
        valid = false;
        MITK_ERROR << "Target threshold is negative.";
      }
      break;
    }
  default :
    {
      valid = false;
      MITK_ERROR << "Specified unknown Thresholding Scheme";
    }
  }

  return valid;
}

mitk::ConnectomicsNetwork::Pointer mitk::ConnectomicsNetworkThresholder::ThresholdByRandomRemoval( mitk::ConnectomicsNetwork::Pointer input, double targetDensity )
{
  mitk::ConnectomicsNetwork::Pointer result = mitk::ConnectomicsNetwork::New();
  result->ImportNetwort( input );

  mitk::ConnectomicsStatisticsCalculator::Pointer calculator = mitk::ConnectomicsStatisticsCalculator::New();

  calculator->SetNetwork( result );
  calculator->Update();

  //the random number generator
  vnl_random rng( (unsigned int) rand() );

  bool notBelow( targetDensity < calculator->GetConnectionDensity() );

  double minWeight( result->GetMaximumWeight() );
  int count( 0 );

  while( notBelow )
  {
 //   result = Threshold( result, loop );
    std::vector< EdgeDescriptorType > candidateVector;
    minWeight = result->GetMaximumWeight();
    count = 0;

    // determine minimum weight and number of edges having that weight
    NetworkType* boostGraph = result->GetBoostGraph();
    EdgeIteratorType iterator, end;
    // sets iterator to start end end to end
    boost::tie(iterator, end) = boost::edges( *boostGraph );

    for ( ; iterator != end; ++iterator)
    {
      double tempWeight;

      // the value of an iterator is a descriptor
      tempWeight = (*boostGraph)[ *iterator ].weight;

      if( mitk::Equal( tempWeight, minWeight ) )
      {
        candidateVector.push_back( *iterator );
        count++;
      }
      else if( tempWeight < minWeight )
      {
        candidateVector.clear();
        candidateVector.push_back( *iterator );
        minWeight = tempWeight;
        count = 1;
      }
    }

    // Which to delete
    int deleteNumber( rng.lrand32( count - 1 ) );

    boost::remove_edge( candidateVector.at( deleteNumber ), *boostGraph );

    calculator->SetNetwork( result );
    calculator->Update();
    notBelow = targetDensity < calculator->GetConnectionDensity();
  }

  result->UpdateIDs();
  return result;
}

mitk::ConnectomicsNetwork::Pointer mitk::ConnectomicsNetworkThresholder::ThresholdBelowDensity( mitk::ConnectomicsNetwork::Pointer input, double targetDensity )
{
  mitk::ConnectomicsNetwork::Pointer result = mitk::ConnectomicsNetwork::New();
  result->ImportNetwort( input );

  mitk::ConnectomicsStatisticsCalculator::Pointer calculator = mitk::ConnectomicsStatisticsCalculator::New();

  calculator->SetNetwork( result );
  calculator->Update();

  bool notBelow( targetDensity < calculator->GetConnectionDensity() );

  for( int loop( 1 ); notBelow; loop++ )
  {
    result = Threshold( result, loop );
    calculator->SetNetwork( result );
    calculator->Update();
    notBelow = targetDensity < calculator->GetConnectionDensity();
  }

  return result;
}

mitk::ConnectomicsNetwork::Pointer mitk::ConnectomicsNetworkThresholder::Threshold( mitk::ConnectomicsNetwork::Pointer input, double targetThreshold )
{
  mitk::ConnectomicsNetwork::Pointer result = mitk::ConnectomicsNetwork::New();
  result->ImportNetwort( input );

  NetworkType* boostGraph = result->GetBoostGraph();

  EdgeIteratorType iterator, end;

  // set to true if iterators are invalidated by deleting an edge
  bool edgeHasBeenRemoved( true );

  // if no edge has been removed in the last loop, we are done
  while( edgeHasBeenRemoved )
  {
    edgeHasBeenRemoved = false;
    // sets iterator to start and end to end
    boost::tie(iterator, end) = boost::edges( *boostGraph );

    bool endReached( false );
    while( !edgeHasBeenRemoved && !endReached )
    {
      if( iterator != end )
      {
        // If the edge is below the target threshold it is deleted
        if( (*boostGraph)[ *iterator ].weight < targetThreshold )
        {
          edgeHasBeenRemoved = true;
          // this invalidates all iterators
          boost::remove_edge( *iterator, *boostGraph );
        }
        else
        {
          ++iterator;
        }
      }
      else
      {
        endReached = true;
      }
    }
  }

  result->UpdateIDs();

  return result;
}
