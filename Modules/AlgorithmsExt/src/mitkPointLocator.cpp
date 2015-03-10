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

#include "mitkPointLocator.h"
#include <vtkPointSet.h>
#include <ANN/ANN.h>


mitk::PointLocator::PointLocator() :
  m_SearchTreeInitialized(false),
  m_VtkPoints(NULL), m_MitkPoints(NULL),
  m_ItkPoints(NULL),
  m_ANNK(1), m_ANNDimension(3),
  m_ANNEpsilon(0), m_ANNDataPoints(NULL),
  m_ANNQueryPoint(NULL), m_ANNPointIndexes(NULL),
  m_ANNDistances(NULL), m_ANNTree(NULL)
{

}



mitk::PointLocator::~PointLocator()
{
  if ( m_SearchTreeInitialized )
    DestroyANN();
}



void mitk::PointLocator::SetPoints( vtkPointSet* pointSet )
{
  if ( pointSet == NULL )
  {
     itkWarningMacro("Points are NULL!");
     return;
  }
  vtkPoints* points = pointSet->GetPoints();

  if(m_VtkPoints)
  {
    if ( (m_VtkPoints == points) && (m_VtkPoints->GetMTime() == points->GetMTime()) )
    {
      return; //no need to recalculate search tree
    }
  }
  m_VtkPoints = points;

  size_t size = points->GetNumberOfPoints();
  if ( m_ANNDataPoints != NULL )
    delete[] m_ANNDataPoints;
  m_ANNDataPoints = annAllocPts( size, m_ANNDimension );
  m_IndexToPointIdContainer.clear();
  m_IndexToPointIdContainer.resize( size );
  for( vtkIdType i = 0; (unsigned)i < size; ++i )
  {
    double* currentPoint = points->GetPoint( i );
    (m_ANNDataPoints[i])[0] = currentPoint[0];
    (m_ANNDataPoints[i])[1] = currentPoint[1];
    (m_ANNDataPoints[i])[2] = currentPoint[2];
    m_IndexToPointIdContainer[i] = i;
  }
  InitANN();
}



void mitk::PointLocator::SetPoints( mitk::PointSet* points )
{
  if ( points == NULL )
  {
     itkWarningMacro("Points are NULL!");
     return;
  }

  if(m_MitkPoints)
  {
    if ( (m_MitkPoints == points) && (m_MitkPoints->GetMTime() == points->GetMTime()) )
    {
      return; //no need to recalculate search tree
    }
  }
  m_MitkPoints = points;

  size_t size = points->GetSize();
  if ( m_ANNDataPoints != NULL )
    delete[] m_ANNDataPoints;
  m_ANNDataPoints = annAllocPts( size, m_ANNDimension );
  m_IndexToPointIdContainer.clear();
  m_IndexToPointIdContainer.resize( size );
  size_t counter = 0;
  mitk::PointSet::PointsContainer* pointsContainer = points->GetPointSet()->GetPoints();
  mitk::PointSet::PointsContainer::Iterator it;
  mitk::PointSet::PointType currentPoint;
  mitk::PointSet::PointsContainer::ElementIdentifier currentId;
  for( it = pointsContainer->Begin(); it != pointsContainer->End(); ++it, ++counter )
  {
    currentPoint = it->Value();
    currentId = it->Index();
    (m_ANNDataPoints[counter])[0] = currentPoint[0];
    (m_ANNDataPoints[counter])[1] = currentPoint[1];
    (m_ANNDataPoints[counter])[2] = currentPoint[2];
    m_IndexToPointIdContainer[counter] = currentId;
  }
  InitANN();
}


void mitk::PointLocator::SetPoints( ITKPointSet* pointSet )
{
  if ( pointSet == NULL )
  {
    itkWarningMacro("Points are NULL!");
    return;
  }

  if(m_ItkPoints)
  {
    if ( (m_ItkPoints == pointSet) && (m_ItkPoints->GetMTime() == pointSet->GetMTime()) )
    {
      return; //no need to recalculate search tree
    }
  }
  m_ItkPoints = pointSet;

  size_t size = pointSet->GetNumberOfPoints();
  if ( m_ANNDataPoints != NULL )
    delete[] m_ANNDataPoints;
  m_ANNDataPoints = annAllocPts( size, m_ANNDimension );
  m_IndexToPointIdContainer.clear();
  m_IndexToPointIdContainer.resize( size );
  size_t counter = 0;
  ITKPointSet::PointsContainerConstPointer pointsContainer = pointSet->GetPoints();
  ITKPointSet::PointsContainer::ConstIterator it;
  ITKPointSet::PointType currentPoint;
  ITKPointSet::PointsContainer::ElementIdentifier currentId;
  for( it = pointsContainer->Begin(); it != pointsContainer->End(); ++it, ++counter )
  {
    currentPoint = it->Value();
    currentId = it->Index();
    (m_ANNDataPoints[counter])[0] = currentPoint[0];
    (m_ANNDataPoints[counter])[1] = currentPoint[1];
    (m_ANNDataPoints[counter])[2] = currentPoint[2];
    m_IndexToPointIdContainer[counter] = currentId;
  }
  InitANN();
}



