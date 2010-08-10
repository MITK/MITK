/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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
