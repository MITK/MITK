#include "Geometry2D.h"

//##ModelId=3DDE65E00122
void mitk::Geometry2D::MapPoint(const Point3f &pt3d, Point2f &pt2d)
{
}

//##ModelId=3DDE65E301DE
void mitk::Geometry2D::MapPoint(const Point2f &pt2d, Point3f &pt3d)
{
}

//##ModelId=3DE76C1D023D
void mitk::Geometry2D::MapVector(const Vector2f &pt2d, Vector3f &pt3d)
{
}

//##ModelId=3DE76C7C02F8
void mitk::Geometry2D::MapVector(const Vector3f &pt3d, Vector2f &pt2d)
{
}

//##ModelId=3DE7895602F7
void mitk::Geometry2D::PointUnitsToMM(const Point2f &pt_units, Point2f &pt_mm)
{
}

//##ModelId=3DE7895C01CE
void mitk::Geometry2D::PointMMToUnits(const Point2f &pt_mm, Point2f &pt_units)
{
}

//##ModelId=3DE78B870238
itk::Transform<float,3,2>::Pointer mitk::Geometry2D::GetTransfrom()
{
	itkExceptionMacro("Transform not yet supported."); 
}

//##ModelId=3E02DFAF02AD
double mitk::Geometry2D::GetTime()
{
	return -1;
}

