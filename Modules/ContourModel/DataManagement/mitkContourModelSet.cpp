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
#include <mitkContourModelSet.h>
#include <vtkMath.h>
#include <algorithm>

mitk::ContourModelSet::ContourModelSet() :
  m_Contours(),
  m_UpdateBoundingBox(true)
{
  this->InitializeEmpty();
}



mitk::ContourModelSet::ContourModelSet(const mitk::ContourModelSet &other) :
  mitk::BaseData(other),
  m_Contours(other.m_Contours)
{
  this->InitializeTimeGeometry(1);
}



mitk::ContourModelSet::~ContourModelSet()
{
  this->m_Contours.clear();
}


void mitk::ContourModelSet::InitializeEmpty()
{
  this->InitializeTimeGeometry(1);
  m_Contours.resize(0);
}


void mitk::ContourModelSet::AddContourModel(mitk::ContourModel &contourModel)
{
  this->m_Contours.push_back(&contourModel);
  m_UpdateBoundingBox = true;
}


void mitk::ContourModelSet::AddContourModel(mitk::ContourModel::Pointer contourModel)
{
  this->m_Contours.push_back(contourModel);
  m_UpdateBoundingBox = true;
}



mitk::ContourModel* mitk::ContourModelSet::GetContourModelAt(int index) const
{
  if( index >= 0 && static_cast<ContourModelListType::size_type>(index) < this->m_Contours.size() )
  {
    return this->m_Contours.at(index).GetPointer();
  }
  else
  {
    return NULL;
  }
}

bool mitk::ContourModelSet::IsEmpty() const
{
  return this->m_Contours.empty();
}


mitk::ContourModelSet::ContourModelListType* mitk::ContourModelSet::GetContourModelList()
{
  return &(this->m_Contours);
}


bool mitk::ContourModelSet::RemoveContourModel(mitk::ContourModel* contourModel)
{
  ContourModelSetIterator it = this->m_Contours.begin();

  ContourModelSetIterator end = this->m_Contours.end();

  //search for ContourModel and remove it if exists
  while(it != end)
  {
    if((*it) == contourModel)
    {
      this->m_Contours.erase(it);
      m_UpdateBoundingBox = true;
      return true;
    }

    it++;
  }

  return false;
}


bool mitk::ContourModelSet::RemoveContourModelAt(int index)
{
  if( index >= 0 && static_cast<ContourModelListType::size_type>(index) < this->m_Contours.size() )
  {
    this->m_Contours.erase(this->m_Contours.begin()+index);
    m_UpdateBoundingBox = true;
    return true;
  }
  else
  {
    return false;
  }
}


void mitk::ContourModelSet::Clear()
{
  this->m_Contours.clear();
  m_UpdateBoundingBox = true;
}


void mitk::ContourModelSet::UpdateOutputInformation()
{
  if ( this->GetSource() )
  {
    this->GetSource()->UpdateOutputInformation();
  }

  if(this->m_UpdateBoundingBox)
  {

    //update the bounds of the geometry according to the stored vertices
    mitk::ScalarType mitkBounds[6];

    //calculate the boundingbox at each timestep
    typedef itk::BoundingBox<unsigned long, 3, ScalarType>        BoundingBoxType;
    typedef BoundingBoxType::PointsContainer                      PointsContainer;

    int timesteps = this->GetTimeSteps();

    //iterate over the timesteps
    for(int currenTimeStep = 0; currenTimeStep < timesteps; currenTimeStep++)
    {
      //only update bounds if the contour was modified
      if (this->GetMTime() > this->GetGeometry(currenTimeStep)->GetBoundingBox()->GetMTime())
      {
        mitkBounds[0] = 0.0;
        mitkBounds[1] = 0.0;
        mitkBounds[2] = 0.0;
        mitkBounds[3] = 0.0;
        mitkBounds[4] = 0.0;
        mitkBounds[5] = 0.0;

        BoundingBoxType::Pointer boundingBox = BoundingBoxType::New();

        PointsContainer::Pointer points = PointsContainer::New();

        mitk::ContourModelSet::ContourModelSetIterator contoursIt = this->Begin();
        mitk::ContourModelSet::ContourModelSetIterator contoursEnd = this->End();

        while(contoursIt!=contoursEnd)
        {

          mitk::ContourModel::VertexIterator it = contoursIt->GetPointer()->Begin(currenTimeStep);
          mitk::ContourModel::VertexIterator end = contoursIt->GetPointer()->End(currenTimeStep);

          //fill the boundingbox with the points
          while(it != end)
          {
            Point3D currentP = (*it)->Coordinates;
            BoundingBoxType::PointType p;
            p.CastFrom(currentP);
            points->InsertElement(points->Size(), p);

            it++;
          }

          ++contoursIt;
        }

        //construct the new boundingBox
        boundingBox->SetPoints(points);
        boundingBox->ComputeBoundingBox();
        BoundingBoxType::BoundsArrayType tmp = boundingBox->GetBounds();
        mitkBounds[0] = tmp[0];
        mitkBounds[1] = tmp[1];
        mitkBounds[2] = tmp[2];
        mitkBounds[3] = tmp[3];
        mitkBounds[4] = tmp[4];
        mitkBounds[5] = tmp[5];

        //set boundingBox at current timestep
        BaseGeometry* geometry3d = this->GetGeometry(currenTimeStep);
        geometry3d->SetBounds(mitkBounds);
      }
    }

    this->m_UpdateBoundingBox = false;
  }
  GetTimeGeometry()->Update();
}
