#include "PlaneGeometry.h"

//##ModelId=3E395F22035A
mitk::PlaneGeometry::PlaneGeometry() : Geometry2D(10.0, 10.0), m_ScaleFactorMMPerUnitX(1.0), m_ScaleFactorMMPerUnitY(1.0)
{
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
    m_ScaleFactorMMPerUnitX=m_PlaneView.getOrientation1().length()/m_WidthInUnits;
    m_ScaleFactorMMPerUnitY=m_PlaneView.getOrientation2().length()/m_HeightInUnits;
}

//##ModelId=3E3AEB7C001C
itk::Transform<float,3,2>::Pointer mitk::PlaneGeometry::GetTransfrom() const
{
	itkExceptionMacro("Transform not yet supported."); 	
	return NULL;
}

//##ModelId=3E3B9C6E02B5
void mitk::PlaneGeometry::Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const
{
    m_PlaneView.map(pt3d_mm, pt2d_mm);
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

