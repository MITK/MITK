/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <limits>
#include <mitkArbitraryTimeGeometry.h>

#include <algorithm>

#include <mitkGeometry3D.h>

mitk::ArbitraryTimeGeometry::ArbitraryTimeGeometry() = default;

mitk::ArbitraryTimeGeometry::~ArbitraryTimeGeometry() = default;

void mitk::ArbitraryTimeGeometry::Initialize()
{
  this->ClearAllGeometries();
  Geometry3D::Pointer geo = Geometry3D::New();
  geo->Initialize();

  this->AppendNewTimeStep(geo, 0, 1);

  Update();
}

mitk::TimeStepType mitk::ArbitraryTimeGeometry::CountTimeSteps() const
{
  return static_cast<TimeStepType>(m_GeometryVector.size());
}

mitk::TimePointType mitk::ArbitraryTimeGeometry::GetMinimumTimePoint() const
{
  return m_MinimumTimePoints.empty() ? 0.0 : m_MinimumTimePoints.front();
}

mitk::TimePointType mitk::ArbitraryTimeGeometry::GetMaximumTimePoint() const
{
  TimePointType result = 0;
  if ( !m_MaximumTimePoints.empty() )
  {
    result = m_MaximumTimePoints.back();
  }

  ///////////////////////////////////////
  // Workarround T27883. See https://phabricator.mitk.org/T27883#219473 for more details.
  // This workarround should be removed as soon as T28262 is solved!
  if (this->HasCollapsedFinalTimeStep())
  {
    result = m_MinimumTimePoints.back() + 1;
  }
  // End of workarround for T27883
  //////////////////////////////////////

  return result;
}

mitk::TimePointType mitk::ArbitraryTimeGeometry::GetMinimumTimePoint( TimeStepType step ) const
{
  return step < m_MinimumTimePoints.size() ? m_MinimumTimePoints[step] : 0.0f;
};

mitk::TimePointType mitk::ArbitraryTimeGeometry::GetMaximumTimePoint( TimeStepType step ) const
{
  TimePointType result = 0;
  if (step < m_MaximumTimePoints.size())
  {
    result = m_MaximumTimePoints[step];
  }

  ///////////////////////////////////////
  // Workarround T27883. See https://phabricator.mitk.org/T27883#219473 for more details.
  // This workarround should be removed as soon as T28262 is solved!
  if (step + 1 == m_MaximumTimePoints.size() && this->HasCollapsedFinalTimeStep())
  {
    result = m_MinimumTimePoints[step] + 1;
  }
  // End of workarround for T27883
  //////////////////////////////////////

  return result;
};

mitk::TimeBounds mitk::ArbitraryTimeGeometry::GetTimeBounds() const
{
  TimeBounds bounds;
  bounds[0] = this->GetMinimumTimePoint();
  bounds[1] = this->GetMaximumTimePoint();
  return bounds;
}

mitk::TimeBounds mitk::ArbitraryTimeGeometry::GetTimeBounds(TimeStepType step) const
{
  TimeBounds bounds;
  bounds[0] = this->GetMinimumTimePoint( step );
  bounds[1] = this->GetMaximumTimePoint( step );
  return bounds;
}

bool mitk::ArbitraryTimeGeometry::IsValidTimePoint(TimePointType timePoint) const
{
  return this->GetMinimumTimePoint() <= timePoint &&
    (timePoint < this->GetMaximumTimePoint() || (this->HasCollapsedFinalTimeStep() && timePoint <= this->GetMaximumTimePoint()));
}

bool mitk::ArbitraryTimeGeometry::IsValidTimeStep(TimeStepType timeStep) const
{
  return timeStep < this->CountTimeSteps();
}

mitk::TimePointType mitk::ArbitraryTimeGeometry::TimeStepToTimePoint( TimeStepType timeStep ) const
{
  TimePointType result = 0.0;

  if (timeStep < m_MinimumTimePoints.size() )
  {
    result = m_MinimumTimePoints[timeStep];
  }

  return result;
}

