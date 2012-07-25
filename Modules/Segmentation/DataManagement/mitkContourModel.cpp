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
  this->m_Contour = mitk::ContourModelElement::New();
  m_SelectedVertex = NULL;
}

mitk::ContourModel::ContourModel(const mitk::ContourModel &other) :
m_Contour(other.m_Contour)
{
  m_SelectedVertex = NULL;
}

mitk::ContourModel::~ContourModel()
{
  m_SelectedVertex = NULL;
  this->m_Contour = NULL;
}


bool mitk::ContourModel::SelectVertexAt(int index)
{
  if(index < 0 || index > this->m_Contour->GetSize())
    return false;

  this->m_SelectedVertex = this->m_Contour->GetVertexAt(index);

  return true;
}

bool mitk::ContourModel::SelectVertexAt(mitk::Point3D &point, float eps)
{
  this->m_SelectedVertex = this->m_Contour->GetVertexAt(point, eps);
  return this->m_SelectedVertex != NULL;
}

bool mitk::ContourModel::RemoveVertexAt(int index)
{
  if(index < 0 || index > this->m_Contour->GetSize())
    return false;

  this->m_Contour->RemoveVertexAt(index);

  return true;
}

bool mitk::ContourModel::RemoveVertexAt(mitk::Point3D &point, float eps)
{
  return this->m_Contour->RemoveVertexAt(point, eps);
}

void mitk::ContourModel::MoveSelectedVertex(mitk::Vector3D &translate)
{
  if(this->m_SelectedVertex)
  {
    this->MoveVertex(this->m_SelectedVertex,translate);
  }
}

void mitk::ContourModel::MoveContour(mitk::Vector3D &translate)
{
  VertexListType* vList = this->m_Contour->GetVertexList();
  VertexIterator it = vList->begin();
  VertexIterator end = vList->end();

  while(it != end)
  {
    this->MoveVertex((*it),translate);
    it++;
  }

}



void mitk::ContourModel::MoveVertex(VertexType* vertex, mitk::Vector3D &vector)
{
  vertex->Coordinates[0] += vector[0];
  vertex->Coordinates[1] += vector[3];
  vertex->Coordinates[2] += vector[2];
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