mitk::PointLocator::IdType mitk::PointLocator::FindClosestPoint( const double point[3] )
{
  m_ANNQueryPoint[0] = point[0];
  m_ANNQueryPoint[1] = point[1];
  m_ANNQueryPoint[2] = point[2];
  return FindClosestANNPoint( m_ANNQueryPoint );
}



mitk::PointLocator::IdType mitk::PointLocator::FindClosestPoint( double x, double y, double z )
{
  m_ANNQueryPoint[0] = x;
  m_ANNQueryPoint[1] = y;
  m_ANNQueryPoint[2] = z;
  return FindClosestANNPoint( m_ANNQueryPoint );
}



mitk::PointLocator::IdType mitk::PointLocator::FindClosestPoint( mitk::PointSet::PointType point )
{
  m_ANNQueryPoint[0] = point[0];
  m_ANNQueryPoint[1] = point[1];
  m_ANNQueryPoint[2] = point[2];
  return FindClosestANNPoint( m_ANNQueryPoint );
}

mitk::PointLocator::IdType mitk::PointLocator::FindClosestANNPoint( const ANNpoint& point)
{
  if ( ! m_SearchTreeInitialized )
    return -1;
  m_ANNTree->annkSearch(point, m_ANNK, m_ANNPointIndexes, m_ANNDistances);
  return m_IndexToPointIdContainer[m_ANNPointIndexes[0]];
}

mitk::PointLocator::DistanceType mitk::PointLocator::GetMinimalDistance( mitk::PointSet::PointType point )
{
  m_ANNQueryPoint[0] = point[0];
  m_ANNQueryPoint[1] = point[1];
  m_ANNQueryPoint[2] = point[2];
  return GetMinimalDistance( m_ANNQueryPoint );
}

mitk::PointLocator::DistanceType mitk::PointLocator::GetMinimalDistance( const ANNpoint& point)
{
  if ( ! m_SearchTreeInitialized )
    return -1;
  m_ANNTree->annkSearch(point, m_ANNK, m_ANNPointIndexes, m_ANNDistances);
  return m_ANNDistances[0];
}


void mitk::PointLocator::InitANN()
{
  if ( m_SearchTreeInitialized )
    DestroyANN();

  m_ANNQueryPoint = annAllocPt( m_ANNDimension );
  m_ANNPointIndexes = new ANNidx[m_ANNK];
  m_ANNDistances = new ANNdist[m_ANNK];
  m_ANNTree = new ANNkd_tree( m_ANNDataPoints, m_IndexToPointIdContainer.size(), m_ANNDimension );

  m_SearchTreeInitialized = true;
}



void mitk::PointLocator::DestroyANN()
{
  m_SearchTreeInitialized = false;
  if ( m_ANNQueryPoint != NULL )
    annDeallocPt( m_ANNQueryPoint );
  if ( m_ANNDataPoints != NULL )
    annDeallocPts( m_ANNDataPoints );
  if ( m_ANNPointIndexes != NULL )
    delete[] m_ANNPointIndexes;
  if ( m_ANNDistances != NULL )
    delete[] m_ANNDistances;
  if ( m_ANNTree != NULL )
    delete m_ANNTree;
}

bool mitk::PointLocator::FindClosestPointAndDistance( mitk::PointSet::PointType point, IdType* id, DistanceType* dist )
{
  m_ANNQueryPoint[0] = point[0];
  m_ANNQueryPoint[1] = point[1];
  m_ANNQueryPoint[2] = point[2];

  m_ANNTree->annkSearch( m_ANNQueryPoint, m_ANNK, m_ANNPointIndexes, m_ANNDistances);

  *id = m_IndexToPointIdContainer[m_ANNPointIndexes[0]];
  *dist = m_ANNDistances[0];
  return true;
}
