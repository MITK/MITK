#include "mitkGeometry2D.h"

//##ModelId=3DDE65E00122
/*!
  \todo implement the method! 
*/
bool mitk::Geometry2D::Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const
{
    return false;
}

//##ModelId=3DDE65E301DE
/*!
  \todo implement the method! 
*/
void mitk::Geometry2D::Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const
{
}

//##ModelId=3DE7895602F7
/*!
  \todo implement the method! 
*/
void mitk::Geometry2D::UnitsToMM(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const
{
}

//##ModelId=3DE7895C01CE
/*!
  \todo implement the method! 
*/
void mitk::Geometry2D::MMToUnits(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const
{
}

//##ModelId=3E395E0802E6
mitk::Geometry2D::Geometry2D() : m_WidthInUnits(1), m_HeightInUnits(1)
{
}

//##ModelId=3E395E080318
mitk::Geometry2D::~Geometry2D()
{
}

//##ModelId=3E3B98C5019F
/*!
  \todo implement the method! 
*/
void mitk::Geometry2D::UnitsToMM(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const
{
}

//##ModelId=3E3B98C9019B
/*!
  \todo implement the method! 
*/
void mitk::Geometry2D::MMToUnits(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const
{
}

void mitk::Geometry2D::SetSizeInUnits(unsigned int width, unsigned int height)
{
    m_WidthInUnits  = width;
    m_HeightInUnits = height;
    Modified();
}

//##ModelId=3EF48EA10320
bool mitk::Geometry2D::Project(const mitk::Point3D &pt3d_mm, mitk::Point3D &projectedPt3d_mm) const
{
    Point2D tmp;
    bool result=Map(pt3d_mm, tmp);
    Map(tmp, projectedPt3d_mm);
    return result;
 }

//##ModelId=3EF48F170280
/*!
  \todo implement the method! 
*/
bool mitk::Geometry2D::Map(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const
{
    return false;
}

//##ModelId=3EF48F2E00D4
/*!
  \todo implement the method! 
*/
void mitk::Geometry2D::Map(const mitk::Point2D & atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const
{
}

//##ModelId=3EF48F8F01B0
bool mitk::Geometry2D::Project(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector3D &projectedVec3d_mm) const
{
    Vector2D tmp;
    Point2D point2d;
    bool result=Map(atPt3d_mm, vec3d_mm, tmp);
    Map(atPt3d_mm, point2d);
    Map(point2d, tmp, projectedVec3d_mm);
    return result;
}

