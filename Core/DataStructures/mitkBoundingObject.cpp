#include "mitkBoundingObject.h"
#include "vtkTransform.h"
#include "mitkVector.h"

mitk::BoundingObject::BoundingObject()
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
