
/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 11989 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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