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


#include "mitkAbstractTransformGeometry.h"
#include <vtkAbstractTransform.h>

mitk::AbstractTransformGeometry::AbstractTransformGeometry() : m_Plane(NULL), m_FrameGeometry(NULL)
{
  Initialize();
}

mitk::AbstractTransformGeometry::AbstractTransformGeometry(const AbstractTransformGeometry& other) : Superclass(other)
{
  if(other.m_ParametricBoundingBox.IsNotNull())
  {
    this->SetParametricBounds(m_ParametricBoundingBox->GetBounds());
  }

  this->SetPlane(other.m_Plane);

  this->SetFrameGeometry(other.m_FrameGeometry);
}


mitk::AbstractTransformGeometry::~AbstractTransformGeometry()
{
}

void mitk::AbstractTransformGeometry::Initialize()
{
  Superclass::Initialize();

  m_ItkVtkAbstractTransform = itk::VtkAbstractTransform<ScalarType>::New();
}

vtkAbstractTransform* mitk::AbstractTransformGeometry::GetVtkAbstractTransform() const
{
  return m_ItkVtkAbstractTransform->GetVtkAbstractTransform();
}

mitk::ScalarType mitk::AbstractTransformGeometry::GetParametricExtentInMM(int direction) const
{
  if(m_Plane.IsNull())
  {
    itkExceptionMacro(<<"m_Plane is NULL.");
  }
  return m_Plane->GetExtentInMM(direction);
}

const mitk::Transform3D* mitk::AbstractTransformGeometry::GetParametricTransform() const
{
  return m_ItkVtkAbstractTransform;
}

bool mitk::AbstractTransformGeometry::Project(const mitk::Point3D &pt3d_mm, mitk::Point3D &projectedPt3d_mm) const
{
  assert(m_BoundingBox.IsNotNull());

  mitk::Point2D pt2d_mm;
  bool isInside;
  isInside = Map(pt3d_mm, pt2d_mm);
  Map(pt2d_mm, projectedPt3d_mm);
  return isInside;
  //Point3D pt3d_units;
  //pt3d_units = m_ItkVtkAbstractTransform->BackTransform(pt3d_mm);
  //pt3d_units[2] = 0;
  //projectedPt3d_mm = m_ItkVtkAbstractTransform->TransformPoint(pt3d_units);
  //return const_cast<BoundingBox*>(m_BoundingBox.GetPointer())->IsInside(pt3d_units);
}

bool mitk::AbstractTransformGeometry::Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const
{
  assert((m_ItkVtkAbstractTransform.IsNotNull()) && (m_Plane.IsNotNull()));

  Point3D pt3d_units;
  pt3d_units = m_ItkVtkAbstractTransform->BackTransform(pt3d_mm);

  return m_Plane->Map(pt3d_units, pt2d_mm);
}

void mitk::AbstractTransformGeometry::Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const
{
  assert((m_ItkVtkAbstractTransform.IsNotNull()) && (m_Plane.IsNotNull()));

  m_Plane->Map(pt2d_mm, pt3d_mm);
  pt3d_mm = m_ItkVtkAbstractTransform->TransformPoint(pt3d_mm);
}

bool mitk::AbstractTransformGeometry::Project(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector3D &projectedVec3d_mm) const
{
  itkExceptionMacro("not implemented yet - replace GetIndexToWorldTransform by m_ItkVtkAbstractTransform->GetInverseVtkAbstractTransform()");
  assert(m_BoundingBox.IsNotNull());

  Vector3D vec3d_units;
  vec3d_units = GetIndexToWorldTransform()->BackTransform(vec3d_mm);
  vec3d_units[2] = 0;
  projectedVec3d_mm = GetIndexToWorldTransform()->TransformVector(vec3d_units);

  Point3D pt3d_units;
  pt3d_units = GetIndexToWorldTransform()->BackTransformPoint(atPt3d_mm);
  return const_cast<BoundingBox*>(m_BoundingBox.GetPointer())->IsInside(pt3d_units);
}

