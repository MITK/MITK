
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

#ifndef _MITK_CORRELATIONCALCULATOR_TXX
#define _MITK_CORRELATIONCALCULATOR_TXX

#include <algorithm>
#include <mitkLogMacros.h>
#include <mitkLog.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
#include <mitkImagePixelReadAccessor.h>

#include <itkImageRegionConstIteratorWithIndex.h>

template< class T >
mitk::CorrelationCalculator<T>::CorrelationCalculator()
    : m_InvalidTimeSeries( true )
    , m_ParcellationImage( nullptr )
    , m_TimeSeriesImage( nullptr )
{
}

template< class T >
mitk::CorrelationCalculator<T>::~CorrelationCalculator()
{
}

template< class T >
double mitk::CorrelationCalculator<T>::CalculatePearsonCorrelationCoefficient( const std::vector<T>& one, const std::vector<T>& two )
{
    double standardDevOne( std::sqrt(CalculateCovariance( one, one )) );
    double standardDevTwo( std::sqrt(CalculateCovariance( two, two )) );

    return CalculateCovariance(one, two) / (standardDevOne * standardDevTwo);

}

template< class T >
double mitk::CorrelationCalculator<T>::CalculateCovariance( const std::vector<T>& one, const std::vector<T>& two )
{
    if( one.size() != two.size() )
    {
        MITK_WARN << "Expect two same length vectors for covariance.";
        return 0;
    }

    double meanOne( std::accumulate( one.begin(), one.end(), 0.0 ) / double(one.size()) );
    double meanTwo( std::accumulate( two.begin(), two.end(), 0.0 ) / double(two.size()) );

    double sum( 0.0 );

    for(unsigned int index(0); index < one.size(); ++index)
    {
        sum += (one[index] - meanOne ) * (two[index] - meanTwo);
    }

    double n( one.size() );

    return sum/n;
}

template< class T >
std::vector< double > mitk::CorrelationCalculator<T>::ExtractAverageTimeSeriesOfParcel( int parcelValue )
{
  if( this->m_InvalidTimeSeries)
  {
    this->ExtractAllAverageTimeSeries();
    this->m_InvalidTimeSeries = false;
  }

  std::vector< double > result;

  if( m_AverageTimeSeries.count(parcelValue) > 0 )
  {
    result = m_AverageTimeSeries.find(parcelValue)->second;
  }

  return result;
}

template< class T >
void mitk::CorrelationCalculator<T>::DoWholeCorrelation(  )
{
  if(this->m_TimeSeriesImage->GetDimension() != 4 )
  {
    MITK_ERROR << "Expect 3D+t timeseries image.";
    return;
  }
  // calculate numbe of voxels
  unsigned int numberOfVoxels(1);
  unsigned int dim[3];
  for(unsigned int loop(0); loop < 3; ++loop)
  {
    numberOfVoxels *= this->m_TimeSeriesImage->GetDimension( loop );
    dim[loop] = this->m_TimeSeriesImage->GetDimension( loop );
  }
  m_CorrelationMatrix = vnl_matrix<double>(numberOfVoxels, numberOfVoxels);
  unsigned int numberOfSteps( this->m_TimeSeriesImage->GetDimension( 3 ));

  //iterate over all voxels and calculate correlation
  for( unsigned int i( 0 ); i < numberOfVoxels; ++i )
  {
    for( unsigned int j(0); j < i; ++j)
    {
      std::vector< T > one, two;
      one.resize(numberOfSteps);
      two.resize(numberOfSteps);

      mitk::ImagePixelReadAccessor<T, 4> readAccess(m_TimeSeriesImage);

      itk::Index<4> idx_i;
      itk::Index<4> idx_j;

      idx_i[2] = (i / (dim[0] * dim[1])) % dim[2];
      idx_i[1] = (i / dim[0]) % dim[1];
      idx_i[0] = i % dim[0];

      idx_j[2] = (j / (dim[0] * dim[1])) % dim[2];
      idx_j[1] = (j / dim[0]) % dim[1];
      idx_j[0] = j % dim[0];

      for(unsigned int timestep(0); timestep < numberOfSteps; ++timestep)
      {
        idx_i[3] = timestep;
        idx_j[3] = timestep;
        one[timestep] = readAccess.GetPixelByIndex(idx_i);
        two[timestep] = readAccess.GetPixelByIndex(idx_j);
      }

      m_CorrelationMatrix[i][j] = mitk::CorrelationCalculator<T>::CalculatePearsonCorrelationCoefficient( one, two );
      m_CorrelationMatrix[j][i] = m_CorrelationMatrix[i][j];
    }
    m_CorrelationMatrix[i][i] = 1;
  }
}

