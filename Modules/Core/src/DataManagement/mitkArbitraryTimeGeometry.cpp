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
#include <mitkArbitraryTimeGeometry.h>
#include <limits>

#include <algorithm>

#include <mitkGeometry3D.h>

mitk::ArbitraryTimeGeometry::ArbitraryTimeGeometry(): m_MinimumTimePoint(0)
{
}

mitk::ArbitraryTimeGeometry::~ArbitraryTimeGeometry()
{
}

void mitk::ArbitraryTimeGeometry::Initialize()
{
  this->ClearAllGeometries();
  Geometry3D::Pointer geo = Geometry3D::New();
  this->AppendTimeStep(geo,1,0);
}

mitk::TimeStepType mitk::ArbitraryTimeGeometry::CountTimeSteps () const
{
  return static_cast<TimeStepType>(m_GeometryVector.size() );
}

mitk::TimePointType mitk::ArbitraryTimeGeometry::GetMinimumTimePoint () const
{
  return m_MinimumTimePoint;
}

mitk::TimePointType mitk::ArbitraryTimeGeometry::GetMaximumTimePoint () const
{
  TimePointType result = 0;
  if (!m_MaximumTimePoints.empty())
  {
    result = m_MaximumTimePoints.back();
  }
  return result;
}

mitk::TimePointType mitk::ArbitraryTimeGeometry::GetMinimumTimePoint(TimeStepType step) const
{
  TimePointType result = m_MinimumTimePoint;
  if (step > 0 && step<=m_MaximumTimePoints.size())
  {
    result = m_MaximumTimePoints[step-1];
  }
  return result;
};

mitk::TimePointType mitk::ArbitraryTimeGeometry::GetMaximumTimePoint(TimeStepType step) const
{
  TimePointType result = 0;
  if (step<m_MaximumTimePoints.size())
  {
    result = m_MaximumTimePoints[step];
  }
  return result;
};

mitk::TimeBounds mitk::ArbitraryTimeGeometry::GetTimeBounds () const
{
  TimeBounds bounds;
  bounds[0] = this->GetMinimumTimePoint();
  bounds[1] = this->GetMaximumTimePoint();
  return bounds;
}

mitk::TimeBounds mitk::ArbitraryTimeGeometry::GetTimeBounds (TimeStepType step) const
{
  TimeBounds bounds;
  bounds[0] = this->GetMinimumTimePoint(step);
  bounds[1] = this->GetMaximumTimePoint(step);
  return bounds;
}

bool mitk::ArbitraryTimeGeometry::IsValidTimePoint (TimePointType timePoint) const
{
  return this->GetMinimumTimePoint() <= timePoint && timePoint < this->GetMaximumTimePoint();
}

bool mitk::ArbitraryTimeGeometry::IsValidTimeStep (TimeStepType timeStep) const
{
  return timeStep <  this->CountTimeSteps();
}

mitk::TimePointType mitk::ArbitraryTimeGeometry::TimeStepToTimePoint( TimeStepType timeStep) const
{
  TimePointType result = 0.0;

  if(timeStep == 0)
  {
    result = m_MinimumTimePoint;
  }
  else if(timeStep > 0 && timeStep < m_MaximumTimePoints.size())
  {
    result = m_MaximumTimePoints[timeStep-1];
  }

  return result;
}

mitk::TimeStepType mitk::ArbitraryTimeGeometry::TimePointToTimeStep( TimePointType timePoint) const
{
  mitk::TimeStepType result = 0;

  if (timePoint>=m_MinimumTimePoint)
  {
    for (std::vector<TimePointType>::const_iterator pos = m_MaximumTimePoints.begin(); pos != m_MaximumTimePoints.end(); ++pos)
    {
      if (timePoint<*pos)
      {
        result = pos - m_MaximumTimePoints.begin();
        break;
      }
    }
  }

  return result;
}

mitk::BaseGeometry::Pointer mitk::ArbitraryTimeGeometry::GetGeometryForTimeStep( TimeStepType timeStep) const
{
  if (IsValidTimeStep(timeStep))
  {
    return dynamic_cast<BaseGeometry*>(m_GeometryVector[timeStep].GetPointer());
  }
  else
  {
    return 0;
  }
}

mitk::BaseGeometry::Pointer mitk::ArbitraryTimeGeometry::GetGeometryForTimePoint(TimePointType timePoint) const
{
  if (this->IsValidTimePoint(timePoint))
  {
    TimeStepType timeStep = this->TimePointToTimeStep(timePoint);
    return this->GetGeometryForTimeStep(timeStep);
  }
  else
  {
    return 0;
  }
}


