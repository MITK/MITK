/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <mitkContourModel.h>
#include <mitkPlaneGeometry.h>

mitk::ContourModel::ContourModel() : m_UpdateBoundingBox(true)
{
  // set to initial state
  this->InitializeEmpty();
}

mitk::ContourModel::ContourModel(const ContourModel &other)
  : BaseData(other), m_ContourSeries(other.m_ContourSeries), m_lineInterpolation(other.m_lineInterpolation)
{
  m_SelectedVertex = nullptr;
}

mitk::ContourModel::~ContourModel()
{
  m_SelectedVertex = nullptr;
  this->m_ContourSeries.clear(); // TODO check destruction
}

void mitk::ContourModel::AddVertex(const Point3D &vertex, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    this->AddVertex(vertex, false, timestep);
  }
}

void mitk::ContourModel::AddVertex(const Point3D &vertex, bool isControlPoint, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->AddVertex(vertex, isControlPoint);
    this->InvokeEvent(ContourModelSizeChangeEvent());
    this->Modified();
    this->m_UpdateBoundingBox = true;
  }
}

void mitk::ContourModel::AddVertex(const VertexType &vertex, TimeStepType timestep)
{
  this->AddVertex(vertex.Coordinates, vertex.IsControlPoint, timestep);
}

void mitk::ContourModel::AddVertexAtFront(const Point3D &vertex, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    this->AddVertexAtFront(vertex, false, timestep);
  }
}

void mitk::ContourModel::AddVertexAtFront(const Point3D &vertex, bool isControlPoint, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->AddVertexAtFront(vertex, isControlPoint);
    this->InvokeEvent(ContourModelSizeChangeEvent());
    this->Modified();
    this->m_UpdateBoundingBox = true;
  }
}

void mitk::ContourModel::AddVertexAtFront(const VertexType &vertex, TimeStepType timestep)
{
  this->AddVertexAtFront(vertex.Coordinates, vertex.IsControlPoint, timestep);
}

bool mitk::ContourModel::SetVertexAt(int pointId, const Point3D &point, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    if (pointId >= 0 && this->m_ContourSeries[timestep]->GetSize() > ContourElement::VertexSizeType(pointId))
    {
      this->m_ContourSeries[timestep]->SetVertexAt(pointId, point);
      this->Modified();
      this->m_UpdateBoundingBox = true;
      return true;
    }
    return false;
  }
  return false;
}

bool mitk::ContourModel::SetVertexAt(int pointId, const VertexType *vertex, TimeStepType timestep)
{
  if (vertex == nullptr)
    return false;

  if (!this->IsEmptyTimeStep(timestep))
  {
    if (pointId >= 0 && this->m_ContourSeries[timestep]->GetSize() > ContourElement::VertexSizeType(pointId))
    {
      this->m_ContourSeries[timestep]->SetVertexAt(pointId, vertex);
      this->Modified();
      this->m_UpdateBoundingBox = true;
      return true;
    }
    return false;
  }
  return false;
}

void mitk::ContourModel::InsertVertexAtIndex(const Point3D &vertex, int index, bool isControlPoint, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    if (index >= 0 && this->m_ContourSeries[timestep]->GetSize() > ContourElement::VertexSizeType(index))
    {
      this->m_ContourSeries[timestep]->InsertVertexAtIndex(vertex, isControlPoint, index);
      this->InvokeEvent(ContourModelSizeChangeEvent());
      this->Modified();
      this->m_UpdateBoundingBox = true;
    }
  }
}

void mitk::ContourModel::UpdateContour(const ContourModel* sourceModel, TimeStepType destinationTimeStep, TimeStepType sourceTimeStep)
{
  if (nullptr == sourceModel)
  {
    mitkThrow() << "Cannot update contour. Passed source model is invalid.";
  }

  if (!sourceModel->GetTimeGeometry()->IsValidTimeStep(sourceTimeStep))
  {
    mitkThrow() << "Cannot update contour. Source contour time geometry does not support passed time step. Invalid time step: " << sourceTimeStep;
  }

  if (!this->GetTimeGeometry()->IsValidTimeStep(destinationTimeStep))
  {
    MITK_WARN << "Cannot update contour. Contour time geometry does not support passed time step. Invalid time step: " << destinationTimeStep;
    return;
  }

  this->Clear(destinationTimeStep);

  std::for_each(sourceModel->Begin(sourceTimeStep), sourceModel->End(sourceTimeStep), [this, destinationTimeStep](ContourElement::VertexType* vertex) {
    this->m_ContourSeries[destinationTimeStep]->AddVertex(vertex->Coordinates, vertex->IsControlPoint);
  });

  this->InvokeEvent(ContourModelSizeChangeEvent());
  this->Modified();
  this->m_UpdateBoundingBox = true;
}

