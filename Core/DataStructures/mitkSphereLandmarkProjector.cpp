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


#include "mitkSphereLandmarkProjector.h"
#include <vtkThinPlateSplineTransform.h>

#include <vtkTransform.h>
#include <vtkSphericalTransform.h>
#include <vtkGeneralTransform.h>

mitk::SphereLandmarkProjector::SphereLandmarkProjector()
{
  m_SphericalTransform        = vtkSphericalTransform::New();
  m_SphereRotation            = vtkTransform::New();
  m_SpatialPlacementTransform = vtkTransform::New();
  m_PlaneToSphericalTransform = vtkGeneralTransform::New();

  m_SphereRotation->RotateX(90);

  //setup parameter-plane of the sphere: x is phi, y is theta; the radius is always 1
  mitk::ScalarType origin[3] = {1,            0,  2*vnl_math::pi};  //  (1,   0, 6.28)  (1,   0,    0)
  mitk::ScalarType right[3]  = {0,            0, -2*vnl_math::pi};  //  (0,3.14,    0)  (0,   0, 6.28)
  mitk::ScalarType bottom[3] = {0, vnl_math::pi,               0};  //  (0,   0,-6.28)  (0,3.14,    0)

  m_SphereParameterPlane = mitk::PlaneGeometry::New();
  m_SphereParameterPlane->InitializeStandardPlane(right, bottom);
  m_SphereParameterPlane->SetOrigin(origin);
  m_SphereParameterPlane->SetSizeInUnits(100, 50);

  m_ParameterPlane = m_SphereParameterPlane;
}

mitk::SphereLandmarkProjector::~SphereLandmarkProjector()
{
  m_SphericalTransform->Delete();
  m_SphereRotation->Delete();
  m_SpatialPlacementTransform->Delete();
  m_PlaneToSphericalTransform->Delete();
}

void mitk::SphereLandmarkProjector::ComputeCompleteAbstractTransform()
{
  m_PlaneToSphericalTransform->Identity();
  m_PlaneToSphericalTransform->PostMultiply();
  m_PlaneToSphericalTransform->Concatenate(m_SphericalTransform);
  m_PlaneToSphericalTransform->Concatenate(m_SphereRotation);
  m_PlaneToSphericalTransform->Concatenate(m_InterpolatingAbstractTransform);//GetInterpolatingAbstractTransform());
  m_PlaneToSphericalTransform->Concatenate(m_SpatialPlacementTransform);

  m_CompleteAbstractTransform = m_PlaneToSphericalTransform;
}

void mitk::SphereLandmarkProjector::ProjectLandmarks(
  const mitk::PointSet::DataType::PointsContainer* targetLandmarks)
{
  unsigned int size=targetLandmarks->Size();
  mitk::PointSet::DataType::PointsContainer::ConstIterator pointiterator, start = targetLandmarks->Begin();
  mitk::PointSet::DataType::PointsContainer::ElementIdentifier id;

  //Part I: Calculate center of sphere
  mitk::Point3D center;
  center.Fill(0);
  mitk::ScalarType radius;
  mitk::PointSet::PointType point;
  //determine center
  for(id=0, pointiterator=start;id<size;++id, ++pointiterator)
  {
    point = pointiterator->Value();
    center[0]+=point[0];
    center[1]+=point[1];
    center[2]+=point[2];
  }
  center[0]/=(mitk::ScalarType)size;
  center[1]/=(mitk::ScalarType)size;
  center[2]/=(mitk::ScalarType)size;
  //determine radius
  switch(0)
  {
  case 0/*MIN*/:
    radius = mitk::ScalarTypeNumericTraits::max();
    for(id=0, pointiterator=start;id<size;++id, ++pointiterator)
    {
      point = pointiterator->Value();
      mitk::Vector3D v;
      v[0]=point[0]-center[0];
      v[1]=point[1]-center[1];
      v[2]=point[2]-center[2];
      if (v.GetNorm() < radius) radius = v.GetNorm();      
    }
    break;
  case 1/*MAX*/:
    radius = 0;
    for(id=0, pointiterator=start;id<size;++id, ++pointiterator)
    {
      point = pointiterator->Value();
      mitk::Vector3D v;
      v[0]=point[0]-center[0];
      v[1]=point[1]-center[1];
      v[2]=point[2]-center[2];
      if (v.GetNorm() > radius) radius = v.GetNorm();      
    }
    break;
  case 2/*AVERAGE*/:
    radius = 0;
    for(id=0, pointiterator=start;id<size;++id, ++pointiterator)
    {
      point = pointiterator->Value();
      mitk::Vector3D v;
      v[0]=point[0]-center[0];
      v[1]=point[1]-center[1];
      v[2]=point[2]-center[2];
      radius += v.GetNorm();
    }
    radius*=1.0/size;
    break;
  }
  mitk::Point3D origin = m_SphereParameterPlane->GetOrigin(); origin[0]=radius; m_SphereParameterPlane->SetOrigin(origin);
  m_SpatialPlacementTransform->GetMatrix()->SetElement(0, 3, center[0]);
  m_SpatialPlacementTransform->GetMatrix()->SetElement(1, 3, center[1]);
  m_SpatialPlacementTransform->GetMatrix()->SetElement(2, 3, center[2]);


  //Part II: Project points on sphere
  mitk::Point3D projectedPoint;

  m_WritableFinalTargetLandmarks->Initialize();
  m_ProjectedLandmarks->Initialize();

  m_WritableFinalTargetLandmarks->Reserve(size);
  m_ProjectedLandmarks->Reserve(size);
  for(id=0, pointiterator=start;id<size;++id, ++pointiterator)
  { 
    point = pointiterator->Value();

    mitk::Vector3D v;
    v=point-center;
    mitk::FillVector3D(point, v[0], v[1], v[2]);
    v.Normalize(); v*=radius;
    mitk::FillVector3D(projectedPoint, v[0], v[1], v[2]);

    m_WritableFinalTargetLandmarks->InsertElement(id, point);
    m_ProjectedLandmarks->InsertElement(id, projectedPoint);
  }
}
