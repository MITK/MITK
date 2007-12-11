/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkPlaneFit.h"

#include "mitkPlaneGeometry.h"
#include "mitkGeometryData.h"

#include <vnl/algo/vnl_svd.h>
#include <vcl_iostream.h>


mitk::PlaneFit::PlaneFit()
: m_PointSet( NULL )
{
  m_TimeSlicedGeometry = mitk::TimeSlicedGeometry::New();
}

mitk::PlaneFit::~PlaneFit()
{
}


void mitk::PlaneFit::GenerateOutputInformation()
{
  mitk::PointSet::ConstPointer input  = this->GetInput();
  mitk::GeometryData::Pointer output  = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  if (input.IsNull()) return;

  if ( m_PointSet == NULL )
  {
    return;
  }

  bool update = false;
  if ( output->GetGeometry() == NULL || output->GetTimeSlicedGeometry() == NULL )
    update = true;
  if ( ( ! update ) && ( output->GetTimeSlicedGeometry()->GetTimeSteps() != input->GetTimeSlicedGeometry()->GetTimeSteps() ) )
    update = true;
  if ( update )
  {
    mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
  
    m_TimeSlicedGeometry->InitializeEvenlyTimed( 
      planeGeometry, m_PointSet->GetPointSetSeriesSize() );
  
    unsigned int t;
    for ( t = 0;
          (t < m_PointSet->GetPointSetSeriesSize())
          && (t < m_Planes.size());
          ++t )
    {
      m_TimeSlicedGeometry->SetGeometry3D( m_Planes[t], (int) t );
    }
  
    output->SetGeometry( m_TimeSlicedGeometry );
  }
}

void mitk::PlaneFit::GenerateData()
{
  unsigned int t;
  for ( t = 0; t < m_PointSet->GetPointSetSeriesSize(); ++t )
  {
    // check number of data points - less then 3points isn't enough
    if ( m_PointSet->GetSize( t ) >= 3 )
    {
      this->CalculateCentroid( t );

      this->ProcessPointSet( t );

      this->InitializePlane( t );
    }
  }
}

void mitk::PlaneFit::SetInput( const mitk::PointSet* pointSet )
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, 
    const_cast< mitk::PointSet * >( pointSet ) );

  m_PointSet = pointSet;
  unsigned int pointSetSize = pointSet->GetPointSetSeriesSize();

  m_Planes.resize( pointSetSize );
  m_Centroids.resize( pointSetSize );
  m_PlaneVectors.resize( pointSetSize );

  unsigned int t;
  for ( t = 0; t < pointSetSize; ++t )
  {
    m_Planes[t] = mitk::PlaneGeometry::New();
  }
}

const mitk::PointSet* mitk::PlaneFit::GetInput()
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<const mitk::PointSet * > (this->ProcessObject::GetInput(0) );
}


void mitk::PlaneFit::CalculateCentroid( int t )
{  
  if ( m_PointSet == NULL ) return;

  int ps_total = m_PointSet->GetSize( t );
  
  m_Centroids[t][0] = m_Centroids[t][1] = m_Centroids[t][2] = 0.0;

  // sum of all points
  mitk::PointSet::PointsContainer::Iterator pit, end;
  pit = m_PointSet->GetPointSet( t )->GetPoints()->Begin();
  end = m_PointSet->GetPointSet( t )->GetPoints()->End();
  for ( ; pit!=end; ++pit )
  {
    mitk::Point3D p3d = pit.Value();
    m_Centroids[t][0] += p3d[0]; 
    m_Centroids[t][1] += p3d[1];
    m_Centroids[t][2] += p3d[2];
  }

  // calculation of centroid
  m_Centroids[t][0] /= ps_total;
  m_Centroids[t][1] /= ps_total;
  m_Centroids[t][2] /= ps_total;
}


void mitk::PlaneFit::ProcessPointSet( int t )
{
  if (m_PointSet == NULL ) return;

  // int matrix with POINTS x (X,Y,Z)
  vnl_matrix<mitk::ScalarType> dataM( m_PointSet->GetSize( t ), 3);

  // calculate point distance to centroid and inserting it in the matrix
  mitk::PointSet::PointsContainer::Iterator pit, end;
  pit = m_PointSet->GetPointSet( t )->GetPoints()->Begin();
  end = m_PointSet->GetPointSet( t )->GetPoints()->End();
  
  for (int p=0; pit!=end; pit++, p++)
  {
    mitk::Point3D p3d = pit.Value();
    dataM[p][0] = p3d[0] - m_Centroids[t][0];
    dataM[p][1] = p3d[1] - m_Centroids[t][1];
    dataM[p][2] = p3d[2] - m_Centroids[t][2];
  }

  // process the SVD (singular value decomposition) from ITK
  // the vector will be orderd 	descending 
  vnl_svd<mitk::ScalarType> svd(dataM, 0.0);

  // calculate the SVD of A
  vnl_vector<mitk::ScalarType> v = svd.nullvector();


  // Avoid erratic normal sign switching when the plane changes minimally
  // by negating the vector for negative x values.
  if ( v[0] < 0 )
  {
    v = -v;
  }

  m_PlaneVectors[t][0] = v[0];
  m_PlaneVectors[t][1] = v[1];
  m_PlaneVectors[t][2] = v[2];

}

mitk::PlaneGeometry::Pointer mitk::PlaneFit::GetPlaneGeometry( int t )
{
  return m_Planes[t];
}

const mitk::Vector3D &mitk::PlaneFit::GetPlaneNormal( int t ) const
{
  return m_PlaneVectors[t];
}

const mitk::Point3D &mitk::PlaneFit::GetCentroid( int t ) const
{
  return m_Centroids[t];
}

void mitk::PlaneFit::InitializePlane( int t )
{
  m_Planes[t]->InitializePlane( m_Centroids[t], m_PlaneVectors[t] );
}