bool mitk::ContourModel::IsEmpty(TimeStepType timestep) const
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->IsEmpty();
  }
  return true;
}

bool mitk::ContourModel::IsEmpty() const
{
  return this->IsEmpty(0);
}

int mitk::ContourModel::GetNumberOfVertices(TimeStepType timestep) const
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->GetSize();
  }
  return -1;
}

const mitk::ContourModel::VertexType *mitk::ContourModel::GetVertexAt(int index, TimeStepType timestep) const
{
  if (!this->IsEmptyTimeStep(timestep) && this->m_ContourSeries[timestep]->GetSize() > mitk::ContourElement::VertexSizeType(index))
  {
    return this->m_ContourSeries[timestep]->GetVertexAt(index);
  }
  return nullptr;
}

int mitk::ContourModel::GetIndex(const VertexType *vertex, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->GetIndex(vertex);
  }
  return -1;
}

void mitk::ContourModel::Close(TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->Close();
    this->InvokeEvent(ContourModelClosedEvent());
    this->Modified();
    this->m_UpdateBoundingBox = true;
  }
}

void mitk::ContourModel::Open(TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->Open();
    this->InvokeEvent(ContourModelClosedEvent());
    this->Modified();
    this->m_UpdateBoundingBox = true;
  }
}

void mitk::ContourModel::SetClosed(bool isClosed, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->SetClosed(isClosed);
    this->InvokeEvent(ContourModelClosedEvent());
    this->Modified();
    this->m_UpdateBoundingBox = true;
  }
}

bool mitk::ContourModel::IsEmptyTimeStep(unsigned int t) const
{
  return (this->m_ContourSeries.size() <= t);
}

bool mitk::ContourModel::IsNearContour(Point3D &point, float eps, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->IsNearContour(point, eps);
  }
  return false;
}

void mitk::ContourModel::Concatenate(ContourModel *other, TimeStepType timestep, bool check)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    if (!this->m_ContourSeries[timestep]->IsClosed())
    {
      this->m_ContourSeries[timestep]->Concatenate(other->m_ContourSeries[timestep], check);
      this->InvokeEvent(ContourModelSizeChangeEvent());
      this->Modified();
      this->m_UpdateBoundingBox = true;
    }
  }
}

mitk::ContourModel::VertexIterator mitk::ContourModel::Begin(TimeStepType timestep) const
{
  return this->IteratorBegin(timestep);
}

mitk::ContourModel::VertexIterator mitk::ContourModel::IteratorBegin(TimeStepType timestep) const
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->IteratorBegin();
  }
  else
  {
    mitkThrow() << "No iterator at invalid timestep " << timestep << ". There are only " << this->GetTimeSteps()
                << " timesteps available.";
  }
}

mitk::ContourModel::VertexIterator mitk::ContourModel::End(TimeStepType timestep) const
{
  return this->IteratorEnd(timestep);
}

mitk::ContourModel::VertexIterator mitk::ContourModel::IteratorEnd(TimeStepType timestep) const
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->IteratorEnd();
  }
  else
  {
    mitkThrow() << "No iterator at invalid timestep " << timestep << ". There are only " << this->GetTimeSteps()
                << " timesteps available.";
  }
}

bool mitk::ContourModel::IsClosed(int timestep) const
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    return this->m_ContourSeries[timestep]->IsClosed();
  }
  return false;
}

bool mitk::ContourModel::SelectVertexAt(Point3D &point, float eps, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    this->m_SelectedVertex = this->m_ContourSeries[timestep]->GetVertexAt(point, eps);
  }
  return this->m_SelectedVertex != nullptr;
}