mitk::TimeStepType mitk::ArbitraryTimeGeometry::TimePointToTimeStep(TimePointType timePoint) const
{
  mitk::TimeStepType result = 0;

  if (timePoint >= GetMinimumTimePoint())
  {
    for (auto pos = m_MaximumTimePoints.cbegin(); pos != m_MaximumTimePoints.cend(); ++pos)
    {
      ///////////////////////////////////////
      // Workarround T27883. See https://phabricator.mitk.org/T27883#219473 for more details.
      // The part ("+1.") inline marked as workarround should be removed as soon as T28262 is solved!
      if (timePoint < *pos
          || (pos == std::prev(m_MaximumTimePoints.cend())
              && timePoint <= *pos +1//<- +1 is the workarround
              && this->HasCollapsedFinalTimeStep()))
      {
        break;
      }
      // End of workarround for T27883
      //////////////////////////////////////

      ++result;
    }
  }

  return result;
}

mitk::BaseGeometry::Pointer mitk::ArbitraryTimeGeometry::GetGeometryForTimeStep(TimeStepType timeStep) const
{
  if ( IsValidTimeStep( timeStep ) )
  {
    return m_GeometryVector[timeStep];
  }
  else
  {
    return nullptr;
  }
}

mitk::BaseGeometry::Pointer
  mitk::ArbitraryTimeGeometry::GetGeometryForTimePoint( TimePointType timePoint ) const
{
  if ( this->IsValidTimePoint( timePoint ) )
  {
    const TimeStepType timeStep = this->TimePointToTimeStep( timePoint );
    return this->GetGeometryForTimeStep( timeStep );
  }
  else
  {
    return nullptr;
  }
}

mitk::BaseGeometry::Pointer
  mitk::ArbitraryTimeGeometry::GetGeometryCloneForTimeStep( TimeStepType timeStep ) const
{
  if ( timeStep >= m_GeometryVector.size() )
    return nullptr;
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
  m_MinimumTimePoints.clear();
  m_MaximumTimePoints.clear();
}

void mitk::ArbitraryTimeGeometry::ReserveSpaceForGeometries( TimeStepType numberOfGeometries )
{
  m_GeometryVector.reserve( numberOfGeometries );
  m_MinimumTimePoints.reserve( numberOfGeometries );
  m_MaximumTimePoints.reserve( numberOfGeometries );
}

void mitk::ArbitraryTimeGeometry::Expand( mitk::TimeStepType size )
{
  m_GeometryVector.reserve( size );

  const mitk::TimeStepType lastIndex = this->CountTimeSteps() - 1;
  const TimePointType minTP    = this->GetMinimumTimePoint( lastIndex );
  TimePointType maxTP          = this->GetMaximumTimePoint( lastIndex );
  const TimePointType duration = maxTP - minTP;

  while (m_GeometryVector.size() < size)
  {
    m_GeometryVector.push_back( Geometry3D::New().GetPointer() );
    m_MinimumTimePoints.push_back( maxTP );
    maxTP += duration;
    m_MaximumTimePoints.push_back( maxTP );
  }
}

void mitk::ArbitraryTimeGeometry::ReplaceTimeStepGeometries(const BaseGeometry *geometry)
{
  for ( auto pos = m_GeometryVector.begin(); pos != m_GeometryVector.end(); ++pos )
  {
    *pos = geometry->Clone();
  }
}

void mitk::ArbitraryTimeGeometry::SetTimeStepGeometry(BaseGeometry *geometry, TimeStepType timeStep)
{
  assert( timeStep <= m_GeometryVector.size() );

  if ( timeStep == m_GeometryVector.size() )
  {
    m_GeometryVector.push_back( geometry );
  }

  m_GeometryVector[timeStep] = geometry;
}

