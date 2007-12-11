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

#include <mitkRingSpline.h>

#include <mitkInteractionConst.h>
#include <mitkGeometryData.h>
#include <mitkOperation.h>



mitk::RingSpline::RingSpline()
{ 
  m_PlaneFit = mitk::PlaneFit::New();
  m_PlaneFit->SetInput(this); 
}


mitk::RingSpline::~RingSpline(){}


void mitk::RingSpline::DoSortPoints()
{ 
  //std::cout<<"RingSpline::SortPointSet() - stand by "<<std::endl;

  mitk::PointSet::MeshType::Pointer mesh = this->GetPointSet();
  mesh->Update();
  
  if ( this->GetSize() < 3 ) return;

  //start calculation
  m_PlaneFit->Update();
  
  mitk::Geometry2D* geometry2D = dynamic_cast<mitk::Geometry2D*>( m_PlaneFit->GetOutput()->GetGeometry());
  
  // sorting of points with rotating vector in plane geometry
  const mitk::Point3D &centroid = m_PlaneFit->GetCentroid();
  
  typedef std::map<float,mitk::Point3D> SortedPointMapType;
  SortedPointMapType sortedPointMap;
  
  // Project each point on the calculated plane and calculate the angles of
  // each projected point from the starting point, taking the centroid as
  // origin.
  mitk::PointSet::PointsContainer::Iterator pit, end;
  pit = mesh->GetPoints()->Begin();

  mitk::Vector2D v0, v1;
  geometry2D->Map( centroid, pit.Value() - centroid, v0 );

  float alpha, alpha0 = atan2( v0[0], v0[1] );

  sortedPointMap[0.0] = pit.Value();

  end = mesh->GetPoints()->End();
  for ( ++pit; pit != end; pit++ )
  {
    // Map 3D point onto 2D plane
    geometry2D->Map( centroid, pit.Value() - centroid, v1 );

    alpha = atan2( v1[0], v1[1] ) - alpha0;
    sortedPointMap[alpha]= pit.Value();
  }

  // remove points
  mesh->GetPoints()->Initialize();
  mesh->GetCells()->Initialize();
  
  int i;
  SortedPointMapType::iterator it;
  for( it = sortedPointMap.begin(), i = 0; it != sortedPointMap.end(); it++, i++ )
  {
    mesh->SetPoint(i,it->second);
  }

  CellAutoPointer polygon;
  polygon.TakeOwnership( new PolygonCellType );

  int size = (signed) sortedPointMap.size();
  for ( i = 0; i < size; ++i )
  {
    polygon->SetPointId( i, i );
  }

  std::cout<<"RingSpline::SortPointSet() - finished "<<std::endl;

  mesh->SetCell( 0, polygon );
  
}

