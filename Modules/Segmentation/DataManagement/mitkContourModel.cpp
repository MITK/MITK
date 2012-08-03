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
  this->InitializeEmpty();
}



mitk::ContourModel::ContourModel(const mitk::ContourModel &other) :
m_ContourSeries(other.m_ContourSeries), m_lineInterpolation(other.m_lineInterpolation)
{
  m_SelectedVertex = NULL;
}



mitk::ContourModel::~ContourModel()
{
  m_SelectedVertex = NULL;
  this->m_ContourSeries.clear();//TODO check destruction
}



void mitk::ContourModel::AddVertex(mitk::Point3D &vertex, unsigned int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    this->AddVertex(vertex, false, timestep);
  }
}



void mitk::ContourModel::AddVertex(mitk::Point3D &vertex, bool isActive, unsigned int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->AddVertex(vertex, isActive);
  }
}



int mitk::ContourModel::GetNumberOfVertices( unsigned int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->GetSize();
  }
  return -1;
}



const mitk::ContourModel::VertexType* mitk::ContourModel::GetVertexAt(int index, int timestep) const
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->GetVertexAt(index);
  }
  return NULL;
}



void mitk::ContourModel::Close(unsigned int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->Close();
  }
}



bool mitk::ContourModel::IsEmptyTimeStep(unsigned int t) const
{
  return (t < 0) || (this->m_ContourSeries.size() <= t);
}



void mitk::ContourModel::Concatenate(mitk::ContourModel* other, unsigned int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->Concatenate(other->m_ContourSeries[timestep]);
  }
}



mitk::ContourModel::VertexIterator mitk::ContourModel::IteratorBegin(unsigned int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->ConstIteratorBegin();
  }
  else
  {
    throw std::exception("invalid timestep");// << timestep << "only " << this->GetTimeSteps() << " available.");
  }
}



mitk::ContourModel::VertexIterator mitk::ContourModel::IteratorEnd( unsigned int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->ConstIteratorEnd();
  }
  else
  {
    throw std::exception("invalid timestep");// + timestep + "only " + this->GetTimeSteps() + " available.");
  }
}



bool mitk::ContourModel::IsClosed(unsigned int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->IsClosed();
  }
  return false;
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
  if(!this->IsEmptyTimeStep(timestep))
  {
    this->m_SelectedVertex = this->m_ContourSeries[timestep]->GetVertexAt(point, eps);
  }
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
  if(!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->RemoveVertexAt(point, eps);
  }
  return false;
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
  if(!this->IsEmptyTimeStep(timestep))
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
  return Superclass::GetUpdatedGeometry(t);
}



mitk::Geometry3D* mitk::ContourModel::GetGeometry (int t)const
{
  return Superclass::GetGeometry(t);
}



void mitk::ContourModel::SetRequestedRegion (itk::DataObject *data)
{

}


void mitk::ContourModel::Clear()
{
  //clear data and set to initial state again
  this->ClearData();
  this->InitializeEmpty();
}



void mitk::ContourModel::ClearData()
{
  //call the superclass, this releases the data of BaseData
  Superclass::ClearData();

  //clear out the time resolved contours
  this->m_ContourSeries.clear();
}



void mitk::ContourModel::InitializeEmpty()
{
  this->m_ContourSeries.resize(0);
  this->m_ContourSeries.push_back(mitk::ContourModelElement::New());

  this->InitializeTimeSlicedGeometry(1);

  m_SelectedVertex = NULL;
  this->m_lineInterpolation = ContourModel::LINEAR;
}



void mitk::ContourModel::UpdateOutputInformation()
{
  //Superclass::UpdateOutputInformation();/*
  if ( this->GetSource() )
  {
    this->GetSource()->UpdateOutputInformation();
  }

  //update the bounds of the geometry
  float mitkBounds[6];

  //if no controlPoints are available the boundingbox is 0 in all dimensions
  if (this->GetNumberOfVertices() == 0)
  {
    mitkBounds[0] = 0.0;
    mitkBounds[1] = 0.0;
    mitkBounds[2] = 0.0;
    mitkBounds[3] = 0.0;
    mitkBounds[4] = 0.0;
    mitkBounds[5] = 0.0;
  }
  else
  {
    //calculate the boundingbox accroding to all point in the contour in each timestep

    typedef itk::BoundingBox<unsigned long, 3, ScalarType>        BoundingBoxType;
    typedef BoundingBoxType::PointsContainer                      PointsContainer;

    unsigned int timesteps = this->GetTimeSteps();

    //update the boudingbox of each timestep
    for(int currenTimeStep = 0; currenTimeStep < timesteps; currenTimeStep++)
    {

      BoundingBoxType::Pointer boundingBox = BoundingBoxType::New();

      PointsContainer::Pointer points = PointsContainer::New();

      VertexIterator it = this->IteratorBegin(currenTimeStep);
      VertexIterator end = this->IteratorEnd(currenTimeStep);

      //fill the boundingbox with the points
      while(it != end)
      {
        Point3D currentP = (*it)->Coordinates;
        BoundingBoxType::PointType p;
        p.CastFrom(currentP);
        points->InsertElement(points->Size(), p);

        it++;
      }

      boundingBox->SetPoints(points);
      boundingBox->ComputeBoundingBox();
      BoundingBoxType::BoundsArrayType tmp = boundingBox->GetBounds();
      mitkBounds[0] = tmp[0];
      mitkBounds[1] = tmp[1];
      mitkBounds[2] = tmp[2];
      mitkBounds[3] = tmp[3];
      mitkBounds[4] = tmp[4];
      mitkBounds[5] = tmp[5];

      Geometry3D* geometry3d = this->GetGeometry(currenTimeStep);
      geometry3d->SetBounds(mitkBounds);
    }

    GetTimeSlicedGeometry()->UpdateInformation();
  }//*/
}



void mitk::ContourModel::ExecuteOperation(mitk::Operation* operation)
{

}