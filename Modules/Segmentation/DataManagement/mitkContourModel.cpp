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
  m_ContourSeries.push_back(mitk::ContourModelElement::New());

  Superclass::InitializeTimeSlicedGeometry(1);
  m_SelectedVertex = NULL;
}

mitk::ContourModel::ContourModel(const mitk::ContourModel &other) :
m_ContourSeries(other.m_ContourSeries)
{
  m_SelectedVertex = NULL;
}

mitk::ContourModel::~ContourModel()
{
  m_SelectedVertex = NULL;
  this->m_ContourSeries.clear();//TODO check destruction
}


bool mitk::ContourModel::SelectVertexAt(int index, unsigned int timestep)
{
  if(index < 0 || index > this->m_ContourSeries[timestep]->GetSize())
    return false;

  this->m_SelectedVertex = this->m_ContourSeries[timestep]->GetVertexAt(index);

  return true;
}

bool mitk::ContourModel::SelectVertexAt(mitk::Point3D &point, float eps, unsigned int timestep)
{
  this->m_SelectedVertex = this->m_ContourSeries[timestep]->GetVertexAt(point, eps);
  return this->m_SelectedVertex != NULL;
}

bool mitk::ContourModel::RemoveVertexAt(int index, unsigned int timestep)
{
  if(index < 0 || index > this->m_ContourSeries[timestep]->GetSize())
    return false;

  this->m_ContourSeries[timestep]->RemoveVertexAt(index);

  return true;
}

bool mitk::ContourModel::RemoveVertexAt(mitk::Point3D &point, float eps, unsigned int timestep)
{
  return this->m_ContourSeries[timestep]->RemoveVertexAt(point, eps);
}

void mitk::ContourModel::MoveSelectedVertex(mitk::Vector3D &translate)
{
  if(this->m_SelectedVertex)
  {
    this->MoveVertex(this->m_SelectedVertex,translate);
  }
}

void mitk::ContourModel::MoveContour(mitk::Vector3D &translate, unsigned int timestep)
{
  VertexListType* vList = this->m_ContourSeries[timestep]->GetVertexList();
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
  vertex->Coordinates[1] += vector[1];
  vertex->Coordinates[2] += vector[2];
}


void mitk::ContourModel::Expand( unsigned int timeSteps )
{
  unsigned int oldSize = this->m_ContourSeries.size();

  if( timeSteps > oldSize )
  {
    Superclass::Expand(timeSteps);
    m_ContourSeries.push_back(mitk::ContourModelElement::New());
  }
}


void mitk::ContourModel::SetRequestedRegionToLargestPossibleRegion ()
{
  
}

bool mitk::ContourModel::RequestedRegionIsOutsideOfTheBufferedRegion ()
{
  return false;
}

bool mitk::ContourModel::VerifyRequestedRegion ()
{
  return true;
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