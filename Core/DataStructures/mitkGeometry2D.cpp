#include "Geometry2D.h"

//##ModelId=3DDE65E00122
void mitk::Geometry2D::Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const
{
}

//##ModelId=3DDE65E301DE
void mitk::Geometry2D::Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const
{
}

//##ModelId=3DE7895602F7
void mitk::Geometry2D::UnitsToMM(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const
{
}

//##ModelId=3DE7895C01CE
void mitk::Geometry2D::MMToUnits(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const
{
}

//##ModelId=3DE78B870238
itk::Transform<float,3,2>::Pointer mitk::Geometry2D::GetTransfrom() const
{
	itkExceptionMacro("Transform not yet supported.");
    return NULL;
}

//##ModelId=3E02DFAF02AD
double mitk::Geometry2D::GetTime() const
{
	return -1;
}

//##ModelId=3E395E0802E6
mitk::Geometry2D::Geometry2D(unsigned int width_units, unsigned int height_units) : m_WidthInUnits(width_units), m_HeightInUnits(height_units)
{
}


//##ModelId=3E395E080318
mitk::Geometry2D::~Geometry2D()
{
}

//##ModelId=3E3B98C5019F
void mitk::Geometry2D::UnitsToMM(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const
{
}

//##ModelId=3E3B98C9019B
void mitk::Geometry2D::MMToUnits(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const
{
}

void mitk::Geometry2D::SetSizeInUnits(unsigned int width, unsigned int height)
{
    m_WidthInUnits  = width;
    m_HeightInUnits = height;
    Modified();
}
