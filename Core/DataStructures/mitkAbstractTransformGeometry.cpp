#include "mitkAbstractTransformGeometry.h"
#include <vtkTransform.h>
#include <vtkAbstractTransform.h>

//##ModelId=3EF4A266029C
mitk::AbstractTransformGeometry::AbstractTransformGeometry() : 
  m_ScaleFactorMMPerUnitX(1.0), m_ScaleFactorMMPerUnitY(1.0), 
  m_VtkAbstractTransform(NULL), m_InverseVtkAbstractTransform(NULL), 
  m_LastVtkAbstractTransformTimeStamp(0)
{
  Geometry2D::SetWidthInUnits(10);
  Geometry2D::SetHeightInUnits(10);
}


//##ModelId=3EF4A266029D
mitk::AbstractTransformGeometry::~AbstractTransformGeometry()
{
}

//##ModelId=3EF4A2660237
vtkAbstractTransform* mitk::AbstractTransformGeometry::GetVtkAbstractTransform() const
{
  return m_VtkAbstractTransform;
}

//##ModelId=3EF4A2660239
void mitk::AbstractTransformGeometry::SetVtkAbstractTransform(vtkAbstractTransform* aVtkAbstractTransform)
{
  if(m_VtkAbstractTransform==aVtkAbstractTransform)
    return;

  if(m_VtkAbstractTransform!=NULL)
    m_VtkAbstractTransform->UnRegister(NULL);

  if(m_InverseVtkAbstractTransform!=NULL)
    m_InverseVtkAbstractTransform->UnRegister(NULL);

  m_VtkAbstractTransform=aVtkAbstractTransform;
  if(m_VtkAbstractTransform!=NULL)
  {
    m_VtkAbstractTransform->Register(NULL);
    m_InverseVtkAbstractTransform=m_VtkAbstractTransform->GetInverse();
  }

  m_LastVtkAbstractTransformTimeStamp = m_VtkAbstractTransform->GetMTime();

  Modified();
}

//##ModelId=3EF4A2660241
const mitk::PlaneView& mitk::AbstractTransformGeometry::GetPlaneView() const
{
  return m_PlaneView;
}

//##ModelId=3EF4A2660243
void mitk::AbstractTransformGeometry::SetPlaneView(const mitk::PlaneView& aPlaneView)
{
  m_PlaneView=aPlaneView;

  m_WidthInUnits = m_PlaneView.getOrientation1().length();
  m_HeightInUnits = m_PlaneView.getOrientation2().length();

  m_ScaleFactorMMPerUnitX=m_PlaneView.getOrientation1().length()/m_WidthInUnits;
  m_ScaleFactorMMPerUnitY=m_PlaneView.getOrientation2().length()/m_HeightInUnits;

  Modified();
}

//##ModelId=3EF4A2660256
bool mitk::AbstractTransformGeometry::Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const
{
  if(m_InverseVtkAbstractTransform==NULL) return false;

  float vtkpt[3];
  vec2vtk(pt3d_mm, vtkpt);
  m_InverseVtkAbstractTransform->TransformPoint(vtkpt, vtkpt);
  mitk::Point3D tmp;
  vtk2vec(vtkpt, tmp);
  return m_PlaneView.map(tmp, pt2d_mm);
}

//##ModelId=3EF4A266025F
void mitk::AbstractTransformGeometry::Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const
{
  m_PlaneView.map(pt2d_mm, pt3d_mm);
  float vtkpt[3];
  vec2vtk(pt3d_mm, vtkpt);
  m_VtkAbstractTransform->TransformPoint(vtkpt, vtkpt);
  vtk2vec(vtkpt, pt3d_mm);
}

//##ModelId=3EF4A2660263
void mitk::AbstractTransformGeometry::UnitsToMM(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const
{
  pt_mm.x=m_ScaleFactorMMPerUnitX*pt_units.x;
  pt_mm.y=m_ScaleFactorMMPerUnitY*pt_units.y;
}

//##ModelId=3EF4A266026C
void mitk::AbstractTransformGeometry::MMToUnits(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const
{
  pt_units.x=pt_mm.x*(1.0/m_ScaleFactorMMPerUnitX);
  pt_units.y=pt_mm.y*(1.0/m_ScaleFactorMMPerUnitY);
}

//##ModelId=3EF4A2660275
void mitk::AbstractTransformGeometry::UnitsToMM(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const
{
  vec_mm.x=m_ScaleFactorMMPerUnitX*vec_units.x;
  vec_mm.y=m_ScaleFactorMMPerUnitY*vec_units.y;
}

//##ModelId=3EF4A266027E
void mitk::AbstractTransformGeometry::MMToUnits(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const
{
  vec_units.x=vec_mm.x*(1.0/m_ScaleFactorMMPerUnitX);
  vec_units.y=vec_mm.y*(1.0/m_ScaleFactorMMPerUnitY);
}

//##ModelId=3EF4A266024C
void mitk::AbstractTransformGeometry::TransformGeometry(const vtkTransform * transform)
{
  itkWarningMacro("Sorry, transform of an AbstractTransformGeometry not yet supported.");
}

//##ModelId=3EF4A2660288
bool mitk::AbstractTransformGeometry::Map(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const
{
  if(m_InverseVtkAbstractTransform==NULL) return false;

  float vtkpt[3], vtkvec[3];
  vec2vtk(atPt3d_mm, vtkpt);
  vec2vtk(vec3d_mm, vtkvec);
  m_InverseVtkAbstractTransform->TransformVectorAtPoint(vtkpt, vtkvec, vtkvec);
  mitk::Vector3D tmp;
  vtk2vec(vtkvec, tmp);
  return m_PlaneView.map(tmp, vec2d_mm);
}

//##ModelId=3EF4A2660292
void mitk::AbstractTransformGeometry::Map(const mitk::Point2D & atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const
{
  m_PlaneView.map(vec2d_mm, vec3d_mm);
  Point3D atPt3d_mm;
  Map(atPt2d_mm, atPt3d_mm);
  float vtkpt[3], vtkvec[3];
  vec2vtk(atPt3d_mm, vtkpt);
  vec2vtk(vec3d_mm, vtkvec);
  m_VtkAbstractTransform->TransformVectorAtPoint(vtkpt, vtkvec, vtkvec);
  vtk2vec(vtkvec, vec3d_mm);
}

unsigned long mitk::AbstractTransformGeometry::GetMTime() const
{
  if(m_LastVtkAbstractTransformTimeStamp<m_VtkAbstractTransform->GetMTime())
  {
    m_LastVtkAbstractTransformTimeStamp=m_VtkAbstractTransform->GetMTime();
    Modified();
  }

  return Superclass::GetMTime();
}