bool mitk::AbstractTransformGeometry::Map(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const
{
  assert((m_ItkVtkAbstractTransform.IsNotNull()) && (m_Plane.IsNotNull()));

  float vtkpt[3], vtkvec[3];
  itk2vtk(atPt3d_mm, vtkpt);
  itk2vtk(vec3d_mm, vtkvec);
  m_ItkVtkAbstractTransform->GetInverseVtkAbstractTransform()->TransformVectorAtPoint(vtkpt, vtkvec, vtkvec);
  mitk::Vector3D vec3d_units;
  vtk2itk(vtkvec, vec3d_units);
  return m_Plane->Map(atPt3d_mm, vec3d_units, vec2d_mm);
}

void mitk::AbstractTransformGeometry::Map(const mitk::Point2D & atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const
{
  m_Plane->Map(atPt2d_mm, vec2d_mm, vec3d_mm);
  Point3D atPt3d_mm;
  Map(atPt2d_mm, atPt3d_mm);
  float vtkpt[3], vtkvec[3];
  itk2vtk(atPt3d_mm, vtkpt);
  itk2vtk(vec3d_mm, vtkvec);
  m_ItkVtkAbstractTransform->GetVtkAbstractTransform()->TransformVectorAtPoint(vtkpt, vtkvec, vtkvec);
  vtk2itk(vtkvec, vec3d_mm);
}

void mitk::AbstractTransformGeometry::IndexToWorld(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const
{
  m_Plane->IndexToWorld(pt_units, pt_mm);
}

void mitk::AbstractTransformGeometry::WorldToIndex(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const
{
  m_Plane->WorldToIndex(pt_mm, pt_units);
}

void mitk::AbstractTransformGeometry::IndexToWorld(const mitk::Point2D & /*atPt2d_units*/, const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const
{
  MITK_WARN<<"Warning! Call of the deprecated function AbstractTransformGeometry::IndexToWorld(point, vec, vec). Use AbstractTransformGeometry::IndexToWorld(vec, vec) instead!";
  this->IndexToWorld(vec_units, vec_mm);
}

void mitk::AbstractTransformGeometry::IndexToWorld(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const
{
  m_Plane->IndexToWorld(vec_units, vec_mm);
}

void mitk::AbstractTransformGeometry::WorldToIndex(const mitk::Point2D & /*atPt2d_mm*/, const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const
{
  MITK_WARN<<"Warning! Call of the deprecated function AbstractTransformGeometry::WorldToIndex(point, vec, vec). Use AbstractTransformGeometry::WorldToIndex(vec, vec) instead!";
  this->WorldToIndex(vec_mm, vec_units);
}

void mitk::AbstractTransformGeometry::WorldToIndex(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const
{
  m_Plane->WorldToIndex(vec_mm, vec_units);
}


bool mitk::AbstractTransformGeometry::IsAbove(const mitk::Point3D& pt3d_mm) const
{
  assert((m_ItkVtkAbstractTransform.IsNotNull()) && (m_Plane.IsNotNull()));

  Point3D pt3d_ParametricWorld;
  pt3d_ParametricWorld = m_ItkVtkAbstractTransform->BackTransform(pt3d_mm);

  Point3D pt3d_ParametricUnits;
  ((Geometry3D*)m_Plane)->WorldToIndex(pt3d_ParametricWorld, pt3d_ParametricUnits);

  return (pt3d_ParametricUnits[2] > m_ParametricBoundingBox->GetBounds()[4]);
}

void mitk::AbstractTransformGeometry::SetVtkAbstractTransform(vtkAbstractTransform* aVtkAbstractTransform)
{
  m_ItkVtkAbstractTransform->SetVtkAbstractTransform(aVtkAbstractTransform);
}

void mitk::AbstractTransformGeometry::SetPlane(const mitk::PlaneGeometry* aPlane)
{
  if(aPlane!=NULL)
  {
    m_Plane = static_cast<mitk::PlaneGeometry*>(aPlane->Clone().GetPointer());

    BoundingBox::BoundsArrayType b=m_Plane->GetBoundingBox()->GetBounds();

    SetParametricBounds(b);

    CalculateFrameGeometry();
  }
  else
  {
    if(m_Plane.IsNull())
      return;
    m_Plane=NULL;
  }
  Modified();
}

void mitk::AbstractTransformGeometry::CalculateFrameGeometry()
{
  if((m_Plane.IsNull()) || (m_FrameGeometry.IsNotNull()))
    return;
  //@warning affine-transforms and bounding-box should be set by specific sub-classes!
  SetBounds(m_Plane->GetBoundingBox()->GetBounds());
}

void mitk::AbstractTransformGeometry::SetFrameGeometry(const mitk::Geometry3D* frameGeometry)
{
  if((frameGeometry != NULL) && (frameGeometry->IsValid()))
  {
    m_FrameGeometry = static_cast<mitk::Geometry3D*>(frameGeometry->Clone().GetPointer());
    SetIndexToWorldTransform(m_FrameGeometry->GetIndexToWorldTransform());
    SetBounds(m_FrameGeometry->GetBounds());
  }
  else
  {
    m_FrameGeometry = NULL;
  }
}

unsigned long mitk::AbstractTransformGeometry::GetMTime() const
{
  if(Superclass::GetMTime()<m_ItkVtkAbstractTransform->GetMTime())
    return m_ItkVtkAbstractTransform->GetMTime();

  return Superclass::GetMTime();
}

void mitk::AbstractTransformGeometry::SetOversampling(float oversampling)
{
  if(m_Plane.IsNull())
  {
    itkExceptionMacro(<< "m_Plane is not set.");
  }

  mitk::BoundingBox::BoundsArrayType bounds = m_Plane->GetBounds();
  bounds[1]*=oversampling; bounds[3]*=oversampling; bounds[5]*=oversampling;
  SetParametricBounds(bounds);
}

mitk::AffineGeometryFrame3D::Pointer mitk::AbstractTransformGeometry::Clone() const
{
  Self::Pointer newGeometry = new AbstractTransformGeometry(*this);
  newGeometry->UnRegister();
  return newGeometry.GetPointer();
}
