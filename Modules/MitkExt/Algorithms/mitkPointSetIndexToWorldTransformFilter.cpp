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

#include "mitkPointSetIndexToWorldTransformFilter.h"


mitk::PointSetIndexToWorldTransformFilter::PointSetIndexToWorldTransformFilter()
{

}

mitk::PointSetIndexToWorldTransformFilter::~PointSetIndexToWorldTransformFilter()
{

}

void mitk::PointSetIndexToWorldTransformFilter::GenerateData()
{
  mitk::PointSet::ConstPointer input  = this->GetInput();
  mitk::PointSet::Pointer output  = this->GetOutput();

  typedef std::vector<mitk::Point3D> PointContainer;
  PointContainer points;
  int pointNo = 0;

  while(pointNo!= input->GetSize())
  {
    mitk::Point3D current = input->GetPoint(pointNo);
    points.push_back(current);
    pointNo++;
  }
  output->GetGeometry()->SetIdentity();
  PointContainer::iterator pointsIter = points.begin();

  pointNo = 0;
  while(pointsIter != points.end())
  {
    output->SetPoint(pointNo,(*pointsIter));
    pointNo++;
    pointsIter++;
  }

}
