#include "mitkAbstractTransformGeometry.h"
#include <vtkTransform.h>
#include <vtkAbstractTransform.h>

//##ModelId=3EF4A266029C
mitk::AbstractTransformGeometry::AbstractTransformGeometry() : m_Plane(NULL)
{
  m_ItkVtkAbstractTransform = itk::VtkAbstractTransform<ScalarType>::New();
  m_ParametricTransform = m_ItkVtkAbstractTransform;

  Initialize();
}


//##ModelId=3EF4A266029D
mitk::AbstractTransformGeometry::~AbstractTransformGeometry()
{
}

//##ModelId=3EF4A2660237
vtkAbstractTransform* mitk::AbstractTransformGeometry::GetVtkAbstractTransform() const
{
  return m_ItkVtkAbstractTransform->GetVtkAbstractTransform();
}

//##ModelId=3EF4A2660239
void mitk::AbstractTransformGeometry::SetVtkAbstractTransform(vtkAbstractTransform* aVtkAbstractTransform)
{
  m_ItkVtkAbstractTransform->SetVtkAbstractTransform(aVtkAbstractTransform);
}

//##ModelId=3EF4A2660243
void mitk::AbstractTransformGeometry::SetPlane(const mitk::PlaneGeometry* aPlane)
{
  if(aPlane!=NULL)
  {
    m_Plane = dynamic_cast<mitk::PlaneGeometry*>(aPlane->Clone().GetPointer());

    SetParametricBounds(m_Plane->GetBoundingBox()->GetBounds());

    //@warning affine-transforms and bounding-box should be set by specific sub-classes!
    SetBounds(m_Plane->GetBoundingBox()->GetBounds());
  }
  else
  {
    if(m_Plane==NULL)
      return;
    m_Plane=NULL;
  }
  Modified();
}

//##ModelId=3EF4A2660256
bool mitk::AbstractTransformGeometry::Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const
{
  assert((m_ItkVtkAbstractTransform.IsNotNull()) && (m_Plane.IsNotNull()));

  Point3D pt3d_units;
  pt3d_units = m_ItkVtkAbstractTransform->BackTransform(pt3d_mm);

  return m_Plane->Map(pt3d_units, pt2d_mm);
}

//##ModelId=3EF4A266025F
void mitk::AbstractTransformGeometry::Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const
{
  assert((m_ItkVtkAbstractTransform.IsNotNull()) && (m_Plane.IsNotNull()));

  m_Plane->Map(pt2d_mm, pt3d_mm);
  pt3d_mm = m_ItkVtkAbstractTransform->TransformPoint(pt3d_mm);
}

//##ModelId=3EF4A2660288
bool mitk::AbstractTransformGeometry::Map(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const
{
  assert((m_ItkVtkAbstractTransform.IsNotNull()) && (m_Plane.IsNotNull()));

  float vtkpt[3], vtkvec[3];
  itk2vtk(atPt3d_mm, vtkpt);
  itk2vtk(vec3d_mm, vtkvec);
  m_ItkVtkAbstractTransform->GetInverseVtkAbstractTransform()->TransformVectorAtPoint(vtkpt, vtkvec, vtkvec);
  mitk::Vector3D tmp;
  vtk2vnl(vtkvec, tmp.Get_vnl_vector());
  return m_Plane->Map(atPt3d_mm, tmp, vec2d_mm);
}

//##ModelId=3EF4A2660292
void mitk::AbstractTransformGeometry::Map(const mitk::Point2D & atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const
{
  m_Plane->Map(atPt2d_mm, vec2d_mm, vec3d_mm);
  Point3D atPt3d_mm;
  Map(atPt2d_mm, atPt3d_mm);
  float vtkpt[3], vtkvec[3];
  itk2vtk(atPt3d_mm, vtkpt);
  itk2vtk(vec3d_mm, vtkvec);
  m_ItkVtkAbstractTransform->GetVtkAbstractTransform()->TransformVectorAtPoint(vtkpt, vtkvec, vtkvec);
  vtk2vnl(vtkvec, vec3d_mm.Get_vnl_vector());
}

unsigned long mitk::AbstractTransformGeometry::GetMTime() const
{
  if(Superclass::GetMTime()<m_ItkVtkAbstractTransform->GetMTime())
    return m_ItkVtkAbstractTransform->GetMTime();

  return Superclass::GetMTime();
}

mitk::AffineGeometryFrame3D::Pointer mitk::AbstractTransformGeometry::Clone() const
{
  Self::Pointer newGeometry = Self::New();
  newGeometry->Initialize();
  InitializeGeometry(newGeometry);
  return newGeometry.GetPointer();
}

void mitk::AbstractTransformGeometry::InitializeGeometry(Self * newGeometry) const
{
  Superclass::InitializeGeometry(newGeometry);
  newGeometry->SetPlane(m_Plane);
}
