#include "BaseData.h"

mitk::BaseData::BaseData() 
{
	m_PropertyList = PropertyList::New();
}

mitk::BaseData::~BaseData() 
{

}

//##ModelId=3DCBE2BA0139
const mitk::Geometry3D& mitk::BaseData::GetGeometry() const
{
	return *m_Geometry3D.GetPointer();
}

mitk::PropertyList::Pointer mitk::BaseData::GetPropertyList() 
{
	return m_PropertyList;
}

//##ModelId=3E3C4ACB0046
mitk::Geometry2D::ConstPointer mitk::BaseData::GetGeometry2D(int s, int t)
{
    itkWarningMacro("GetGeometry2D not yet completely implemented. Appropriate setting of the update extent is missing.");
 
    SetRequestedRegionToLargestPossibleRegion();

    UpdateOutputInformation();

    return GetGeometry().GetGeometry2D(s,t);
}

