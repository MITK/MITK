/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "mitkContourSet.h"
#include <mitkProportionalTimeGeometry.h>

mitk::ContourSet::ContourSet() :
  m_ContourVector( ContourVectorType() ),
  m_NumberOfContours (0)
{
  ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
  timeGeometry->Initialize(1);
  SetTimeGeometry(timeGeometry);
}

mitk::ContourSet::~ContourSet()
{
}

void mitk::ContourSet::AddContour(unsigned int index, mitk::Contour::Pointer contour)
{
  m_ContourVector.insert(std::make_pair( index , contour) );
}


void mitk::ContourSet::RemoveContour(unsigned long index)
{
  m_ContourVector.erase( index );
}

void mitk::ContourSet::UpdateOutputInformation()
{
  mitk::ContourSet::ContourVectorType contourVec = GetContours();
  mitk::ContourSet::ContourIterator contoursIterator = contourVec.begin();
  mitk::ContourSet::ContourIterator contoursIteratorEnd = contourVec.end();

  // initialize container
  mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();

  mitk::BoundingBox::PointIdentifier pointid=0;
  mitk::Point3D point;

  mitk::AffineTransform3D* transform = GetGeometry(0)->GetIndexToWorldTransform();
  mitk::AffineTransform3D::Pointer inverse = mitk::AffineTransform3D::New();
  transform->GetInverse(inverse);

  // calculate a bounding box that includes all contours
  // \todo probably we should do this additionally for each time-step
  while (contoursIterator != contoursIteratorEnd)
  {
    const TimeGeometry* geometry = (*contoursIterator).second->GetUpdatedTimeGeometry();
    unsigned char i;
    for(i=0; i<8; ++i)
    {
      point = inverse->TransformPoint(geometry->GetCornerPointInWorld(i));
      if(point[0]*point[0]+point[1]*point[1]+point[2]*point[2] < mitk::large)
        pointscontainer->InsertElement( pointid++, point);
      else
      {
        itkGenericOutputMacro( << "Unrealistically distant corner point encountered. Ignored. BoundingObject: " << (*contoursIterator).second );
      }
    }
    ++contoursIterator;
  }

  mitk::BoundingBox::Pointer boundingBox = mitk::BoundingBox::New();
  boundingBox->SetPoints(pointscontainer);
  boundingBox->ComputeBoundingBox();

  BaseGeometry* geometry3d = GetGeometry(0);
  geometry3d->SetIndexToWorldTransform(transform);
  geometry3d->SetBounds(boundingBox->GetBounds());

  ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
  timeGeometry->Initialize(geometry3d,GetTimeGeometry()->CountTimeSteps());
  SetTimeGeometry(timeGeometry);

}

void mitk::ContourSet::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::ContourSet::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return true;
}

bool mitk::ContourSet::VerifyRequestedRegion()
{
    return true;
}

void mitk::ContourSet::SetRequestedRegion( const itk::DataObject*)
{
}

void mitk::ContourSet::Initialize()
{
  m_ContourVector = ContourVectorType();
  ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
  timeGeometry->Initialize(1);
  SetTimeGeometry(timeGeometry);
}


unsigned int mitk::ContourSet::GetNumberOfContours()
{
  return m_ContourVector.size();
}

mitk::ContourSet::ContourVectorType mitk::ContourSet::GetContours()
{
  return m_ContourVector;
}
