#include "Geometry3D.h"

//##ModelId=3DCBF3AD0110
const unsigned int *mitk::Geometry3D::GetDimensions() const
{
	return m_Dimensions;
}

//##ModelId=3DDE649E00A3
unsigned int mitk::Geometry3D::GetDataDimension() const
{
	return m_Dimension;
}

//##ModelId=3DCBF50C0377
mitk::Geometry2D::Pointer mitk::Geometry3D::GetGeometry2D(int s, int t) const
{
	itkExceptionMacro("GetGeometry2D not yet supported."); 	
	return NULL;
}

//##ModelId=3DCBF5D40253
void mitk::Geometry3D::GetBoundingBox() const
{
	itkExceptionMacro("BoundingBox not yet supported."); 	
}

//##ModelId=3DCBF5E9037F
double mitk::Geometry3D::GetTime(int t) const
{
	itkExceptionMacro("GetTime not yet supported."); 	
	return 0;
}

//##ModelId=3DE763C500C4
mitk::Geometry3D::TransformPointer mitk::Geometry3D::GetTransfrom() const
{
	itkExceptionMacro("Transform not yet supported."); 	
	return NULL;
}

//##ModelId=3DDE65D1028A
void mitk::Geometry3D::PointMMToUnits(const Point3f &pt_mm, Point3f &pt_units, float t) const
{
	itkExceptionMacro("Conversions not yet supported."); 	
}

//##ModelId=3DDE65DC0151
void mitk::Geometry3D::PointUnitsToMM(const Point3f &pt_units, Point3f &pt_mm, float t) const
{
	itkExceptionMacro("Conversions not yet supported."); 	
}

//##ModelId=3DCBC65C017C
const double *mitk::Geometry3D::GetSpacing() const
{
	itkExceptionMacro("Spacing not yet supported."); 	
	return NULL;
}

//##ModelId=3E15578402BD
bool mitk::Geometry3D::SetGeometry2D(mitk::Geometry2D geometry2D, int s, int t)
{
	itkExceptionMacro("SetGeometry2D not yet supported."); 	
	return false;
}

//##ModelId=3E155839024F
bool mitk::Geometry3D::SetGeometry2D(ipPicDescriptor* pic, int s, int t)
{
	itkExceptionMacro("SetGeometry2D not yet supported."); 	
	return false;
}

//##ModelId=3E3453C703AF
void mitk::Geometry3D::Initialize(unsigned int dimension, const unsigned int* dimensions)
{
    m_Dimension=dimension;

    if(m_Dimensions!=NULL)
        delete m_Dimensions;

    m_Dimensions = new unsigned int [m_Dimension];
    memcpy(m_Dimensions,dimensions,m_Dimension*sizeof(*m_Dimensions));

	unsigned int i, max;

	max=m_Dimension<=4?m_Dimension:4;

	for(i=0;i<max;++i)
	{
		m_LargestPossibleRegion.SetIndex(i, 0);
		m_LargestPossibleRegion.SetSize (i, m_Dimensions[i]);
	}
	for(;i<4;++i)
	{
		m_LargestPossibleRegion.SetIndex(i, 0);
		m_LargestPossibleRegion.SetSize (i, 1);
	}
}

//##ModelId=3E15572E0269
mitk::Geometry3D::Geometry3D(unsigned int dimension, const unsigned int* dimensions) : m_Dimension(0), m_Dimensions(NULL)
{
    Initialize(dimension, dimensions);
}

//##ModelId=3E3452F10253
mitk::Geometry3D::Geometry3D(const mitk::Geometry3D& right) : m_Dimensions(NULL)
{
    Initialize(right.GetDataDimension(),right.GetDimensions());
}

//##ModelId=3E3452F10393
// mitk::Geometry3D& mitk::Geometry3D::operator=(const mitk::Geometry3D& right)
// {
//     return Geometry3D(right);
// }

//##ModelId=3E3456C50067
mitk::Geometry3D::~Geometry3D()
{
    if(m_Dimensions!=NULL)
        delete m_Dimensions;
}

