#include "mitkGeometry2DData.h"
#include <itkProcessObject.h>

//##ModelId=3E639CD30201
mitk::Geometry2DData::Geometry2DData()
{
}

//##ModelId=3E639CD30233
mitk::Geometry2DData::~Geometry2DData()
{
}

//##ModelId=3E6423D2030E
void mitk::Geometry2DData::SetGeometry2D( const mitk::Geometry2D *geometry2d, int t )
{
    m_Geometry2D=geometry2d;
    Modified();
}

//##ModelId=3E66CC5A0295
void mitk::Geometry2DData::UpdateOutputInformation()
{
    SetPipelineMTime(m_Geometry2D->GetMTime());
	if (this->GetSource())
	{
		this->GetSource()->UpdateOutputInformation();
	}
}

//##ModelId=3E66CC5A02B4
void mitk::Geometry2DData::SetRequestedRegionToLargestPossibleRegion()
{

}

//##ModelId=3E66CC5A02D2
bool mitk::Geometry2DData::RequestedRegionIsOutsideOfTheBufferedRegion()
{
 	if(VerifyRequestedRegion()==false) return true;

    if(m_Geometry2D==NULL) return true;

	return false;
}

//##ModelId=3E66CC5A02F0
bool mitk::Geometry2DData::VerifyRequestedRegion()
{
    if(m_Geometry2D==NULL) return false;

    return true;
}

//##ModelId=3E66CC5A030E
void mitk::Geometry2DData::SetRequestedRegion(itk::DataObject *data)
{

}

//##ModelId=3E67D85E00B7
void mitk::Geometry2DData::CopyInformation(const itk::DataObject *data)
{
}