bool mitk::ContourModel::SelectVertexAt(int index, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep) && index >= 0)
  {
    return (this->m_SelectedVertex = this->m_ContourSeries[timestep]->GetVertexAt(index));
  }
  return false;
}

bool mitk::ContourModel::SetControlVertexAt(Point3D &point, float eps, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    VertexType *vertex = this->m_ContourSeries[timestep]->GetVertexAt(point, eps);
    if (vertex != nullptr)
    {
      vertex->IsControlPoint = true;
      return true;
    }
  }
  return false;
}

bool mitk::ContourModel::SetControlVertexAt(int index, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep) && index >= 0)
  {
    VertexType *vertex = this->m_ContourSeries[timestep]->GetVertexAt(index);
    if (vertex != nullptr)
    {
      vertex->IsControlPoint = true;
      return true;
    }
  }
  return false;
}

bool mitk::ContourModel::RemoveVertex(const VertexType *vertex, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    if (this->m_ContourSeries[timestep]->RemoveVertex(vertex))
    {
      this->Modified();
      this->m_UpdateBoundingBox = true;
      this->InvokeEvent(ContourModelSizeChangeEvent());
      return true;
    }
  }
  return false;
}

bool mitk::ContourModel::RemoveVertexAt(int index, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    if (this->m_ContourSeries[timestep]->RemoveVertexAt(index))
    {
      this->Modified();
      this->m_UpdateBoundingBox = true;
      this->InvokeEvent(ContourModelSizeChangeEvent());
      return true;
    }
  }
  return false;
}

bool mitk::ContourModel::RemoveVertexAt(Point3D &point, float eps, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    if (this->m_ContourSeries[timestep]->RemoveVertexAt(point, eps))
    {
      this->Modified();
      this->m_UpdateBoundingBox = true;
      this->InvokeEvent(ContourModelSizeChangeEvent());
      return true;
    }
  }
  return false;
}

void mitk::ContourModel::ShiftSelectedVertex(Vector3D &translate)
{
  if (this->m_SelectedVertex)
  {
    this->ShiftVertex(this->m_SelectedVertex, translate);
    this->Modified();
    this->m_UpdateBoundingBox = true;
  }
}

void mitk::ContourModel::ShiftContour(Vector3D &translate, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    // shift all vertices
    for (auto vertex : *(this->m_ContourSeries[timestep]))
    {
      this->ShiftVertex(vertex, translate);
    }

    this->Modified();
    this->m_UpdateBoundingBox = true;
    this->InvokeEvent(ContourModelShiftEvent());
  }
}

void mitk::ContourModel::ShiftVertex(VertexType *vertex, Vector3D &vector)
{
  vertex->Coordinates[0] += vector[0];
  vertex->Coordinates[1] += vector[1];
  vertex->Coordinates[2] += vector[2];
}

void mitk::ContourModel::Clear(TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    // clear data at timestep
    this->m_ContourSeries[timestep]->Clear();
    this->Modified();
    this->m_UpdateBoundingBox = true;
  }
}

void mitk::ContourModel::Expand(unsigned int timeSteps)
{
  std::size_t oldSize = this->m_ContourSeries.size();

  if (static_cast<std::size_t>(timeSteps) > oldSize)
  {
    Superclass::Expand(timeSteps);

    // insert contours for each new timestep
    for (std::size_t i = oldSize; i < static_cast<std::size_t>(timeSteps); i++)
    {
      m_ContourSeries.push_back(ContourElement::New());
    }

    this->InvokeEvent(ContourModelExpandTimeBoundsEvent());
  }
}

void mitk::ContourModel::SetRequestedRegionToLargestPossibleRegion()
{
  // no support for regions
}

bool mitk::ContourModel::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  // no support for regions
  return false;
}

bool mitk::ContourModel::VerifyRequestedRegion()
{
  // no support for regions
  return true;
}

void mitk::ContourModel::SetRequestedRegion(const itk::DataObject * /*data*/)
{
  // no support for regions
}

void mitk::ContourModel::Clear()
{
  // clear data and set to initial state again
  this->ClearData();
  this->InitializeEmpty();
  this->Modified();
  this->m_UpdateBoundingBox = true;
}

