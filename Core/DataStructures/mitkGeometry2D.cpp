#include "mitkGeometry2D.h"
#include <vtkTransform.h>

//##ModelId=3E395E0802E6
mitk::Geometry2D::Geometry2D()
{
}

//##ModelId=3E395E080318
mitk::Geometry2D::~Geometry2D()
{
}

//##ModelId=3DDE65E00122
bool mitk::Geometry2D::Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const
{
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();

  Point3D pt3d_units;
  pt3d_units = GetUnitsToMMAffineTransform()->BackTransformPoint(pt3d_mm);
  pt2d_mm[0]=pt3d_units[0]*m_UnitsToMMAffineTransform->GetMatrix().GetVnlMatrix().get_column(0).magnitude();
  pt2d_mm[1]=pt3d_units[1]*m_UnitsToMMAffineTransform->GetMatrix().GetVnlMatrix().get_column(1).magnitude();

  return const_cast<BoundingBox*>(m_BoundingBox.GetPointer())->IsInside(pt3d_units);
}

//##ModelId=3DDE65E301DE
void mitk::Geometry2D::Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const
{
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();

  Point3D pt3d_units;
  pt3d_units[0]=pt2d_mm[0]/m_UnitsToMMAffineTransform->GetMatrix().GetVnlMatrix().get_column(0).magnitude();
  pt3d_units[1]=pt2d_mm[1]/m_UnitsToMMAffineTransform->GetMatrix().GetVnlMatrix().get_column(1).magnitude();
  pt3d_units[2]=0;
  pt3d_mm = GetUnitsToMMAffineTransform()->TransformPoint(pt3d_units);
}

//##ModelId=3DE7895602F7
void mitk::Geometry2D::UnitsToMM(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const
{
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();
  mitk::Point3D pt3d;
  FillVector3D(pt3d, pt_units[0], pt_units[1], 0);
  pt3d = GetParametricTransform()->TransformPoint(pt3d);
  pt_mm[0]=pt3d[0];
  pt_mm[1]=pt3d[1];
}

//##ModelId=3DE7895C01CE
void mitk::Geometry2D::MMToUnits(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const
{
  itkExceptionMacro(<< "No BackTransform in itk::Transform ==> no general MMToUnits(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) possible. Has to be implemented in sub-class.");
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();
  pt_units[0]=pt_mm[0]/m_UnitsToMMAffineTransform->GetMatrix().GetVnlMatrix().get_column(0).magnitude();
  pt_units[1]=pt_mm[1]/m_UnitsToMMAffineTransform->GetMatrix().GetVnlMatrix().get_column(1).magnitude();
}

//##ModelId=3E3B98C5019F
void mitk::Geometry2D::UnitsToMM(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const
{
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();
  mitk::Vector3D vec3d;
  FillVector3D(vec3d, vec_units[0], vec_units[1], 0);
  vec3d = GetParametricTransform()->TransformVector(vec3d);
  vec_mm[0]=vec3d[0];
  vec_mm[1]=vec3d[1];
}

//##ModelId=3E3B98C9019B
void mitk::Geometry2D::MMToUnits(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const
{
  itkExceptionMacro(<< "No BackTransform in itk::Transform ==> no general MMToUnits(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) possible. Has to be implemented in sub-class.");
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();
  vec_units[0]=vec_mm[0]/m_UnitsToMMAffineTransform->GetMatrix().GetVnlMatrix().get_column(0).magnitude();
  vec_units[1]=vec_mm[1]/m_UnitsToMMAffineTransform->GetMatrix().GetVnlMatrix().get_column(1).magnitude();
}

void mitk::Geometry2D::SetSizeInUnits(mitk::ScalarType width, mitk::ScalarType height)
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

//##ModelId=3EF48EA10320
bool mitk::Geometry2D::Project(const mitk::Point3D &pt3d_mm, mitk::Point3D &projectedPt3d_mm) const
{
  assert(m_BoundingBox.IsNotNull());

  Point3D pt3d_units;
  pt3d_units = GetUnitsToMMAffineTransform()->BackTransformPoint(pt3d_mm);
  pt3d_units[2] = 0;
  projectedPt3d_mm = GetUnitsToMMAffineTransform()->TransformPoint(pt3d_units);
  return const_cast<BoundingBox*>(m_BoundingBox.GetPointer())->IsInside(pt3d_units);
 }

//##ModelId=3EF48F170280
bool mitk::Geometry2D::Map(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const
{
  Point2D pt2d_mm_start, pt2d_mm_end;
  Point3D pt3d_mm_end;
  bool inside=Map(atPt3d_mm, pt2d_mm_start);
  pt3d_mm_end = atPt3d_mm+vec3d_mm;
  inside&=Map(pt3d_mm_end, pt2d_mm_end);
  vec2d_mm=pt2d_mm_end-pt2d_mm_start;
  return inside;
}

//##ModelId=3EF48F2E00D4
void mitk::Geometry2D::Map(const mitk::Point2D & atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const
{
  //@todo implement parallel to the other Map method!
  assert(false);
}

//##ModelId=3EF48F8F01B0
bool mitk::Geometry2D::Project(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector3D &projectedVec3d_mm) const
{
  assert(m_BoundingBox.IsNotNull());

  Vector3D vec3d_units;
  vec3d_units = GetUnitsToMMAffineTransform()->BackTransform(vec3d_mm);
  vec3d_units[2] = 0;
  projectedVec3d_mm = GetUnitsToMMAffineTransform()->TransformVector(vec3d_units);

  Point3D pt3d_units;
  pt3d_units = GetUnitsToMMAffineTransform()->BackTransformPoint(atPt3d_mm);
  return const_cast<BoundingBox*>(m_BoundingBox.GetPointer())->IsInside(pt3d_units);
}

mitk::AffineGeometryFrame3D::Pointer mitk::Geometry2D::Clone() const
{
  Self::Pointer newGeometry = Self::New();
  newGeometry->Initialize();
  InitializeGeometry(newGeometry);
  return newGeometry.GetPointer();
}

