#include "mitkPointData.h"
#include <itkProcessObject.h>
#include <mitkBaseProcess.h>

mitk::PointData::PointData()
{
   m_Point3D[0] = 0;
   m_Point3D[1] = 0;
   m_Point3D[2] = 0;

   m_Point2D[0] = 0;
   m_Point2D[1] = 0;
}

mitk::PointData::~PointData()
{

}


void mitk::PointData::UpdateOutputInformation()
{
    if ( this->GetSource() )
    {
        this->GetSource()->UpdateOutputInformation();
    }
}
  
void  mitk::PointData::SetRequestedRegionToLargestPossibleRegion()
{

}
  
bool  mitk::PointData::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    if ( VerifyRequestedRegion() == false )
        return true;
    else
        return false;
}
  
bool  mitk::PointData::VerifyRequestedRegion()
{
   return true;
}
 
void  mitk::PointData::SetRequestedRegion(itk::DataObject *data)
{

}

void  mitk::PointData::CopyInformation(const itk::DataObject *data)
{
    Superclass::CopyInformation(data);
}

