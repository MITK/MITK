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

mitk::ConnectomicsNetwork::Pointer mitk::ConnectomicsNetworkThresholder::ThresholdByRandomRemoval( mitk::ConnectomicsNetwork::Pointer input, double targetDensity )
{
  return input;
}

mitk::ConnectomicsNetwork::Pointer mitk::ConnectomicsNetworkThresholder::ThresholdBelowDensity( mitk::ConnectomicsNetwork::Pointer input, double targetDensity )
{
  mitk::ConnectomicsNetwork::Pointer result( input );

  mitk::ConnectomicsStatisticsCalculator::Pointer calculator;

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
  mitk::ConnectomicsNetwork::Pointer result( input );

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

    for ( ; iterator != end && !edgeHasBeenRemoved; ++iterator)
    {
      // If the edge is below the target threshold it is deleted
      if( (*boostGraph)[ *iterator ].weight < targetThreshold )
      {
        edgeHasBeenRemoved = true;
        // this invalidates all iterators
        boost::remove_edge( *iterator, *boostGraph );
      }
    }
  }

  result->UpdateIDs();

  return result;
}
