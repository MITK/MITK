#include "Geometry3D.h"
#include "PlaneGeometry.h"

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
    pt_units.set(pt_mm.x/m_Resolution.x, pt_mm.y/m_Resolution.y, pt_mm.z/m_Resolution.z);
}

//##ModelId=3DDE65DC0151
void mitk::Geometry3D::PointUnitsToMM(const Point3f &pt_units, Point3f &pt_mm, float t) const
{
    pt_mm.set(pt_units.x*m_Resolution.x, pt_units.y*m_Resolution.y, pt_units.z*m_Resolution.z);
}

//##ModelId=3DCBC65C017C
const double *mitk::Geometry3D::GetSpacing() const
{
	itkExceptionMacro("Spacing not yet supported."); 	
	return NULL;
}

//##ModelId=3E15578402BD
bool mitk::Geometry3D::SetGeometry2D(mitk::Geometry2D* geometry2D, int s, int t)
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

    m_Geometry2Ds.clear();

    Geometry2D::Pointer gnull=NULL;
    int num=m_LargestPossibleRegion.GetSize(2)*m_LargestPossibleRegion.GetSize(3);

    m_Geometry2Ds.reserve(num);
    m_Geometry2Ds.assign(num, gnull);
    
    m_Resolution.set(1.0, 1.0, 1.0);

    //construct standard view
    Point3f o1, o2; bool rightHanded=true;
	Vector2f viewport(0,0);
	o1.set(m_LargestPossibleRegion.GetSize(0),0,0); PointUnitsToMM(o1, o1);
    o2.set(0,m_LargestPossibleRegion.GetSize(1),0); PointUnitsToMM(o2, o2);
    PlaneView view_std(Point3f(0,0,0), o1, o2, viewport, viewport, rightHanded);

    mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();
    m_Geometry2Ds[0]=planegeometry;
    planegeometry->SetPlaneView(view_std);
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

