#include "BaseData.h"

//##ModelId=3DCBE2BA0139
const mitk::Geometry3D& mitk::BaseData::GetGeometry()
{
	return *m_Geometry3D.GetPointer();
}











