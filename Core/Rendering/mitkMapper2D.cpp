#include "mitkMapper2D.h"

//##ModelId=3E3C46810341
mitk::Mapper2D::Mapper2D()
{
}


//##ModelId=3E3C4681035F
mitk::Mapper2D::~Mapper2D()
{
}

//##ModelId=3E3C45A0009E
void mitk::Mapper2D::SetGeometry3D(const mitk::Geometry3D* aGeometry3D)
{
    m_Geometry3D = aGeometry3D;
    Modified();
}
