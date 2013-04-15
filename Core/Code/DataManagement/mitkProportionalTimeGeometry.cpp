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

mitk::ProportionalTimeGeometry::TimeStepType mitk::ProportionalTimeGeometry::GetNumberOfTimeSteps ()
{
  return static_cast<TimeStepType>(m_GeometryVector.size() );
}

mitk::ProportionalTimeGeometry::TimePointType mitk::ProportionalTimeGeometry::GetMinimumTimePoint ()
{
  return m_FirstTimePoint;
}

mitk::ProportionalTimeGeometry::TimePointType mitk::ProportionalTimeGeometry::GetMaximumTimePoint ()
{
  return m_FirstTimePoint + m_StepDuration + GetNumberOfTimeSteps();
}

mitk::TimeBounds mitk::ProportionalTimeGeometry::GetTimeBounds ()
{
  TimeBounds bounds;
  bounds[0] = this->GetMinimumTimePoint();
  bounds[1] = this->GetMaximumTimePoint();
  return bounds;
}

bool mitk::ProportionalTimeGeometry::IsValidTimePoint (TimePointType& timePoint)
{
  return this->GetMinimumTimePoint() <= timePoint && timePoint < this->GetMaximumTimePoint();
}

bool mitk::ProportionalTimeGeometry::IsValidTimeStep (TimeStepType& timeStep)
{
  return 0 <= timeStep && timeStep <  this->GetNumberOfTimeSteps();
}

mitk::ProportionalTimeGeometry::TimePointType mitk::ProportionalTimeGeometry::TimeStepToTimePoint( TimeStepType& timeStep)
{
  return m_FirstTimePoint + timeStep * m_StepDuration;
}

mitk::ProportionalTimeGeometry::TimeStepType mitk::ProportionalTimeGeometry::TimePointToTimeStep( TimePointType& timePoint)
{
  assert(timePoint >= m_FirstTimePoint);
  return static_cast<TimeStepType>((timePoint -m_FirstTimePoint) / m_StepDuration);
}

mitk::BaseGeometry* mitk::ProportionalTimeGeometry::GetGeometryForTimeStep( TimeStepType timeStep)
{
  return dynamic_cast<BaseGeometry*>(m_GeometryVector[timeStep].GetPointer());
}

mitk::BaseGeometry* mitk::ProportionalTimeGeometry::GetGeometryForTimePoint(TimePointType timePoint)
{
  TimeStepType timeStep = this->TimePointToTimeStep(timePoint);
  return this->GetGeometryForTimeStep(timeStep);
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
