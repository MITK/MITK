/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkBoundingObjectGroup.h"
#include "vtkTransform.h"

mitk::BoundingObjectGroup::BoundingObjectGroup()
:  m_Counter(0), m_CSGMode(Difference) //m_CSGMode(Union) m_CSGMode(Intersection)
{
  m_Geometry3D->Initialize();
  m_BoundingObjects = mitk::BoundingObjectGroup::BoundingObjectContainer::New();
  SetVtkPolyData(NULL);
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
  ScalarType bounds[6]={0,1,0,1,0,1};  // {xmin,x_max, ymin,y_max,zmin,z_max};
  
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
  mitk::BoundingBox::PointType globalMinPoint;  
  mitk::BoundingBox::PointType globalMaxPoint;

  /* Initialize globalMin/MaxPoints */
  boundingObjectsIterator.Value()->UpdateOutputInformation();
  boundingBox = const_cast<mitk::BoundingBox*>(boundingObjectsIterator.Value()->GetGeometry()->GetBoundingBox());
  
  minPoint = boundingBox->GetMinimum();
  mitk::ScalarType min[3];
  min[0] = minPoint[0];   min[1] = minPoint[1];   min[2] = minPoint[2];
  boundingObjectsIterator.Value()->GetGeometry()->GetVtkTransform()->TransformPoint(min, min);
  globalMinPoint[0] = min[0];   globalMinPoint[1] = min[1];   globalMinPoint[2] = min[2];  
  globalMaxPoint[0] = min[0];   globalMaxPoint[1] = min[1];   globalMaxPoint[2] = min[2];
  //boundingObjectsIterator++;  
  while (boundingObjectsIterator != boundingObjectsIteratorEnd) // calculate a bounding box that includes all BoundingObjects
  {  
    boundingObjectsIterator.Value()->UpdateOutputInformation();    
    boundingBox = const_cast<mitk::BoundingBox*>(boundingObjectsIterator.Value()->GetGeometry()->GetBoundingBox());

    /* create all 8 points of the bounding box */
    mitk::BoundingBox::PointsContainerPointer points = mitk::BoundingBox::PointsContainer::New();
    mitk::Point3D p;
    p = boundingBox->GetMinimum();
    points->InsertElement(0, p);
    p[0] = -p[0];
    points->InsertElement(1, p);
    p = boundingBox->GetMinimum();
    p[1] = -p[1];    
    points->InsertElement(2, p);
    p = boundingBox->GetMinimum();
    p[2] = -p[2];    
    points->InsertElement(3, p);
    p = boundingBox->GetMaximum();
    points->InsertElement(4, p);
    p[0] = -p[0];
    points->InsertElement(5, p);
    p = boundingBox->GetMaximum();
    p[1] = -p[1];    
    points->InsertElement(6, p);
    p = boundingBox->GetMaximum();
    p[2] = -p[2];    
    points->InsertElement(7, p);
    mitk::BoundingBox::PointsContainerConstIterator pointsIterator = points->Begin();
    mitk::BoundingBox::PointsContainerConstIterator pointsIteratorEnd = points->End();
    while (pointsIterator != pointsIteratorEnd) // for each vertex of the bounding box
    {
      minPoint = pointsIterator->Value();
      min[0] = minPoint[0];   min[1] = minPoint[1];   min[2] = minPoint[2];
      boundingObjectsIterator.Value()->GetGeometry()->GetVtkTransform()->TransformPoint(min, min);  // transform vertex point to world coordinates

      globalMinPoint[0] = (min[0] < globalMinPoint[0]) ? min[0] : globalMinPoint[0];  // check if world point
      globalMinPoint[1] = (min[1] < globalMinPoint[1]) ? min[1] : globalMinPoint[1];  // has a lower or a
      globalMinPoint[2] = (min[2] < globalMinPoint[2]) ? min[2] : globalMinPoint[2];  // higher value as
      globalMaxPoint[0] = (min[0] > globalMaxPoint[0]) ? min[0] : globalMaxPoint[0];  // the last known highest
      globalMaxPoint[1] = (min[1] > globalMaxPoint[1]) ? min[1] : globalMaxPoint[1];  // value
      globalMaxPoint[2] = (min[2] > globalMaxPoint[2]) ? min[2] : globalMaxPoint[2];  // in each axis
      pointsIterator++;
    }
    boundingObjectsIterator++;
  }

  /* BoundingBox is centered around the center of all sub bounding objects */
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

  mitk::TimeSlicedGeometry::Pointer timeGeometry = const_cast< mitk::TimeSlicedGeometry *>(this->GetTimeSlicedGeometry());
  mitk::Geometry3D::Pointer geometry3d = timeGeometry->GetGeometry3D(0);
  unsigned int timesteps = timeGeometry->GetTimeSteps();
  if(geometry3d.IsNull())
  {
    assert(timesteps==0);
    timeGeometry->SetBounds(bounds);

    /* the objects position is the center of all sub bounding objects */
    timeGeometry->GetVtkTransform()->Identity();
    timeGeometry->GetVtkTransform()->Translate(centerPoint[0], centerPoint[1], centerPoint[2]);
    timeGeometry->TransferVtkToITKTransform();
  }
  else
  {
    assert(timesteps>0);
    geometry3d->SetBounds(bounds);

    /* the objects position is the center of all sub bounding objects */
    geometry3d->GetVtkTransform()->Identity();
    geometry3d->GetVtkTransform()->Translate(centerPoint[0], centerPoint[1], centerPoint[2]);
    geometry3d->TransferVtkToITKTransform();

    timeGeometry->InitializeEvenlyTimed(geometry3d, timesteps);
  }
}

void mitk::BoundingObjectGroup::AddBoundingObject(mitk::BoundingObject::Pointer boundingObject)
{
  m_BoundingObjects->InsertElement( m_Counter++, boundingObject);
  UpdateOutputInformation();
}

bool mitk::BoundingObjectGroup::IsInside(const mitk::Point3D& p) const
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
