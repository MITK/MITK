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


#include "mitkParametricSpline.h"

#include "mitkOperation.h"
#include "mitkXMLWriter.h"
#include "mitkXMLReader.h"
#include "mitkRenderingManager.h"

#include <vtkCardinalSpline.h>

#include <itkSmartPointerForwardReference.txx>

namespace mitk
{

ParametricSpline
::ParametricSpline()
: m_NumberOfPoints( 0 ),
  m_NumberOfTimeSteps( 0 ),
  m_Closed( false )
{
  m_Initialized = false;

  // Initialize listener for point set modified events
  m_PointSetUpdatedCommand = PointSetUpdatedCommand::New();
  m_PointSetUpdatedCommand->SetCallbackFunction( 
    this, &ParametricSpline::GenerateSplines );

  this->InitializeTimeSlicedGeometry( 1 );
}


ParametricSpline
::~ParametricSpline()
{
  int i;
  for ( i = 0; i < m_SplinesX.size(); ++i )
  {
    if ( m_SplinesX[i] != NULL )
    {
      m_SplinesX[i]->Delete();
    }
    if ( m_SplinesY[i] != NULL )
    {
      m_SplinesY[i]->Delete();
    }
    if ( m_SplinesZ[i] != NULL )
    {
      m_SplinesZ[i]->Delete();
    }
  }
}


ParametricSpline::PointType
ParametricSpline
::Evaluate( ScalarType t, unsigned int timeStep )
{
  Point3D point;
  point[0] = m_SplinesX[timeStep]->Evaluate( t );
  point[1] = m_SplinesY[timeStep]->Evaluate( t );
  point[2] = m_SplinesZ[timeStep]->Evaluate( t );
  return point;
}


ScalarType 
ParametricSpline
::GetRangeMin( unsigned int timeStep ) const
{
  return m_TStart[timeStep];
}


ScalarType 
ParametricSpline
::GetRangeMax( unsigned int timeStep ) const
{
  return m_TEnd[timeStep];
}


bool
ParametricSpline
::IsClosed() const
{
  return m_Closed;
}


bool
ParametricSpline
::IsValidCurve( int timeStep ) const
{
  mitk::PointSet::DataType::Pointer pointSet = 
    m_PointSet->GetPointSet( timeStep );
  if ( pointSet.IsNotNull() && (pointSet->GetNumberOfPoints() >= 3) )
  {
    return true;
  }
  else
  {
    return false;
  }
}


void
ParametricSpline
::SetPointSet( PointSet::Pointer pointSet )
{
  // Add listener for PointSet events: call GenerateSplines when PointSet 
  // changes  
  pointSet->AddObserver( mitk::PointSetEvent(), m_PointSetUpdatedCommand );

  m_PointSet = pointSet;

  this->GenerateSplines();
}


mitk::PointSet::Pointer
ParametricSpline
::GetPointSet()
{
  return m_PointSet;
}


void
ParametricSpline
::GenerateSplines()
{
  if ( m_PointSet.IsNull() )
  {
    return;
  }

  // Initialize the spline functions for each time step (if necessary)
  unsigned int newNumberOfTimeSteps = m_PointSet->GetPointSetSeriesSize();
  if ( m_NumberOfTimeSteps != newNumberOfTimeSteps )
  {
    m_NumberOfTimeSteps = newNumberOfTimeSteps;

    m_SplinesX.resize( m_NumberOfTimeSteps, NULL );
    m_SplinesY.resize( m_NumberOfTimeSteps, NULL );
    m_SplinesZ.resize( m_NumberOfTimeSteps, NULL );

    m_TStart.resize( m_NumberOfTimeSteps );
    m_TEnd.resize( m_NumberOfTimeSteps );

    unsigned int i;
    for ( i = 0; i < m_NumberOfTimeSteps; ++i )
    {
      m_SplinesX[i] = vtkCardinalSpline::New();
      m_SplinesY[i] = vtkCardinalSpline::New();
      m_SplinesZ[i] = vtkCardinalSpline::New();
    }
  }

  // Generate splines from PointSet for each time step
  if ( m_Closed )
  {
    this->GenerateClosedSplines();
  }
  else
  {
    this->GenerateNonClosedSplines();
  }

  this->Modified();
}


void
ParametricSpline
::GenerateClosedSplines()
{
  unsigned int timeStep;
  for ( timeStep = 0; timeStep < m_NumberOfTimeSteps; ++timeStep )
  {
    // Remove points from previous call of this method
    m_SplinesX[timeStep]->RemoveAllPoints();
    m_SplinesY[timeStep]->RemoveAllPoints();
    m_SplinesZ[timeStep]->RemoveAllPoints();

    PointSet::DataType::Pointer pointSet = m_PointSet->GetPointSet( timeStep );
    PointSet::DataType::PointsContainer::Pointer points = pointSet->GetPoints();

    // Start iteration over points at iterator position End() -3
    PointSet::PointsConstIterator it = points->End(); 
    --it; --it; --it;

    PointSet::PointsConstIterator end = points->End();

    int numberOfPoints = pointSet->GetNumberOfPoints();
    if ( numberOfPoints >= 3 )
    {
      ScalarType t = 0.0;

      int i;
      for ( i = -3; i < numberOfPoints + 3; ++i )
      {
        if ( i == 0 ) { m_TStart[timeStep] = t; }
        if ( i == numberOfPoints ) { m_TEnd[timeStep] = t; }

        PointSet::PointType point = it.Value();
        m_SplinesX[timeStep]->AddPoint( t, point[0] );
        m_SplinesY[timeStep]->AddPoint( t, point[1] );
        m_SplinesZ[timeStep]->AddPoint( t, point[2] );

        ++it;
        if ( it == end ) { it = points->Begin(); }
        t += point.EuclideanDistanceTo( it.Value() );
      }
    }
  }
}


void
ParametricSpline
::GenerateNonClosedSplines()
{
  unsigned int timeStep;
  for ( timeStep = 0; timeStep < m_NumberOfTimeSteps; ++timeStep )
  {
    // Remove points from previous call of this method
    m_SplinesX[timeStep]->RemoveAllPoints();
    m_SplinesY[timeStep]->RemoveAllPoints();
    m_SplinesZ[timeStep]->RemoveAllPoints();

    PointSet::DataType::Pointer pointSet = m_PointSet->GetPointSet( timeStep );
    PointSet::DataType::PointsContainer::Pointer points = pointSet->GetPoints();

    unsigned int numberOfPoints = pointSet->GetNumberOfPoints();
    if ( numberOfPoints >= 3 )
    {
      PointSet::PointsConstIterator it = points->Begin();
      PointSet::PointsConstIterator end = points->End();

      ScalarType t = 0.0;
      m_TStart[timeStep] = t;
      while( it != end )
      {
        PointSet::PointType point = it.Value();
        m_SplinesX[timeStep]->AddPoint( t, point[0] );
        m_SplinesY[timeStep]->AddPoint( t, point[1] );
        m_SplinesZ[timeStep]->AddPoint( t, point[2] );

        ++it;
        if ( it == end )
        {
          m_TEnd[timeStep] = t;
        }
        t += point.EuclideanDistanceTo( it.Value() );
      }
    }
  }
}


unsigned int
ParametricSpline
::GetNumberOfPoints() const
{
  return m_NumberOfPoints;
}


void 
ParametricSpline
::InitializeTimeSlicedGeometry( unsigned int timeSteps )
{
  mitk::TimeSlicedGeometry::Pointer timeGeometry = this->GetTimeSlicedGeometry();

  mitk::Geometry3D::Pointer g3d = mitk::Geometry3D::New();
  g3d->Initialize();

  if ( timeSteps > 1 )
  {
    mitk::ScalarType timeBounds[] = {0.0, 1.0};
    g3d->SetTimeBounds( timeBounds );
  }

  //
  // The geometry is propagated automatically to the other items,
  // if EvenlyTimed is true...
  //
  timeGeometry->InitializeEvenlyTimed( g3d.GetPointer(), timeSteps );

  m_Initialized = (timeSteps>0);
}


void 
ParametricSpline
::ExecuteOperation( Operation * /*operation*/ )
{
  // Empty by default. override if needed!
}


void 
ParametricSpline
::UpdateOutputInformation()
{
  // Use methode of BaseData; override in sub-classes
  Superclass::UpdateOutputInformation();
}


void 
ParametricSpline
::SetRequestedRegionToLargestPossibleRegion()
{
  // does not apply
}


bool 
ParametricSpline
::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  // does not apply
  return false;
}


bool 
ParametricSpline
::VerifyRequestedRegion()
{
  // does not apply
  return true;
}


void 
ParametricSpline
::SetRequestedRegion( itk::DataObject * )
{
  // does not apply
}


bool 
ParametricSpline
::WriteXMLData( XMLWriter &xmlWriter )
{
  // to be implemented by sub-classes
  return false;
}


bool 
ParametricSpline
::ReadXMLData( XMLReader &xmlReader )
{
  // to be implemented by sub-classes
  return false;
}

} // namespace mitk
