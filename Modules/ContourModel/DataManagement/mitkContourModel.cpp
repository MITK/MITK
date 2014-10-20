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
#include <mitkPlaneGeometry.h>

mitk::ContourModel::ContourModel() :
  m_UpdateBoundingBox(true)
{
  //set to initial state
  this->InitializeEmpty();
}



mitk::ContourModel::ContourModel(const mitk::ContourModel &other) :
  mitk::BaseData(other),
  m_ContourSeries(other.m_ContourSeries),
  m_lineInterpolation(other.m_lineInterpolation)
{
  m_SelectedVertex = NULL;
}



mitk::ContourModel::~ContourModel()
{
  m_SelectedVertex = NULL;
  this->m_ContourSeries.clear();//TODO check destruction
}



void mitk::ContourModel::AddVertex(mitk::Point3D &vertex, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep) )
  {
    this->AddVertex(vertex, false, timestep);
  }
}



void mitk::ContourModel::AddVertex(mitk::Point3D &vertex, bool isControlPoint, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->AddVertex(vertex, isControlPoint);
    this->InvokeEvent( ContourModelSizeChangeEvent() );
    this->Modified();this->m_UpdateBoundingBox = true;
  }
}



void mitk::ContourModel::AddVertex(VertexType &vertex, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->AddVertex(vertex);
    this->InvokeEvent( ContourModelSizeChangeEvent() );
    this->Modified();this->m_UpdateBoundingBox = true;
  }
}


void mitk::ContourModel::AddVertex(const VertexType* vertex, int timestep)
{
  if(vertex != NULL)
  {
    this->m_ContourSeries[timestep]->AddVertex(*const_cast<VertexType*>(vertex));
  }
}



void mitk::ContourModel::AddVertexAtFront(mitk::Point3D &vertex, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep) )
  {
    this->AddVertexAtFront(vertex, false, timestep);
  }
}



void mitk::ContourModel::AddVertexAtFront(mitk::Point3D &vertex, bool isControlPoint, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->AddVertexAtFront(vertex, isControlPoint);
    this->InvokeEvent( ContourModelSizeChangeEvent() );
    this->Modified();
    this->m_UpdateBoundingBox = true;
  }
}



void mitk::ContourModel::AddVertexAtFront(VertexType &vertex, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->AddVertexAtFront(vertex);
    this->InvokeEvent( ContourModelSizeChangeEvent() );
    this->Modified();
    this->m_UpdateBoundingBox = true;
  }
}



bool mitk::ContourModel::SetVertexAt(int pointId, const Point3D &point, unsigned int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    if(pointId >= 0 && this->m_ContourSeries[timestep]->GetSize() > pointId)
    {
      this->m_ContourSeries[timestep]->SetVertexAt( pointId, point );
      this->Modified();
      this->m_UpdateBoundingBox = true;
      return true;
    }
    return false;
  }
  return false;
}



bool mitk::ContourModel::SetVertexAt(int pointId, const VertexType *vertex, unsigned int timestep)
{
  if(vertex==NULL) return false;

  if(!this->IsEmptyTimeStep(timestep))
  {
    if(pointId >= 0 && this->m_ContourSeries[timestep]->GetSize() > pointId)
    {
      this->m_ContourSeries[timestep]->SetVertexAt( pointId, vertex );
      this->Modified();
      this->m_UpdateBoundingBox = true;
      return true;
    }
    return false;
  }
  return false;
}



void mitk::ContourModel::InsertVertexAtIndex(mitk::Point3D &vertex, int index, bool isControlPoint, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    if(index >= 0 && this->m_ContourSeries[timestep]->GetSize() > index)
    {
      this->m_ContourSeries[timestep]->InsertVertexAtIndex(vertex, isControlPoint, index);
      this->InvokeEvent( ContourModelSizeChangeEvent() );
      this->Modified();
      this->m_UpdateBoundingBox = true;
    }
  }
}


bool mitk::ContourModel::IsEmpty( int timestep) const
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->IsEmpty();
  }
  return true;
}



bool mitk::ContourModel::IsEmpty() const
{
  return this->IsEmpty(0);
}



int mitk::ContourModel::GetNumberOfVertices( int timestep) const
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



int mitk::ContourModel::GetIndex(const VertexType *vertex, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->GetIndex(vertex);
  }
  return -1;
}



void mitk::ContourModel::Close( int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->Close();
    this->InvokeEvent( ContourModelClosedEvent() );
    this->Modified();this->m_UpdateBoundingBox = true;
  }
}


void mitk::ContourModel::Open( int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->Open();
    this->InvokeEvent( ContourModelClosedEvent() );
    this->Modified();this->m_UpdateBoundingBox = true;
  }
}


void mitk::ContourModel::SetClosed(bool isClosed, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->SetClosed(isClosed);
    this->InvokeEvent( ContourModelClosedEvent() );
    this->Modified();this->m_UpdateBoundingBox = true;
  }
}



