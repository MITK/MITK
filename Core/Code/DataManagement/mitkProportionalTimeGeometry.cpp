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
#include <mitkProportionalTimeGeometry.h>

mitk::ProportionalTimeGeometry::ProportionalTimeGeometry()
{
}

mitk::ProportionalTimeGeometry::~ProportionalTimeGeometry()
{
}

void mitk::ProportionalTimeGeometry::Initialize()
{
}

mitk::TimeStepType mitk::ProportionalTimeGeometry::GetNumberOfTimeSteps () const
{
  return static_cast<TimeStepType>(m_GeometryVector.size() );
}

mitk::TimePointType mitk::ProportionalTimeGeometry::GetMinimumTimePoint () const
{
  return m_FirstTimePoint;
}

mitk::TimePointType mitk::ProportionalTimeGeometry::GetMaximumTimePoint () const
{
  return m_FirstTimePoint + m_StepDuration * GetNumberOfTimeSteps();
}

mitk::TimeBounds mitk::ProportionalTimeGeometry::GetTimeBounds () const
{
  TimeBounds bounds;
  bounds[0] = this->GetMinimumTimePoint();
  bounds[1] = this->GetMaximumTimePoint();
  return bounds;
}

bool mitk::ProportionalTimeGeometry::IsValidTimePoint (TimePointType timePoint) const
{
  return this->GetMinimumTimePoint() <= timePoint && timePoint < this->GetMaximumTimePoint();
}

bool mitk::ProportionalTimeGeometry::IsValidTimeStep (TimeStepType timeStep) const
{
  return 0 <= timeStep && timeStep <  this->GetNumberOfTimeSteps();
}

mitk::TimePointType mitk::ProportionalTimeGeometry::TimeStepToTimePoint( TimeStepType timeStep) const
{
  return m_FirstTimePoint + timeStep * m_StepDuration;
}

mitk::TimeStepType mitk::ProportionalTimeGeometry::TimePointToTimeStep( TimePointType timePoint) const
{
  assert(timePoint >= m_FirstTimePoint);
  return static_cast<TimeStepType>((timePoint -m_FirstTimePoint) / m_StepDuration);
}

mitk::Geometry3D* mitk::ProportionalTimeGeometry::GetGeometryForTimeStep( TimeStepType timeStep) const
{
  return dynamic_cast<Geometry3D*>(m_GeometryVector[timeStep].GetPointer());
}

mitk::Geometry3D* mitk::ProportionalTimeGeometry::GetGeometryForTimePoint(TimePointType timePoint) const
{
  TimeStepType timeStep = this->TimePointToTimeStep(timePoint);
  return this->GetGeometryForTimeStep(timeStep);
}


mitk::Geometry3D::Pointer mitk::ProportionalTimeGeometry::GetGeometryCloneForTimeStep( TimeStepType timeStep) const
{
    return m_GeometryVector[timeStep].GetPointer();
}

bool mitk::ProportionalTimeGeometry::IsValid()
{
  bool isValid = true;
  isValid &= m_GeometryVector.size() > 0;
  isValid &= m_StepDuration > 0;
  return isValid;
}

void mitk::ProportionalTimeGeometry::ExecuteOperation(mitk::Operation* operation)
{
}

void mitk::ProportionalTimeGeometry::ClearAllGeometries()
{
  m_GeometryVector.clear();
}

void mitk::ProportionalTimeGeometry::ReserveSpaceForGeometries(TimeStepType numberOfGeometries)
{
  m_GeometryVector.reserve(numberOfGeometries);
}

void mitk::ProportionalTimeGeometry::Expand(mitk::TimeStepType size)
{
  m_GeometryVector.reserve(size);
  while  (m_GeometryVector.size() < size)
  {
    m_GeometryVector.push_back(Geometry3D::New());
  }
}

void mitk::ProportionalTimeGeometry::SetTimeStepGeometry(Geometry3D *geometry, TimeStepType timeStep)
{
  assert(timeStep<m_GeometryVector.size());
  assert(timeStep >= 0);

  m_GeometryVector[timeStep] = geometry;
}

mitk::TimeGeometry::Pointer mitk::ProportionalTimeGeometry::Clone() const
{
  ProportionalTimeGeometry::Pointer newTimeGeometry = ProportionalTimeGeometry::New();
  newTimeGeometry->m_BoundingBox = this->m_BoundingBox->DeepCopy();
  newTimeGeometry->m_FirstTimePoint = this->m_FirstTimePoint;
  newTimeGeometry->m_StepDuration = this->m_StepDuration;
  newTimeGeometry->m_GeometryVector.clear();
  newTimeGeometry->Expand(this->GetNumberOfTimeSteps());
  for (TimeStepType i =0; i < GetNumberOfTimeSteps(); ++i)
  {
    Geometry3D* tempGeometry = dynamic_cast<Geometry3D*> (GetGeometryCloneForTimeStep(i)->Clone().GetPointer());
    newTimeGeometry->SetTimeStepGeometry(tempGeometry,i);
  }
  TimeGeometry::Pointer finalPointer = dynamic_cast<TimeGeometry*>(newTimeGeometry.GetPointer());
  return finalPointer;
}
