#include "mitkBoundingObjectGroup.h"
#include "vtkTransform.h"

mitk::BoundingObjectGroup::BoundingObjectGroup()
:  m_Counter(0), m_CSGMode(Difference) //m_CSGMode(Union) m_CSGMode(Intersection)
{
  m_Geometry3D->Initialize();
  m_BoundingObjects = mitk::BoundingObjectGroup::BoundingObjectContainer::New();
}

mitk::BoundingObjectGroup::~BoundingObjectGroup()
{
} 

void mitk::BoundingObjectGroup::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::BoundingObjectGroup::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return ! VerifyRequestedRegion();
}

bool mitk::BoundingObjectGroup::VerifyRequestedRegion()
{
  assert(m_Geometry3D.IsNotNull());
  return true;
}

void mitk::BoundingObjectGroup::SetRequestedRegion(itk::DataObject *data)
{
}

void mitk::BoundingObjectGroup::UpdateOutputInformation()
{  
  float bounds[6]={0,1,0,1,0,1};  // {xmin,x_max, ymin,y_max,zmin,z_max};
  
  // calculate global bounding box
  mitk::BoundingObjectGroup::BoundingObjectContainer::ConstIterator boundingObjectsIterator = m_BoundingObjects->Begin();
  const mitk::BoundingObjectGroup::BoundingObjectContainer::ConstIterator boundingObjectsIteratorEnd = m_BoundingObjects->End();

  if(boundingObjectsIterator == boundingObjectsIteratorEnd) // if there is no BoundingObject, the bounding box is zero
  {
    bounds[0] = bounds[1] = bounds[2] = bounds[3] = bounds[4] = bounds[5] = 0.0;
    return; 
  }
  mitk::BoundingBox::Pointer boundingBox;
  mitk::BoundingBox::PointType minPoint;
  mitk::BoundingBox::PointType maxPoint;
  mitk::BoundingBox::PointType globalMinPoint;  
  mitk::BoundingBox::PointType globalMaxPoint;
  float* posPoint;

  //initialize globalMin/MaxPoints
  boundingObjectsIterator.Value()->UpdateOutputInformation();
  boundingBox = const_cast<mitk::BoundingBox*>(boundingObjectsIterator.Value()->GetGeometry()->GetBoundingBox());
  minPoint = boundingBox->GetMinimum();
  maxPoint = boundingBox->GetMaximum();
  posPoint = boundingObjectsIterator.Value()->GetGeometry()->GetVtkTransform()->GetPosition();

  globalMinPoint[0] = posPoint[0] + minPoint[0];
  globalMinPoint[1] = posPoint[1] + minPoint[1];
  globalMinPoint[2] = posPoint[2] + minPoint[2];
  globalMaxPoint[0] = posPoint[0] + maxPoint[0];
  globalMaxPoint[1] = posPoint[1] + maxPoint[1];
  globalMaxPoint[2] = posPoint[2] + maxPoint[2];
  std::cout << "local Boundingbox 1: " << globalMinPoint[0] << ", " << globalMaxPoint[0] << ", " << globalMinPoint[1] << ", " << globalMaxPoint[1] << ", " << globalMinPoint[2] << ", " << globalMaxPoint[2] << ".\n";
  boundingObjectsIterator++;  
  while (boundingObjectsIterator != boundingObjectsIteratorEnd) // calculate a bounding box that includes all BoundingObjects
  {  
    boundingObjectsIterator.Value()->UpdateOutputInformation();    
    boundingBox = const_cast<mitk::BoundingBox*>(boundingObjectsIterator.Value()->GetGeometry()->GetBoundingBox());
    minPoint = boundingBox->GetMinimum();
    maxPoint = boundingBox->GetMaximum();
    posPoint = boundingObjectsIterator.Value()->GetGeometry()->GetVtkTransform()->GetPosition();

    minPoint[0] = posPoint[0] + minPoint[0];
    minPoint[1] = posPoint[1] + minPoint[1];
    minPoint[2] = posPoint[2] + minPoint[2];
    maxPoint[0] = posPoint[0] + maxPoint[0];
    maxPoint[1] = posPoint[1] + maxPoint[1];
    maxPoint[2] = posPoint[2] + maxPoint[2];

    globalMinPoint[0] = (minPoint[0] < globalMinPoint[0]) ? minPoint[0] : globalMinPoint[0];
    globalMinPoint[1] = (minPoint[1] < globalMinPoint[1]) ? minPoint[1] : globalMinPoint[1];
    globalMinPoint[2] = (minPoint[2] < globalMinPoint[2]) ? minPoint[2] : globalMinPoint[2];

    globalMaxPoint[0] = (maxPoint[0] > globalMaxPoint[0]) ? maxPoint[0] : globalMaxPoint[0];
    globalMaxPoint[1] = (maxPoint[1] > globalMaxPoint[1]) ? maxPoint[1] : globalMaxPoint[1];
    globalMaxPoint[2] = (maxPoint[2] > globalMaxPoint[2]) ? maxPoint[2] : globalMaxPoint[2];
    boundingObjectsIterator++;
  }
  mitk::BoundingBox::PointType centerPoint;
  centerPoint[0] = (globalMinPoint[0] + globalMaxPoint[0])/ 2.0;
  centerPoint[1] = (globalMinPoint[1] + globalMaxPoint[1])/ 2.0;
  centerPoint[2] = (globalMinPoint[2] + globalMaxPoint[2])/ 2.0;

  bounds[0] = globalMinPoint[0] - centerPoint[0]; // x Min
  bounds[1] = globalMaxPoint[0] - centerPoint[0]; // x Max
  bounds[2] = globalMinPoint[1] - centerPoint[1]; // y Min
  bounds[3] = globalMaxPoint[1] - centerPoint[1]; // y Max
  bounds[4] = globalMinPoint[2] - centerPoint[2]; // z Min
  bounds[5] = globalMaxPoint[2] - centerPoint[2]; // z Max

  m_Geometry3D->GetVtkTransform()->Identity();
  m_Geometry3D->GetVtkTransform()->Translate((globalMinPoint[0] + globalMaxPoint[0])/ 2.0, (globalMinPoint[1] + globalMaxPoint[1])/ 2.0, (globalMinPoint[2] + globalMaxPoint[2])/ 2.0);
  m_Geometry3D->SetBoundingBox(bounds);
}