bool mitk::ContourModel::IsEmptyTimeStep(unsigned int t) const
{
  return (this->m_ContourSeries.size() <= t);
}


bool mitk::ContourModel::IsNearContour(mitk::Point3D &point, float eps, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
      return this->m_ContourSeries[timestep]->IsNearContour(point, eps);
  }
  return false;
}

void mitk::ContourModel::Concatenate(mitk::ContourModel* other, int timestep, bool check)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    if( !this->m_ContourSeries[timestep]->IsClosed() )
    {
      this->m_ContourSeries[timestep]->Concatenate(other->m_ContourSeries[timestep], check);
      this->InvokeEvent( ContourModelSizeChangeEvent() );
      this->Modified();this->m_UpdateBoundingBox = true;
    }
  }
}



mitk::ContourModel::VertexIterator mitk::ContourModel::Begin( int timestep) const
{
  return this->IteratorBegin(timestep);
}



mitk::ContourModel::VertexIterator mitk::ContourModel::IteratorBegin( int timestep) const
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->IteratorBegin();
  }
  else
  {
    mitkThrow() << "No iterator at invalid timestep " << timestep << ". There are only " << this->GetTimeSteps() << " timesteps available.";
  }
}



mitk::ContourModel::VertexIterator mitk::ContourModel::End( int timestep) const
{
  return this->IteratorEnd(timestep);
}



mitk::ContourModel::VertexIterator mitk::ContourModel::IteratorEnd( int timestep) const
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->IteratorEnd();
  }
  else
  {
    mitkThrow() << "No iterator at invalid timestep " << timestep << ". There are only " << this->GetTimeSteps() << " timesteps available.";
  }
}



bool mitk::ContourModel::IsClosed( int timestep) const
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->IsClosed();
  }
  return false;
}

bool mitk::ContourModel::SelectVertexAt(mitk::Point3D &point, float eps, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    this->m_SelectedVertex = this->m_ContourSeries[timestep]->GetVertexAt(point, eps);
  }
  return this->m_SelectedVertex != NULL;
}


bool mitk::ContourModel::SelectVertexAt(int index, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep) && index >= 0)
  {
    return (this->m_SelectedVertex = this->m_ContourSeries[timestep]->GetVertexAt(index));
  }
  return false;
}

bool mitk::ContourModel::SetControlVertexAt(mitk::Point3D &point, float eps, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    VertexType* vertex = this->m_ContourSeries[timestep]->GetVertexAt(point, eps);
    if (vertex != NULL)
    {
        vertex->IsControlPoint = true;
        return true;
    }
  }
  return false;
}

bool mitk::ContourModel::SetControlVertexAt(int index, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep) && index >= 0)
  {
      VertexType* vertex = this->m_ContourSeries[timestep]->GetVertexAt(index);
      if (vertex != NULL)
      {
        vertex->IsControlPoint = true;
        return true;
     }
  }
  return false;
}

bool mitk::ContourModel::RemoveVertex(const VertexType *vertex, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    if(this->m_ContourSeries[timestep]->RemoveVertex(vertex))
    {
      this->Modified();this->m_UpdateBoundingBox = true;
      this->InvokeEvent( ContourModelSizeChangeEvent() );
      return true;
    }
  }
  return false;
}



bool mitk::ContourModel::RemoveVertexAt(int index, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    if(this->m_ContourSeries[timestep]->RemoveVertexAt(index))
    {
      this->Modified();this->m_UpdateBoundingBox = true;
      this->InvokeEvent( ContourModelSizeChangeEvent() );
      return true;
    }
  }
  return false;
}



bool mitk::ContourModel::RemoveVertexAt(mitk::Point3D &point, float eps, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    if(this->m_ContourSeries[timestep]->RemoveVertexAt(point, eps))
    {
      this->Modified();this->m_UpdateBoundingBox = true;
      this->InvokeEvent( ContourModelSizeChangeEvent() );
      return true;
    }
  }
  return false;
}



void mitk::ContourModel::ShiftSelectedVertex(mitk::Vector3D &translate)
{
  if(this->m_SelectedVertex)
  {
    this->ShiftVertex(this->m_SelectedVertex,translate);
    this->Modified();this->m_UpdateBoundingBox = true;
  }
}



void mitk::ContourModel::ShiftContour(mitk::Vector3D &translate, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    VertexListType* vList = this->m_ContourSeries[timestep]->GetVertexList();
    VertexIterator it = vList->begin();
    VertexIterator end = vList->end();

    //shift all vertices
    while(it != end)
    {
      this->ShiftVertex((*it),translate);
      it++;
    }

    this->Modified();this->m_UpdateBoundingBox = true;
    this->InvokeEvent( ContourModelShiftEvent() );
  }
}



void mitk::ContourModel::ShiftVertex(VertexType* vertex, mitk::Vector3D &vector)
{
  vertex->Coordinates[0] += vector[0];
  vertex->Coordinates[1] += vector[1];
  vertex->Coordinates[2] += vector[2];
}



