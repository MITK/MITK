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
#include <mitkContourModel.h>

mitk::ContourModel::ContourModel()
{

}

mitk::ContourModel::ContourModel(mitk::ContourModel &other)
{

}

mitk::ContourModel::~ContourModel()
{

}


std::deque<mitk::ContourModelElement::VertexType*>* mitk::ContourModel::GetVertexList()
{
  return NULL;
}

bool mitk::ContourModel::SelectVertexAt(int index)
{
  return NULL;
}

bool mitk::ContourModel::SelectVertexAt(mitk::Point3D &point)
{
  return NULL;
}

bool mitk::ContourModel::RemoveVertexAt(int index)
{
  return NULL;
}

bool mitk::ContourModel::RemoveVertexAt(mitk::Point3D &point)
{
  return NULL;
}

void mitk::ContourModel::MoveSelectedVertex(mitk::Vector3D &translate)
{

}

void mitk::ContourModel::MoveContour(mitk::Vector3D &translate)
{

}


void mitk::ContourModel::SetRequestedRegionToLargestPossibleRegion ()
{
  
}

bool mitk::ContourModel::RequestedRegionIsOutsideOfTheBufferedRegion ()
{
  return NULL;
}

bool mitk::ContourModel::VerifyRequestedRegion ()
{
  return NULL;
}

const mitk::Geometry3D * mitk::ContourModel::GetUpdatedGeometry (int t)
{
  return NULL;
}

mitk::Geometry3D* mitk::ContourModel::GetGeometry (int t)const
{
  return NULL;
}

void mitk::ContourModel::SetRequestedRegion (itk::DataObject *data)
{

}