mitk::BaseGeometry::Pointer mitk::ArbitraryTimeGeometry::GetGeometryCloneForTimeStep( TimeStepType timeStep) const
{
  if (timeStep >= m_GeometryVector.size())
    return 0;
  return m_GeometryVector[timeStep]->Clone();
}

bool mitk::ArbitraryTimeGeometry::IsValid() const
{
  bool isValid = true;
  isValid &= m_GeometryVector.size() > 0;
  return isValid;
}

void mitk::ArbitraryTimeGeometry::ClearAllGeometries()
{
  m_GeometryVector.clear();
  m_MinimumTimePoint = 0;
  m_MaximumTimePoints.clear();
}

void mitk::ArbitraryTimeGeometry::ReserveSpaceForGeometries(TimeStepType numberOfGeometries)
{
  m_GeometryVector.reserve(numberOfGeometries);
  m_MaximumTimePoints.reserve(numberOfGeometries);
}

void mitk::ArbitraryTimeGeometry::Expand(mitk::TimeStepType size)
{
  m_GeometryVector.reserve(size);

  TimeBounds bounds;
  TimePointType minTP = this->GetMinimumTimePoint(this->CountTimeSteps()-1);
  TimePointType maxTP = this->GetMaximumTimePoint(this->CountTimeSteps()-1);
  TimePointType duration = maxTP-minTP;

  while  (m_GeometryVector.size() < size)
  {
    m_GeometryVector.push_back(Geometry3D::New().GetPointer());
    maxTP += duration;
    m_MaximumTimePoints.push_back(maxTP);
  }
}

void mitk::ArbitraryTimeGeometry::ReplaceTimeStepGeometries(const BaseGeometry* geometry)
{
    for (std::vector<BaseGeometry::Pointer>::iterator pos = m_GeometryVector.begin(); pos != m_GeometryVector.end(); ++pos)
    {
      *pos = geometry->Clone();
    }
}

void mitk::ArbitraryTimeGeometry::SetTimeStepGeometry(BaseGeometry* geometry, TimeStepType timeStep)
{
  assert(timeStep<m_GeometryVector.size());

  if (timeStep>=m_GeometryVector.size())
  {
    return;
  }

  m_GeometryVector[timeStep] = geometry;
}

itk::LightObject::Pointer mitk::ArbitraryTimeGeometry::InternalClone() const
{
  itk::LightObject::Pointer parent = Superclass::InternalClone();
  ArbitraryTimeGeometry::Pointer newTimeGeometry =
    dynamic_cast<ArbitraryTimeGeometry * > (parent.GetPointer());
  newTimeGeometry->m_MinimumTimePoint = this->m_MinimumTimePoint;
  newTimeGeometry->m_MaximumTimePoints = this->m_MaximumTimePoints;
  newTimeGeometry->m_GeometryVector.clear();
  for (TimeStepType i =0; i < CountTimeSteps(); ++i)
  {
    newTimeGeometry->m_GeometryVector.push_back(this->m_GeometryVector[i]->Clone());
  }
  newTimeGeometry->componentSize = componentSize;
  return parent;
}

void mitk::ArbitraryTimeGeometry::AppendTimeStep(BaseGeometry* geometry, TimePointType maximumTimePoint, TimePointType minimumTimePoint)
{
  if (!geometry)
  {
    mitkThrow() << "Cannot append geometry to time geometry. Invalid geometry passed (NULL pointer).";
  }

  if (!m_GeometryVector.empty())
  {
    if (m_MaximumTimePoints.back()>maximumTimePoint)
    {
      mitkThrow() << "Cannot append geometry to time geometry. Time bound conflict.";
    }
  }
  else
  {
    m_MinimumTimePoint = minimumTimePoint;
  }

  m_GeometryVector.push_back(geometry);
  m_MaximumTimePoints.push_back(maximumTimePoint);
}

void mitk::ArbitraryTimeGeometry::AppendTimeStepClone(const BaseGeometry* geometry, TimePointType maximumTimePoint, TimePointType minimumTimePoint)
{
  BaseGeometry::Pointer clone = geometry->Clone();

  this->AppendTimeStep(clone, maximumTimePoint, minimumTimePoint);
};


void mitk::ArbitraryTimeGeometry::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << " MinimumTimePoint: " << this->GetMinimumTimePoint() << " ms"<< std::endl;
  os << indent << " MaximumTimePoint: " << this->GetMaximumTimePoint() << " ms" << std::endl;

  os << std::endl;
  os << indent << " max TimeBounds: " << std::endl;
  for (TimeStepType i=0; i<m_MaximumTimePoints.size(); ++i)
  {
    os << indent.GetNextIndent() << "Step "<< i <<": "<<m_MaximumTimePoints[i] << " ms" << std::endl;
  }

}