void mitk::ContourModel::Clear(int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    //clear data at timestep
    this->m_ContourSeries[timestep]->Clear();
    this->InitializeEmpty();
    this->Modified();this->m_UpdateBoundingBox = true;
  }
}



void mitk::ContourModel::Expand(unsigned int timeSteps )
{
  std::size_t oldSize = this->m_ContourSeries.size();

  if( static_cast<std::size_t>(timeSteps) > oldSize )
  {
    Superclass::Expand(timeSteps);

    //insert contours for each new timestep
    for( std::size_t i = oldSize; i < static_cast<std::size_t>(timeSteps); i++)
    {
      m_ContourSeries.push_back(mitk::ContourElement::New());
    }

    this->InvokeEvent( ContourModelExpandTimeBoundsEvent() );
  }
}



void mitk::ContourModel::SetRequestedRegionToLargestPossibleRegion ()
{
  //no support for regions
}



bool mitk::ContourModel::RequestedRegionIsOutsideOfTheBufferedRegion ()
{
  //no support for regions
  return false;
}



bool mitk::ContourModel::VerifyRequestedRegion ()
{
  //no support for regions
  return true;
}



const mitk::BaseGeometry * mitk::ContourModel::GetUpdatedGeometry (int t)
{
  return Superclass::GetUpdatedGeometry(t);
}



mitk::BaseGeometry* mitk::ContourModel::GetGeometry (int t)const
{
  return Superclass::GetGeometry(t);
}


void mitk::ContourModel::SetRequestedRegion( const itk::DataObject* /*data*/)
{
  //no support for regions
}


void mitk::ContourModel::Clear()
{
  //clear data and set to initial state again
  this->ClearData();
  this->InitializeEmpty();
  this->Modified();this->m_UpdateBoundingBox = true;
}


void mitk::ContourModel::RedistributeControlVertices(int period, int timestep)
{
  if(!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->RedistributeControlVertices(this->GetSelectedVertex(), period);
    this->InvokeEvent( ContourModelClosedEvent() );
    this->Modified();this->m_UpdateBoundingBox = true;
  }
}


void mitk::ContourModel::ClearData()
{
  //call the superclass, this releases the data of BaseData
  Superclass::ClearData();

  //clear out the time resolved contours
  this->m_ContourSeries.clear();
}



void mitk::ContourModel::Initialize()
{
  this->InitializeEmpty();
  this->Modified();this->m_UpdateBoundingBox = true;
}



void mitk::ContourModel::Initialize(mitk::ContourModel &other)
{
  mitk::TimeStepType numberOfTimesteps = other.GetTimeGeometry()->CountTimeSteps();
  this->InitializeTimeGeometry(numberOfTimesteps);

  for(mitk::TimeStepType currentTimestep = 0; currentTimestep < numberOfTimesteps; currentTimestep++)
  {
    this->m_ContourSeries.push_back(mitk::ContourElement::New());
    this->SetClosed(other.IsClosed(currentTimestep),currentTimestep);
  }

  m_SelectedVertex = NULL;
  this->m_lineInterpolation = other.m_lineInterpolation;
  this->Modified();this->m_UpdateBoundingBox = true;
}



void mitk::ContourModel::InitializeEmpty()
{
  //clear data at timesteps
  this->m_ContourSeries.resize(0);
  this->m_ContourSeries.push_back(mitk::ContourElement::New());

  //set number of timesteps to one
  this->InitializeTimeGeometry(1);

  m_SelectedVertex = NULL;
  this->m_lineInterpolation = ContourModel::LINEAR;
}


void mitk::ContourModel::UpdateOutputInformation()
{
  if ( this->GetSource() )
  {
    this->GetSource()->UpdateOutputInformation();
  }

  if(this->m_UpdateBoundingBox)
  {
    //update the bounds of the geometry according to the stored vertices
  ScalarType mitkBounds[6];

    //calculate the boundingbox at each timestep
    typedef itk::BoundingBox<unsigned long, 3, ScalarType>        BoundingBoxType;
    typedef BoundingBoxType::PointsContainer                      PointsContainer;

    int timesteps = this->GetTimeSteps();

    //iterate over the timesteps
    for(int currenTimeStep = 0; currenTimeStep < timesteps; currenTimeStep++)
    {
      if( dynamic_cast< mitk::PlaneGeometry* >(this->GetGeometry(currenTimeStep)) )
      {
        //do not update bounds for 2D geometries, as they are unfortunately defined with min bounds 0!
        return;
      }
      else
      {//we have a 3D geometry -> let's update bounds
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
    }
    this->m_UpdateBoundingBox = false;
  }
  GetTimeGeometry()->Update();
}


void mitk::ContourModel::ExecuteOperation(mitk::Operation* /*operation*/)
{
  //not supported yet
}