void mitk::BoundingObjectGroup::AddBoundingObject(mitk::BoundingObject::Pointer boundingObject)
{
  m_BoundingObjects->InsertElement( m_Counter++, boundingObject);
  UpdateOutputInformation();
}

bool mitk::BoundingObjectGroup::IsInside(mitk::ITKPoint3D p)
{
  mitk::BoundingObjectGroup::BoundingObjectContainer::ConstIterator boundingObjectsIterator = m_BoundingObjects->Begin();
  const mitk::BoundingObjectGroup::BoundingObjectContainer::ConstIterator boundingObjectsIteratorEnd = m_BoundingObjects->End();
  
  bool inside; // initialize with true for intersection, with false for union
  
  switch(m_CSGMode)
  {
    case Intersection:
    {
      inside = true;
      while (boundingObjectsIterator != boundingObjectsIteratorEnd) // check each BoundingObject
      {       // calculate intersection: each point, that is inside each BoundingObject is considered inside the group
        inside = boundingObjectsIterator.Value()->IsInside(p) && inside;
        if (!inside) // shortcut, it is enough to find one object that does not contain the point
          break;
        boundingObjectsIterator++;
      }
      break;
    }
    case Difference:
    {
      bool posInside = false;
      bool negInside = false;
      while (boundingObjectsIterator != boundingObjectsIteratorEnd) // check each BoundingObject
      {
        // calculate union: each point, that is inside least one BoundingObject is considered inside the group        
        if (boundingObjectsIterator.Value()->GetPositive())
          posInside = boundingObjectsIterator.Value()->IsInside(p) || posInside;
        else
          negInside = boundingObjectsIterator.Value()->IsInside(p) || negInside;
        
        boundingObjectsIterator++;
      }
      if (posInside && !negInside)
        inside = true;
      else
        inside = false;
      break;
    }
    case Union:
    default:
    {
      inside = false;
      while (boundingObjectsIterator != boundingObjectsIteratorEnd) // check each BoundingObject
      {       // calculate union: each point, that is inside least one BoundingObject is considered inside the group
        inside = boundingObjectsIterator.Value()->IsInside(p) || inside;  
        if (inside) // shortcut, it is enough to find one object that contains the point
          break;
        boundingObjectsIterator++;
      }
      break;
    }
  }
  return inside;
}

unsigned int mitk::BoundingObjectGroup::GetCount() const
{
  return m_Counter;
}
