#include "BaseData.h"

//##ModelId=3DCBE2BA0139
const mitk::Geometry3D& mitk::BaseData::GetGeometry() const
{
	return *m_Geometry3D.GetPointer();
}