itk::LightObject::Pointer mitk::ArbitraryTimeGeometry::InternalClone() const
{
  itk::LightObject::Pointer parent = Superclass::InternalClone();
  ArbitraryTimeGeometry::Pointer newTimeGeometry = dynamic_cast<ArbitraryTimeGeometry *>(parent.GetPointer());
  newTimeGeometry->m_MinimumTimePoints = this->m_MinimumTimePoints;
  newTimeGeometry->m_MaximumTimePoints = this->m_MaximumTimePoints;
  newTimeGeometry->m_GeometryVector.clear();
  for (TimeStepType i = 0; i < CountTimeSteps(); ++i)
  {
    newTimeGeometry->m_GeometryVector.push_back( this->m_GeometryVector[i]->Clone() );
  }
  return parent;
}

void mitk::ArbitraryTimeGeometry::AppendNewTimeStep(BaseGeometry *geometry,
  TimePointType minimumTimePoint,
  TimePointType maximumTimePoint)
{
  if ( !geometry )
  {
    mitkThrow() << "Cannot append geometry to time geometry. Invalid geometry passed (nullptr pointer).";
  }

  if (maximumTimePoint < minimumTimePoint)
  {
    mitkThrow() << "Cannot append geometry to time geometry. Time bound conflict. Maxmimum time point ("<<maximumTimePoint<<") is smaller than minimum time point ("<<minimumTimePoint<<").";
  }

  if ( !m_GeometryVector.empty() )
  {
    if ( m_MaximumTimePoints.back() > minimumTimePoint )
    {
      mitkThrow() << "Cannot append geometry to time geometry. Time bound conflict new time point and currently last time point overlapp.";
    }
  }

  m_GeometryVector.push_back( geometry );
  m_MinimumTimePoints.push_back( minimumTimePoint );
  m_MaximumTimePoints.push_back( maximumTimePoint );
}

void mitk::ArbitraryTimeGeometry::AppendNewTimeStepClone(const BaseGeometry *geometry,
                                                      TimePointType minimumTimePoint,
                                                      TimePointType maximumTimePoint)
{
  BaseGeometry::Pointer clone = geometry->Clone();

  this->AppendNewTimeStep(clone, minimumTimePoint, maximumTimePoint);
};

void mitk::ArbitraryTimeGeometry::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << " MinimumTimePoint: " << this->GetMinimumTimePoint() << " ms" << std::endl;
  os << indent << " MaximumTimePoint: " << this->GetMaximumTimePoint() << " ms" << std::endl;

  os << std::endl;
  os << indent << " min TimeBounds: " << std::endl;
  for (TimeStepType i = 0; i < m_MinimumTimePoints.size(); ++i)
  {
    os << indent.GetNextIndent() << "Step " << i << ": " << m_MinimumTimePoints[i] << " ms" << std::endl;
  }
  os << std::endl;
  os << indent << " max TimeBounds: " << std::endl;
  for (TimeStepType i = 0; i < m_MaximumTimePoints.size(); ++i)
  {
    os << indent.GetNextIndent() << "Step " << i << ": " << m_MaximumTimePoints[i] << " ms" << std::endl;
  }
  ///////////////////////////////////////
  // Workarround T27883. See https://phabricator.mitk.org/T27883#219473 for more details.
  // This workarround should be removed as soon as T28262 is solved!
  if (this->HasCollapsedFinalTimeStep())
  {
    os << indent << "Caution: This time geometry has a collapsed finale time step." << std::endl;
    os << indent << "         Most likely reason is that no duration could be deduced from the original data" << std::endl;
    os << indent << "         (e.g. DICOM dynamic series stored as single frame images)." << std::endl;
    os << indent << "         Currently we expand it by 1 ms (see T27883 for more details)." << std::endl;
  }
  // End of workarround for T27883
  //////////////////////////////////////
}

bool mitk::ArbitraryTimeGeometry::HasCollapsedFinalTimeStep() const
{
  bool result = false;

  if (!m_MaximumTimePoints.empty() && !m_MinimumTimePoints.empty())
  {
    result = m_MinimumTimePoints.back() == m_MaximumTimePoints.back();
  }

  return result;
}
