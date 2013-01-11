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

#include <itkConnectomicsNetworkToConnectivityMatrixImageFilter.h>

#include <itkImageRegionIterator.h>

#include <vector>

itk::ConnectomicsNetworkToConnectivityMatrixImageFilter::ConnectomicsNetworkToConnectivityMatrixImageFilter()
  : m_BinaryConnectivity(false)
  , m_RescaleConnectivity(false)
  , m_InputNetwork(NULL)
{
}


itk::ConnectomicsNetworkToConnectivityMatrixImageFilter::~ConnectomicsNetworkToConnectivityMatrixImageFilter()
{
}

void itk::ConnectomicsNetworkToConnectivityMatrixImageFilter::GenerateData()
{

  this->AllocateOutputs();

  OutputImageType::Pointer output = this->GetOutput();

  if(m_InputNetwork.IsNull())
  {
    MITK_ERROR << "Failed to generate data, no valid network was set.";
    return;
  }

  typedef mitk::ConnectomicsNetwork::NetworkType NetworkType;
  typedef boost::graph_traits< NetworkType >::vertex_descriptor DescriptorType;
  typedef boost::graph_traits< NetworkType >::vertex_iterator IteratorType;

  // prepare connectivity matrix for data
  std::vector< std::vector< int > > connectivityMatrix;
  int numberOfVertices = m_InputNetwork->GetNumberOfVertices();

  connectivityMatrix.resize( numberOfVertices );
  for( int index(0); index < numberOfVertices; index++ )
  {
    connectivityMatrix[ index ].resize( numberOfVertices );
  }

  // Create LabelToIndex translation
  std::map< std::string, DescriptorType > labelToIndexMap;

  NetworkType boostGraph = *(m_InputNetwork->GetBoostGraph());

  // translate index to label
  IteratorType iterator, end;
  boost::tie(iterator, end) = boost::vertices( boostGraph );

  for ( ; iterator != end; ++iterator)
  {
    labelToIndexMap.insert(
      std::pair< std::string, DescriptorType >(
      boostGraph[ *iterator ].label, *iterator )
      );
  }

  std::vector< std::string > indexToLabel;

  // translate index to label
  indexToLabel.resize( numberOfVertices );

  boost::tie(iterator, end) = boost::vertices( boostGraph );

  for ( ; iterator != end; ++iterator)
  {
    indexToLabel[ *iterator ] = boostGraph[ *iterator ].label;
  }

  //translate label to position
  std::vector< std::string > positionToLabelVector;
  positionToLabelVector = indexToLabel;

  std::sort ( positionToLabelVector.begin(), positionToLabelVector.end() );

  for( int outerLoop( 0 ); outerLoop < numberOfVertices; outerLoop++  )
  {
    DescriptorType fromVertexDescriptor = labelToIndexMap.find( positionToLabelVector[ outerLoop ] )->second;
    for( int innerLoop( outerLoop + 1 ); innerLoop < numberOfVertices; innerLoop++  )
    {
      DescriptorType toVertexDescriptor = labelToIndexMap.find( positionToLabelVector[ innerLoop ] )->second;


      int weight( 0 );

      if( boost::edge(toVertexDescriptor, fromVertexDescriptor, boostGraph ).second )
      {
        weight = m_InputNetwork->GetEdge( fromVertexDescriptor , toVertexDescriptor ).weight;;
      }
      connectivityMatrix[outerLoop][innerLoop] = weight;
      connectivityMatrix[innerLoop][outerLoop] = weight;
    }
  }


  OutputImageType::SpacingType spacing;
  spacing[0] = 1.0;
  spacing[1] = 1.0;
  OutputImageType::PointType origin;
  origin[0] = 0.0;
  origin[1] = 0.0;
  OutputImageType::IndexType index;
  index[0] = 0.0;
  index[1] = 0.0;
  OutputImageType::SizeType size;
  size[0] = numberOfVertices;
  size[1] = numberOfVertices;
  OutputImageType::RegionType region;
  region.SetIndex( index );
  region.SetSize( size );

  output->SetSpacing( spacing );   // Set the image spacing
  output->SetOrigin( origin );     // Set the image origin
  output->SetRegions( region );
  output->Allocate();
  output->FillBuffer(0);


  itk::ImageRegionIterator< OutputImageType > it_connect(output, output->GetLargestPossibleRegion());

  int counter( 0 );

  double rescaleFactor( 1.0 );
  double rescaleMax( 255.0 );

  if( m_RescaleConnectivity )
  {
    rescaleFactor = rescaleMax / m_InputNetwork->GetMaximumWeight();
  }

  // Colour pixels according to connectivity
  if(  m_BinaryConnectivity )
  {
    // binary connectivity
    while( !it_connect.IsAtEnd() )
    {
      if( connectivityMatrix[ ( counter - counter % numberOfVertices ) / numberOfVertices][ counter % numberOfVertices ] )
      {
        it_connect.Set( 1 );
      }
      else
      {
        it_connect.Set( 0 );
      }
      ++it_connect;
      counter++;
    }
  }
  else
  {
    // if desired rescale to the 0-255 range
    while( !it_connect.IsAtEnd() )
    {
      it_connect.Set( ( unsigned short ) rescaleFactor *
        connectivityMatrix[ ( counter - counter % numberOfVertices ) / numberOfVertices][ counter % numberOfVertices ]
      );
      ++it_connect;
      counter++;
    }
  }

}