template< class T >
void mitk::CorrelationCalculator<T>::DoParcelCorrelation(  )
{
  AccessFixedDimensionByItk(this->m_TimeSeriesImage, ExtractAllAverageTimeSeries, 4);
  this->ExtractCenterOfMassForParcels();

  // fill matrix
  m_CorrelationMatrix = vnl_matrix<double>(m_AverageTimeSeries.size(), m_AverageTimeSeries.size());

  unsigned int i(0);
  for( std::map< int, std::vector<double> >::const_iterator it(m_AverageTimeSeries.begin()); it != m_AverageTimeSeries.end(); ++it, ++i)
  {
    unsigned int j(0);
    for(std::map< int, std::vector<double> >::const_iterator inner_it(m_AverageTimeSeries.begin()); inner_it != it; ++inner_it, ++j)
    {
      m_CorrelationMatrix[i][j] = mitk::CorrelationCalculator<double>::CalculatePearsonCorrelationCoefficient( it->second, inner_it->second );
      m_CorrelationMatrix[j][i] = m_CorrelationMatrix[i][j];
    }
    m_CorrelationMatrix[i][i] = 1;
    m_LabelOrderMap.insert(std::pair< unsigned int, int >(i, it->first));
  }
}

template< class T >
void mitk::CorrelationCalculator<T>::Modified(  )
{
  Superclass::Modified();
  this->m_InvalidTimeSeries = true;
}

template< class T>
template< typename TPixel, unsigned int VImageDimension >
void mitk::CorrelationCalculator<T>::ExtractAllAverageTimeSeries( itk::Image<TPixel, VImageDimension>* itkTimeSeriesImage )
{
  std::map< int, std::vector< std::vector< T > > > storage;

  itk::ImageRegionConstIteratorWithIndex< itk::Image<TPixel, VImageDimension> > it_itkTimeSeriesImage(
              itkTimeSeriesImage, itkTimeSeriesImage->GetLargestPossibleRegion() );

  unsigned int timesteps( itkTimeSeriesImage->GetLargestPossibleRegion().GetSize()[3] );

  for( it_itkTimeSeriesImage.GoToBegin(); !it_itkTimeSeriesImage.IsAtEnd(); ++it_itkTimeSeriesImage )
  {
    itk::Index< 4 > itkIndex = it_itkTimeSeriesImage.GetIndex();
    itk::Index< 3 > itkIndex3D;
    for(unsigned int axis( 0 ); axis < 3; ++axis )
    {
      itkIndex3D.SetElement( axis, itkIndex.GetElement(axis) );
    }

    mitk::Point3D tempPoint;

    m_TimeSeriesImage->GetGeometry()->IndexToWorld( itkIndex3D, tempPoint );
    int value( std::floor( m_ParcellationImage->GetPixelValueByWorldCoordinate( tempPoint ) + 0.5 ) );

    if(storage.count(value) == 0)
    {
      std::vector< std::vector< T > > temp;
      temp.resize( timesteps );
      storage.insert(std::pair< int, std::vector< std::vector< T > > >(value, temp));
    }

    storage[value][ itkIndex.GetElement( 3 ) ].push_back( it_itkTimeSeriesImage.Value() );
  }

  // store means
  for( typename std::map< int, std::vector< std::vector< T > > >::iterator it = storage.begin(); it != storage.end(); ++it )
  {
    std::vector< double > means;
    means.resize( timesteps );
    for(unsigned int loop(0); loop < timesteps; ++loop)
    {
      means[loop] = std::accumulate( it->second[loop].begin(), it->second[loop].end(), 0.0 ) / double(it->second[loop].size());
    }
    m_AverageTimeSeries.insert( std::pair< int, std::vector< double > >(it->first, means) );
  }
}

