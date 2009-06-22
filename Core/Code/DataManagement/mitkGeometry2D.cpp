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


#include "mitkGeometry2D.h"
#include <vtkTransform.h>


mitk::Geometry2D::Geometry2D()
  : m_ScaleFactorMMPerUnitX( 1.0 ), 
    m_ScaleFactorMMPerUnitY( 1.0 ),
    m_ReferenceGeometry( NULL )
{
}


mitk::Geometry2D::~Geometry2D()
{
}


void 
mitk::Geometry2D::SetIndexToWorldTransform(
  mitk::AffineTransform3D* transform)
{
  Superclass::SetIndexToWorldTransform(transform);
  
  m_ScaleFactorMMPerUnitX=GetExtentInMM(0)/GetExtent(0);
  m_ScaleFactorMMPerUnitY=GetExtentInMM(1)/GetExtent(1);  

  assert(m_ScaleFactorMMPerUnitX<ScalarTypeNumericTraits::infinity());
  assert(m_ScaleFactorMMPerUnitY<ScalarTypeNumericTraits::infinity());
}


void 
mitk::Geometry2D::SetExtentInMM(int direction, ScalarType extentInMM)
{
  Superclass::SetExtentInMM(direction, extentInMM);

  m_ScaleFactorMMPerUnitX=GetExtentInMM(0)/GetExtent(0);
  m_ScaleFactorMMPerUnitY=GetExtentInMM(1)/GetExtent(1);  

  assert(m_ScaleFactorMMPerUnitX<ScalarTypeNumericTraits::infinity());
  assert(m_ScaleFactorMMPerUnitY<ScalarTypeNumericTraits::infinity());
}


