#include "mitkPlaneGeometry.h"
#include <vtkTransform.h>

//##ModelId=3E395F22035A
mitk::PlaneGeometry::PlaneGeometry() : 
  m_ScaleFactorMMPerUnitX(1.0), 
  m_ScaleFactorMMPerUnitY(1.0)
{
  Geometry2D::SetWidthInUnits(10);
  Geometry2D::SetHeightInUnits(10);
}


//##ModelId=3E395F220382
mitk::PlaneGeometry::~PlaneGeometry()
{
}

//##ModelId=3E395E3E0077
const mitk::PlaneView& mitk::PlaneGeometry::GetPlaneView() const
{
    return m_PlaneView;
}

//##ModelId=3E396ABE0385
void mitk::PlaneGeometry::SetPlaneView(const mitk::PlaneView& aPlaneView)
{
    m_PlaneView=aPlaneView;

    m_WidthInUnits = (unsigned int)m_PlaneView.getOrientation1().length();
    m_HeightInUnits = (unsigned int)m_PlaneView.getOrientation2().length();

    m_ScaleFactorMMPerUnitX=m_PlaneView.getOrientation1().length()/m_WidthInUnits;
    m_ScaleFactorMMPerUnitY=m_PlaneView.getOrientation2().length()/m_HeightInUnits;

    Modified();
}

//##ModelId=3E3B9C6E02B5
bool mitk::PlaneGeometry::Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const
{
    return m_PlaneView.map(pt3d_mm, pt2d_mm);
}

//##ModelId=3E3B9C7101BF
void mitk::PlaneGeometry::Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const
{
    m_PlaneView.map(pt2d_mm, pt3d_mm);
}

//##ModelId=3E3B9C730262
void mitk::PlaneGeometry::UnitsToMM(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const
{
    pt_mm.x=m_ScaleFactorMMPerUnitX*pt_units.x;
    pt_mm.y=m_ScaleFactorMMPerUnitY*pt_units.y;
}

//##ModelId=3E3B9C760112
void mitk::PlaneGeometry::MMToUnits(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const
{
    pt_units.x=pt_mm.x*(1.0/m_ScaleFactorMMPerUnitX);
    pt_units.y=pt_mm.y*(1.0/m_ScaleFactorMMPerUnitY);
}

//##ModelId=3E3B9C8C0145
void mitk::PlaneGeometry::UnitsToMM(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const
{
    vec_mm.x=m_ScaleFactorMMPerUnitX*vec_units.x;
    vec_mm.y=m_ScaleFactorMMPerUnitY*vec_units.y;
}

//##ModelId=3E3B9C8E0152
void mitk::PlaneGeometry::MMToUnits(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const
{
    vec_units.x=vec_mm.x*(1.0/m_ScaleFactorMMPerUnitX);
    vec_units.y=vec_mm.y*(1.0/m_ScaleFactorMMPerUnitY);
}

//##ModelId=3ED91D060363
void mitk::PlaneGeometry::TransformGeometry(const vtkTransform * transform)
{
	float p[3], n[3];
	// the const_casts in the following are safe, since the used TransformNormalAtPoint/TransformPoint methods
	// do not change anything
	vec2vtk(m_PlaneView.normal, n);
    vec2vtk(m_PlaneView.point, p); 
    const_cast<vtkTransform *>(transform)->TransformNormalAtPoint(p, n, n); vtk2vec(n, m_PlaneView.normal);
	const_cast<vtkTransform *>(transform)->TransformPoint(p, p); vtk2vec(p, m_PlaneView.point);
}
//##ModelId=3EF492640343
bool mitk::PlaneGeometry::Map(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const
{
    return m_PlaneView.map(vec3d_mm, vec2d_mm);
}

//##ModelId=3EF49267006C
void mitk::PlaneGeometry::Map(const mitk::Point2D & atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const
{
    m_PlaneView.map(vec2d_mm, vec3d_mm);
}