template< class T >
void mitk::CorrelationCalculator<T>::ExtractCenterOfMassForParcels()
{
  itk::Image< int, 3 >::Pointer itkParcelImage;
  mitk::CastToItkImage( m_ParcellationImage, itkParcelImage );

  itk::ImageRegionConstIteratorWithIndex< itk::Image<int, 3> > it_itkParcelImage(
              itkParcelImage, itkParcelImage->GetLargestPossibleRegion() );

  std::map< int, std::vector< mitk::Point3D > > storage;

  for( it_itkParcelImage.GoToBegin(); !it_itkParcelImage.IsAtEnd(); ++it_itkParcelImage )
  {
    mitk::Point3D tempPoint;

    m_ParcellationImage->GetGeometry()->IndexToWorld( it_itkParcelImage.GetIndex(), tempPoint );

    if( storage.count( it_itkParcelImage.Value() ) == 1 )
    {
      storage[ it_itkParcelImage.Value() ].push_back( tempPoint );
    }
    else
    {
      storage.insert( std::pair< int, std::vector< mitk::Point3D > >(
                        it_itkParcelImage.Value(), std::vector< mitk::Point3D >( 1, tempPoint ) ));
    }
  }

  for( std::map< int, std::vector< mitk::Point3D > >::iterator it( storage.begin() ); it != storage.end(); ++it )
  {
    itk::Vector< int, 3 > tempVector( 0 );

    for( unsigned int loop(0); loop < it->second.size(); ++loop)
    {
      for(unsigned int index(0); index < 3; ++index)
      {
        tempVector[index] = tempVector[index] + it->second[loop][index];
      }
    }

    mitk::Point3D tempPoint;

    for( unsigned int loop(0); loop < 3; ++loop )
    {
      tempPoint.SetElement(loop, tempVector.GetElement(loop) / it->second.size());
    }

    m_ParcelCenterOfMass.insert( std::pair< int, mitk::Point3D >(it->first, tempPoint) );
  }
}

template< class T >
const vnl_matrix< double >* mitk::CorrelationCalculator<T>::GetCorrelationMatrix() const
{
  return &m_CorrelationMatrix;
}

template< class T >
const std::map< unsigned int, int >* mitk::CorrelationCalculator<T>::GetLabelOrderMap() const
{
  return &m_LabelOrderMap;
}

template< class T >
mitk::ConnectomicsNetwork::Pointer mitk::CorrelationCalculator<T>::GetConnectomicsNetwork()
{
  mitk::ConnectomicsNetwork::NetworkType* boostGraph = new mitk::ConnectomicsNetwork::NetworkType;

  std::map< int, mitk::ConnectomicsNetwork::VertexDescriptorType > idToVertexMap;

  // fill nodes
  unsigned int numberOfNodes( m_CorrelationMatrix.columns() );

  for(unsigned int loop(0); loop < numberOfNodes; ++loop )
  {
    idToVertexMap.insert( std::pair< int, mitk::ConnectomicsNetwork::VertexDescriptorType >(loop, boost::add_vertex( *boostGraph )) );
  }

  if( m_ParcelCenterOfMass.size() > 0)
  {
    std::map< int, mitk::Point3D >::const_iterator it = m_ParcelCenterOfMass.begin();
    for(unsigned int loop(0); loop < numberOfNodes, it != m_ParcelCenterOfMass.end(); ++loop, ++it )
    {
      (*boostGraph)[ idToVertexMap[loop] ].id = idToVertexMap[loop];
      (*boostGraph)[ idToVertexMap[loop] ].label = std::to_string( it->first );
      (*boostGraph)[ idToVertexMap[loop] ].coordinates.resize(3);
      for(unsigned int dimension(0); dimension < 3; ++dimension)
      {
        (*boostGraph)[ idToVertexMap[loop] ].coordinates[dimension] = it->second[dimension];
      }
    }
  }

  //fill edges
  for( unsigned int i(0); i < numberOfNodes; ++i)
  {
    for(unsigned int j(0); j < i; ++j)
    {
      mitk::ConnectomicsNetwork::VertexDescriptorType vertexA = idToVertexMap[i];
      mitk::ConnectomicsNetwork::VertexDescriptorType vertexB = idToVertexMap[j];
      boost::add_edge( vertexA, vertexB, *boostGraph );
      (*boostGraph)[ boost::edge(vertexA, vertexB, *boostGraph ).first ].sourceId = (*boostGraph)[vertexA].id;
      (*boostGraph)[ boost::edge(vertexA, vertexB, *boostGraph ).first ].targetId = (*boostGraph)[vertexB].id;
      (*boostGraph)[ boost::edge(vertexA, vertexB, *boostGraph ).first ].weight = 1;
      (*boostGraph)[ boost::edge(vertexA, vertexB, *boostGraph ).first ].edge_weight = m_CorrelationMatrix[i][j];
    }
  }

  // create data node
  mitk::ConnectomicsNetwork::Pointer network = mitk::ConnectomicsNetwork::New();
  network->SetBoostGraph( boostGraph );
  network->SetGeometry( dynamic_cast<mitk::BaseGeometry*>(m_TimeSeriesImage->GetGeometry()->Clone().GetPointer()) );
  network->UpdateBounds();
  network->SetIsModified( true );
  return network;
}

#endif /* _MITK_CORRELATIONCALCULATOR_TXX */