bool 
mitk::Geometry2D::Map(
  const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const
{
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();

  Point3D pt3d_units;
  BackTransform(pt3d_mm, pt3d_units);
  pt2d_mm[0]=pt3d_units[0]*m_ScaleFactorMMPerUnitX;
  pt2d_mm[1]=pt3d_units[1]*m_ScaleFactorMMPerUnitY;
  pt3d_units[2]=0;
  return const_cast<BoundingBox*>(m_BoundingBox.GetPointer())->IsInside(pt3d_units);
}


void 
mitk::Geometry2D::Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const
{
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();

  Point3D pt3d_units;
  pt3d_units[0]=pt2d_mm[0]/m_ScaleFactorMMPerUnitX;
  pt3d_units[1]=pt2d_mm[1]/m_ScaleFactorMMPerUnitY;
  pt3d_units[2]=0;
  pt3d_mm = GetParametricTransform()->TransformPoint(pt3d_units);
}


void 
mitk::Geometry2D::IndexToWorld(
  const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const
{
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();
  mitk::Point3D pt3d;
  FillVector3D(pt3d, pt_units[0], pt_units[1], 0);
  pt3d = GetParametricTransform()->TransformPoint(pt3d);
  pt_mm[0]=pt3d[0];
  pt_mm[1]=pt3d[1];
}


void 
mitk::Geometry2D::WorldToIndex(
  const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const
{
  itkExceptionMacro(<< "No BackTransform in itk::Transform ==> no general" \
    " WorldToIndex(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units)" \
    " possible. Has to be implemented in sub-class.");
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();
  pt_units[0]=pt_mm[0]/m_ScaleFactorMMPerUnitX;
  pt_units[1]=pt_mm[1]/m_ScaleFactorMMPerUnitY;
}


void 
mitk::Geometry2D::IndexToWorld(const mitk::Point2D &/*atPt2d_units*/, 
  const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const
{
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();
  mitk::Vector3D vec3d;
  FillVector3D(vec3d, vec_units[0], vec_units[1], 0);
  vec3d = GetParametricTransform()->TransformVector(vec3d);
  vec_mm[0]=vec3d[0];
  vec_mm[1]=vec3d[1];
}


void 
mitk::Geometry2D::WorldToIndex(const mitk::Point2D &/*atPt2d_mm*/,
  const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const
{
  itkExceptionMacro(<< "No BackTransform in itk::Transform ==> no general" \
    " WorldToIndex(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units)" \
    " possible. Has to be implemented in sub-class.");
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();
  vec_units[0]=vec_mm[0]/m_ScaleFactorMMPerUnitX;
  vec_units[1]=vec_mm[1]/m_ScaleFactorMMPerUnitY;
}

void 
mitk::Geometry2D::SetSizeInUnits(mitk::ScalarType width, mitk::ScalarType height)
{
  ScalarType bounds[6]={0, width, 0, height, 0, 1};
  ScalarType extent, newextentInMM;
  if(GetExtent(0)>0)
  {
    extent = GetExtent(0);
    if(width>extent)
      newextentInMM = GetExtentInMM(0)/width*extent;
    else
      newextentInMM = GetExtentInMM(0)*extent/width;
    SetExtentInMM(0, newextentInMM);
  }
  if(GetExtent(1)>0)
  {
    extent = GetExtent(1);
    if(width>extent)
      newextentInMM = GetExtentInMM(1)/height*extent;
    else
      newextentInMM = GetExtentInMM(1)*extent/height;
    SetExtentInMM(1, newextentInMM);
  }
  SetBounds(bounds);
}


bool 
mitk::Geometry2D::Project(
  const mitk::Point3D &pt3d_mm, mitk::Point3D &projectedPt3d_mm) const
{
  assert(m_BoundingBox.IsNotNull());

  Point3D pt3d_units;
  BackTransform(pt3d_mm, pt3d_units);
  pt3d_units[2] = 0;
  projectedPt3d_mm = GetParametricTransform()->TransformPoint(pt3d_units);
  return const_cast<BoundingBox*>(m_BoundingBox.GetPointer())->IsInside(pt3d_units);
}


bool 
mitk::Geometry2D::Map(const mitk::Point3D & atPt3d_mm,
  const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const
{
  Point2D pt2d_mm_start, pt2d_mm_end;
  Point3D pt3d_mm_end;
  bool inside=Map(atPt3d_mm, pt2d_mm_start);
  pt3d_mm_end = atPt3d_mm+vec3d_mm;
  inside&=Map(pt3d_mm_end, pt2d_mm_end);
  vec2d_mm=pt2d_mm_end-pt2d_mm_start;
  return inside;
}


void 
mitk::Geometry2D::Map(const mitk::Point2D &/*atPt2d_mm*/,
  const mitk::Vector2D &/*vec2d_mm*/, mitk::Vector3D &/*vec3d_mm*/) const
{
  //@todo implement parallel to the other Map method!
  assert(false);
}


bool 
mitk::Geometry2D::Project(const mitk::Point3D & atPt3d_mm,
  const mitk::Vector3D &vec3d_mm, mitk::Vector3D &projectedVec3d_mm) const
{
  assert(m_BoundingBox.IsNotNull());

  Vector3D vec3d_units;
  BackTransform(atPt3d_mm, vec3d_mm, vec3d_units);
  vec3d_units[2] = 0;
  projectedVec3d_mm = GetParametricTransform()->TransformVector(vec3d_units);

  Point3D pt3d_units;
  BackTransform(atPt3d_mm, pt3d_units);
  return const_cast<BoundingBox*>(m_BoundingBox.GetPointer())->IsInside(pt3d_units);
}


mitk::ScalarType 
mitk::Geometry2D::SignedDistance(const mitk::Point3D& pt3d_mm) const
{
  Point3D projectedPoint;
  Project(pt3d_mm, projectedPoint);
  Vector3D direction = pt3d_mm-projectedPoint;
  ScalarType distance = direction.GetNorm();

  if(IsAbove(pt3d_mm) == false)
    distance*=-1.0;

  return distance;
}

bool 
mitk::Geometry2D::IsAbove(const mitk::Point3D& pt3d_mm) const
{
  Point3D pt3d_units;
  Geometry3D::WorldToIndex(pt3d_mm, pt3d_units);
  return (pt3d_units[2] > m_BoundingBox->GetBounds()[4]);
}

mitk::AffineGeometryFrame3D::Pointer 
mitk::Geometry2D::Clone() const
{
  Self::Pointer newGeometry = Self::New();
  newGeometry->Initialize();
  InitializeGeometry(newGeometry);
  return newGeometry.GetPointer();
}


void 
mitk::Geometry2D::InitializeGeometry(Self * newGeometry) const
{
  Superclass::InitializeGeometry(newGeometry);

  newGeometry->SetReferenceGeometry( m_ReferenceGeometry );
}

void 
mitk::Geometry2D::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  os << indent << " ScaleFactorMMPerUnitX: "
     << m_ScaleFactorMMPerUnitX << std::endl;
  os << indent << " ScaleFactorMMPerUnitY: "
     << m_ScaleFactorMMPerUnitY << std::endl;
}

void
mitk::Geometry2D::SetReferenceGeometry( mitk::Geometry3D *geometry )
{
  m_ReferenceGeometry = geometry;
}

mitk::Geometry3D *
mitk::Geometry2D::GetReferenceGeometry() const
{
  return m_ReferenceGeometry;
}

bool
mitk::Geometry2D::HasReferenceGeometry() const
{
  return ( m_ReferenceGeometry != NULL );
}
