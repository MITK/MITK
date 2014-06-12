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

mitk::ArbitraryTimeGeometry::ArbitraryTimeGeometry(): m_MinimumTimePoint(0), m_MaximumTimePoint(0)
{
}

mitk::ArbitraryTimeGeometry::~ArbitraryTimeGeometry()
{
}

void mitk::ArbitraryTimeGeometry::Initialize()
{
  this->ClearAllGeometries();
  Geometry3D::Pointer geo = Geometry3D::New();
  TimeBounds bounds;
  bounds[0] = 0;
  bounds[1] = 1;
  geo->SetTimeBounds(bounds);
  this->AppendTimeStep(geo);
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
  return m_MaximumTimePoint;
}

mitk::TimeBounds mitk::ArbitraryTimeGeometry::GetTimeBounds () const
{
  TimeBounds bounds;
  bounds[0] = this->GetMinimumTimePoint();
  bounds[1] = this->GetMaximumTimePoint();
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
  TimePointType result = 0;

  if (timeStep < m_GeometryVector.size())
  {
    result = m_GeometryVector[timeStep]->GetTimeBounds()[0];
  }

  return result;
}

class HasLargerTimeBoundPredicate
{
public:
  HasLargerTimeBoundPredicate(const mitk::TimePointType& timePoint): m_TimePoint(timePoint)
  {};

  mitk::TimePointType m_TimePoint;
  bool operator() (const mitk::Geometry3D::Pointer& geometry)
  {
    return geometry->GetTimeBounds()[0]>m_TimePoint;
  }
};

mitk::TimeStepType mitk::ArbitraryTimeGeometry::TimePointToTimeStep( TimePointType timePoint) const
{
  if (m_MinimumTimePoint <= timePoint && m_MaximumTimePoint > timePoint)
  {
    std::vector<Geometry3D::Pointer>::const_iterator finding = ::std::find_if(m_GeometryVector.begin(), m_GeometryVector.end(), HasLargerTimeBoundPredicate(timePoint));

    mitk::TimeStepType result = finding - m_GeometryVector.begin() - 1; //index of the correct time step -1;
    return result;
  }
  else
    return 0;
}

mitk::Geometry3D::Pointer mitk::ArbitraryTimeGeometry::GetGeometryForTimeStep( TimeStepType timeStep) const
{
  if (IsValidTimeStep(timeStep))
  {
    return dynamic_cast<Geometry3D*>(m_GeometryVector[timeStep].GetPointer());
  }
  else
  {
    return 0;
  }
}

mitk::Geometry3D::Pointer mitk::ArbitraryTimeGeometry::GetGeometryForTimePoint(TimePointType timePoint) const
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


mitk::Geometry3D::Pointer mitk::ArbitraryTimeGeometry::GetGeometryCloneForTimeStep( TimeStepType timeStep) const
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
  m_MaximumTimePoint = 0;
}

void mitk::ArbitraryTimeGeometry::ReserveSpaceForGeometries(TimeStepType numberOfGeometries)
{
  m_GeometryVector.reserve(numberOfGeometries);
}

void mitk::ArbitraryTimeGeometry::Expand(mitk::TimeStepType size)
{
  m_GeometryVector.reserve(size);
  while  (m_GeometryVector.size() < size)
  {
    m_GeometryVector.push_back(Geometry3D::New());
  }
}

void mitk::ArbitraryTimeGeometry::SetTimeStepGeometry(Geometry3D* geometry, TimeStepType timeStep)
{
  assert(timeStep<m_GeometryVector.size());
  if (timeStep>=m_GeometryVector.size())
  {
    return;
  }

  m_GeometryVector[timeStep] = geometry;

  //Now clean up the vector and remove conflicted steps.
  for (TimeStepType pos = m_GeometryVector.size()-1; pos != 0; --pos)
  {
    bool isConflicted = ((pos>timeStep) &&  (m_GeometryVector[pos]->GetTimeBounds()[0]< geometry->GetTimeBounds()[0])) ||
      ((pos<timeStep) && (m_GeometryVector[pos]->GetTimeBounds()[0]> geometry->GetTimeBounds()[0]));

    if (isConflicted)
      {
        m_GeometryVector.erase(m_GeometryVector.begin()+pos);
      }
  }

  m_MinimumTimePoint = m_GeometryVector.front()->GetTimeBounds()[0];
  m_MaximumTimePoint = m_GeometryVector.back()->GetTimeBounds()[1];
}

itk::LightObject::Pointer mitk::ArbitraryTimeGeometry::InternalClone() const
{
  itk::LightObject::Pointer parent = Superclass::InternalClone();
  ArbitraryTimeGeometry::Pointer newTimeGeometry =
    dynamic_cast<ArbitraryTimeGeometry * > (parent.GetPointer());
  newTimeGeometry->m_MinimumTimePoint = this->m_MinimumTimePoint;
  newTimeGeometry->m_MaximumTimePoint = this->m_MaximumTimePoint;
  newTimeGeometry->m_GeometryVector.clear();
  newTimeGeometry->ReserveSpaceForGeometries(this->CountTimeSteps());
  for (TimeStepType i =0; i < CountTimeSteps(); ++i)
  {
    Geometry3D::Pointer tempGeometry = GetGeometryForTimeStep(i)->Clone();
    newTimeGeometry->AppendTimeStepClone(m_GeometryVector[i]);
  }
  return parent;
}

void mitk::ArbitraryTimeGeometry::AppendTimeStep(Geometry3D* geometry)
{
  if (!geometry)
  {
    mitkThrow() << "Cannot append geometry to time geometry. Invalid geometry passed (NULL pointer).";
  }

  if (!m_GeometryVector.empty())
  {
    if (m_GeometryVector.back()->GetTimeBounds()[0]>geometry->GetTimeBounds()[0])
    {
      mitkThrow() << "Cannot append geometry to time geometry. Time bound conflict.";
    }
  }

  m_GeometryVector.push_back(geometry);
  m_MinimumTimePoint = m_GeometryVector.front()->GetTimeBounds()[0];
  m_MaximumTimePoint = m_GeometryVector.back()->GetTimeBounds()[1];
}

void mitk::ArbitraryTimeGeometry::AppendTimeStepClone(Geometry3D* geometry)
{
  Geometry3D::Pointer clone = geometry->Clone();

  this->AppendTimeStep(clone);
};


void mitk::ArbitraryTimeGeometry::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << " MinimumTimePoint: " << this->GetMinimumTimePoint() << " ms"<< std::endl;
  os << indent << " MaximumTimePoint: " << this->GetMaximumTimePoint() << " ms" << std::endl;

  os << std::endl;
  os << indent << " TimeBounds: " << std::endl;
  for (TimeStepType i=0; i<m_GeometryVector.size(); ++i)
  {
    os << indent.GetNextIndent() << "Step "<< i <<": lower: "<<m_GeometryVector[i]->GetTimeBounds()[0] << " ms; upper: "<<m_GeometryVector[i]->GetTimeBounds()[1] << " ms" << std::endl;
  }

}