void mitk::ContourModel::RedistributeControlVertices(int period, TimeStepType timestep)
{
  if (!this->IsEmptyTimeStep(timestep))
  {
    this->m_ContourSeries[timestep]->RedistributeControlVertices(this->GetSelectedVertex(), period);
    this->InvokeEvent(ContourModelClosedEvent());
    this->Modified();
    this->m_UpdateBoundingBox = true;
  }
}

void mitk::ContourModel::ClearData()
{
  // call the superclass, this releases the data of BaseData
  Superclass::ClearData();

  // clear out the time resolved contours
  this->m_ContourSeries.clear();
}

void mitk::ContourModel::Initialize()
{
  this->InitializeEmpty();
  this->Modified();
  this->m_UpdateBoundingBox = true;
}

void mitk::ContourModel::Initialize(const ContourModel &other)
{
  TimeStepType numberOfTimesteps = other.GetTimeGeometry()->CountTimeSteps();
  this->InitializeTimeGeometry(numberOfTimesteps);

  for (TimeStepType currentTimestep = 0; currentTimestep < numberOfTimesteps; currentTimestep++)
  {
    this->m_ContourSeries.push_back(ContourElement::New());
    this->SetClosed(other.IsClosed(currentTimestep), currentTimestep);
  }

  m_SelectedVertex = nullptr;
  this->m_lineInterpolation = other.m_lineInterpolation;
  this->Modified();
  this->m_UpdateBoundingBox = true;
}

void mitk::ContourModel::InitializeEmpty()
{
  // clear data at timesteps
  this->m_ContourSeries.resize(0);
  this->m_ContourSeries.push_back(ContourElement::New());

  // set number of timesteps to one
  this->InitializeTimeGeometry(1);

  m_SelectedVertex = nullptr;
  this->m_lineInterpolation = ContourModel::LINEAR;
}

void mitk::ContourModel::UpdateOutputInformation()
{
  if (this->GetSource())
  {
    this->GetSource()->UpdateOutputInformation();
  }

  if (this->m_UpdateBoundingBox)
  {
    // update the bounds of the geometry according to the stored vertices
    ScalarType mitkBounds[6];

    // calculate the boundingbox at each timestep
    typedef itk::BoundingBox<unsigned long, 3, ScalarType> BoundingBoxType;
    typedef BoundingBoxType::PointsContainer PointsContainer;

    int timesteps = this->GetTimeSteps();

    // iterate over the timesteps
    for (int currenTimeStep = 0; currenTimeStep < timesteps; currenTimeStep++)
    {
      if (dynamic_cast<PlaneGeometry *>(this->GetGeometry(currenTimeStep)))
      {
        // do not update bounds for 2D geometries, as they are unfortunately defined with min bounds 0!
        return;
      }
      else
      { // we have a 3D geometry -> let's update bounds
        // only update bounds if the contour was modified
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

          auto it = this->IteratorBegin(currenTimeStep);
          auto end = this->IteratorEnd(currenTimeStep);

          // fill the boundingbox with the points
          while (it != end)
          {
            Point3D currentP = (*it)->Coordinates;
            BoundingBoxType::PointType p;
            p.CastFrom(currentP);
            points->InsertElement(points->Size(), p);

            it++;
          }

          // construct the new boundingBox
          boundingBox->SetPoints(points);
          boundingBox->ComputeBoundingBox();
          BoundingBoxType::BoundsArrayType tmp = boundingBox->GetBounds();
          mitkBounds[0] = tmp[0];
          mitkBounds[1] = tmp[1];
          mitkBounds[2] = tmp[2];
          mitkBounds[3] = tmp[3];
          mitkBounds[4] = tmp[4];
          mitkBounds[5] = tmp[5];

          // set boundingBox at current timestep
          BaseGeometry *geometry3d = this->GetGeometry(currenTimeStep);
          geometry3d->SetBounds(mitkBounds);
        }
      }
    }
    this->m_UpdateBoundingBox = false;
  }
  GetTimeGeometry()->Update();
}

void mitk::ContourModel::ExecuteOperation(Operation * /*operation*/)
{
  // not supported yet
}
