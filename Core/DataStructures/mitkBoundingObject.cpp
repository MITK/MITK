#include "mitkBoundingObject.h"
#include "vtkTransform.h"
#include "mitkVector.h"

mitk::BoundingObject::BoundingObject()
  : m_Positive(true)
{
  m_Geometry3D->Initialize();
}

mitk::BoundingObject::~BoundingObject() 
{
} 

void mitk::BoundingObject::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::BoundingObject::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return ! VerifyRequestedRegion();
}

bool mitk::BoundingObject::VerifyRequestedRegion()
{
  assert(m_Geometry3D.IsNotNull());
  return true;
}

void mitk::BoundingObject::SetRequestedRegion(itk::DataObject *data)
{
}

void mitk::BoundingObject::UpdateOutputInformation()
{  
  ScalarType bounds[6]={0,1,0,1,0,1};  //{xmin,x_max, ymin,y_max,zmin,z_max}
  
  // calculate vector from origin (in the center of the cuboid) to each corner
  mitk::ScalarType p[3];
  /* bounding box that is parallel to world axes */
  //p[0] = fabs(m_Geometry3D->GetXAxis()[0]) + fabs(m_Geometry3D->GetYAxis()[0]) + fabs(m_Geometry3D->GetZAxis()[0]);
  //p[1] = fabs(m_Geometry3D->GetXAxis()[1]) + fabs(m_Geometry3D->GetYAxis()[1]) + fabs(m_Geometry3D->GetZAxis()[1]);
  //p[2] = fabs(m_Geometry3D->GetXAxis()[2]) + fabs(m_Geometry3D->GetYAxis()[2]) + fabs(m_Geometry3D->GetZAxis()[2]);
  /* bounding box that is parallel to object axes */
  //p[0] = m_Geometry3D->GetXAxis().GetNorm();
  //p[1] = m_Geometry3D->GetYAxis().GetNorm();
  //p[2] = m_Geometry3D->GetZAxis().GetNorm();
  /* bounding box around the unscaled bounding object */
  p[0] = 1;
  p[1] = 1;
  p[2] = 1;
  bounds[0] = - p[0];
  bounds[1] = + p[0];
  bounds[2] = - p[1];
  bounds[3] = + p[1];
  bounds[4] = - p[2];
  bounds[5] = + p[2];
 
  m_Geometry3D->SetBounds(bounds);
}
mitk::ScalarType mitk::BoundingObject::GetVolume()
{
  return 0.0;
}
