
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

#include <mitkConnectomicsDegreeHistogram.h>

mitk::ConnectomicsDegreeHistogram::ConnectomicsDegreeHistogram()
{
  m_Subject = "Node degree";
}

mitk::ConnectomicsDegreeHistogram::~ConnectomicsDegreeHistogram()
{
}

void mitk::ConnectomicsDegreeHistogram::ComputeFromConnectomicsNetwork( ConnectomicsNetwork* source )
{
  std::vector< int > degreeOfNodesVector = source->GetDegreeOfNodes();

  int maximumDegree( 0 );

  for( int index( 0 ); index < degreeOfNodesVector.size(); index++ )
  {
    if( maximumDegree < degreeOfNodesVector[ index ] )
    {
      maximumDegree = degreeOfNodesVector[ index ];
    }
  }

  this->m_HistogramVector.resize( maximumDegree + 1 );

  for( int index( 0 ); index < m_HistogramVector.size(); index++ )
  {
    this->m_HistogramVector[ index ] = 0;
  }
  this->m_TopValue = maximumDegree;

  for( int index( 0 ); index < degreeOfNodesVector.size(); index++ )
  {
    this->m_HistogramVector[ degreeOfNodesVector[ index ] ]++;

  }
  // successfully created a valid histogram
  this->m_Valid = true;
}
