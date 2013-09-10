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

#include "mitkConnectomicsNetworkConverter.h"

mitk::ConnectomicsNetworkConverter::ConnectomicsNetworkConverter()
{
}

mitk::ConnectomicsNetworkConverter::~ConnectomicsNetworkConverter()
{
}

vnl_matrix<double> mitk::ConnectomicsNetworkConverter::GetNetworkAsVNLAdjacencyMatrix()
{
  int numberOfNodes = boost::num_vertices( *(m_Network->GetBoostGraph()) );

  //initialize a matrix N by N
  vnl_matrix<double> matrix(numberOfNodes, numberOfNodes, 0);

  //Define the vertex iterators
  VertexIteratorType vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices( *(m_Network->GetBoostGraph()) ); vi != vi_end; ++vi)
  {
    //Define the neighbor iterator
    AdjacencyIteratorType neighbors, neighbors_end;
    for(boost::tie(neighbors, neighbors_end) = boost::adjacent_vertices(*vi, *(m_Network->GetBoostGraph()) );
      neighbors != neighbors_end; ++neighbors )
    {
        //Set the i,j element of the matrix to 1.
        matrix.put(*vi, *neighbors, 1);
    }
  }
  return matrix;
}

vnl_matrix<double> mitk::ConnectomicsNetworkConverter::GetNetworkAsVNLDegreeMatrix()
{
  int numberOfNodes = boost::num_vertices( *(m_Network->GetBoostGraph()) );

  vnl_matrix<double> matrix( numberOfNodes, numberOfNodes, 0);

  //Define the vertex iterators
  VertexIteratorType vi, vi_end;
  for(boost::tie(vi, vi_end) = boost::vertices( *(m_Network->GetBoostGraph()) ); vi != vi_end; ++vi)
  {
    double numberOfNeighbors = 0;
    //Define the neighbor iterator
    AdjacencyIteratorType neighbors, neighbors_end;
    for( boost::tie(neighbors, neighbors_end) = adjacent_vertices(*vi, *(m_Network->GetBoostGraph()) );
      neighbors != neighbors_end; ++neighbors )
    {
      numberOfNeighbors++;
    }
    matrix.put(*vi, *vi, numberOfNeighbors);
  }
  return matrix;
}
