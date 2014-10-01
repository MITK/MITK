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
#include <limits>

mitk::ProportionalTimeGeometry::ProportionalTimeGeometry() :
  m_FirstTimePoint(0.0),
  m_StepDuration(1.0)
{
}

mitk::ProportionalTimeGeometry::~ProportionalTimeGeometry()
{
}

void mitk::ProportionalTimeGeometry::Initialize()
{
  m_FirstTimePoint = 0.0;
  m_StepDuration = 1.0;
  m_GeometryVector.resize(1);
}

mitk::TimeStepType mitk::ProportionalTimeGeometry::CountTimeSteps () const
{
  return static_cast<TimeStepType>(m_GeometryVector.size() );
}

mitk::TimePointType mitk::ProportionalTimeGeometry::GetMinimumTimePoint () const
{
  return m_FirstTimePoint;
}

mitk::TimePointType mitk::ProportionalTimeGeometry::GetMaximumTimePoint () const
{
  TimePointType timePoint = m_FirstTimePoint + m_StepDuration * CountTimeSteps();
  if (timePoint >std::numeric_limits<TimePointType>().max())
    timePoint = std::numeric_limits<TimePointType>().max();
  return timePoint;
}

mitk::TimeBounds mitk::ProportionalTimeGeometry::GetTimeBounds () const
{
  TimeBounds bounds;
  bounds[0] = this->GetMinimumTimePoint();
  bounds[1] = this->GetMaximumTimePoint();
  return bounds;
}

mitk::TimePointType  mitk::ProportionalTimeGeometry::GetMinimumTimePoint(TimeStepType step) const
{
  TimePointType timePoint;
  if (step == 0)
  {
    timePoint = m_FirstTimePoint;
  }
  else
  {
    timePoint = m_FirstTimePoint + m_StepDuration * step;
  }
  if (timePoint >std::numeric_limits<TimePointType>().max())
    timePoint = std::numeric_limits<TimePointType>().max();
  return timePoint;
}

mitk::TimePointType   mitk::ProportionalTimeGeometry::GetMaximumTimePoint(TimeStepType step) const
{
  TimePointType timePoint = m_FirstTimePoint + m_StepDuration * (step + 1);
  if (timePoint >std::numeric_limits<TimePointType>().max())
    timePoint = std::numeric_limits<TimePointType>().max();
  return timePoint;
}


mitk::TimeBounds mitk::ProportionalTimeGeometry::GetTimeBounds(TimeStepType step) const
{
  TimeBounds bounds;
  bounds[0] = this->GetMinimumTimePoint(step);
  bounds[1] = this->GetMaximumTimePoint(step);
  return bounds;
}

bool mitk::ProportionalTimeGeometry::IsValidTimePoint (TimePointType timePoint) const
{
  return this->GetMinimumTimePoint() <= timePoint && timePoint < this->GetMaximumTimePoint();
}

bool mitk::ProportionalTimeGeometry::IsValidTimeStep (TimeStepType timeStep) const
{
  return timeStep <  this->CountTimeSteps();
}

mitk::TimePointType mitk::ProportionalTimeGeometry::TimeStepToTimePoint( TimeStepType timeStep) const
{
  if (m_FirstTimePoint <= itk::NumericTraits<TimePointType>::NonpositiveMin() ||
      m_FirstTimePoint >= itk::NumericTraits<TimePointType>::max() ||
      m_StepDuration <= itk::NumericTraits<TimePointType>::min() ||
      m_StepDuration >= itk::NumericTraits<TimePointType>::max())
  {
    return static_cast<TimePointType>(timeStep);
  }

  return m_FirstTimePoint + timeStep * m_StepDuration;
}

mitk::TimeStepType mitk::ProportionalTimeGeometry::TimePointToTimeStep( TimePointType timePoint) const
{
  if (m_FirstTimePoint <= timePoint)
    return static_cast<TimeStepType>((timePoint -m_FirstTimePoint) / m_StepDuration);
  else
    return 0;
}

mitk::BaseGeometry::Pointer mitk::ProportionalTimeGeometry::GetGeometryForTimeStep( TimeStepType timeStep) const
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

mitk::BaseGeometry::Pointer mitk::ProportionalTimeGeometry::GetGeometryForTimePoint(TimePointType timePoint) const
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


mitk::BaseGeometry::Pointer mitk::ProportionalTimeGeometry::GetGeometryCloneForTimeStep( TimeStepType timeStep) const
{
  if (timeStep > m_GeometryVector.size())
    return 0;
  return m_GeometryVector[timeStep]->Clone();
}

bool mitk::ProportionalTimeGeometry::IsValid() const
{
  bool isValid = true;
  isValid &= m_GeometryVector.size() > 0;
  isValid &= m_StepDuration > 0;
  return isValid;
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
  if (m_GeometryVector.size() == 0)
  {
    while  (m_GeometryVector.size() < size)
    {
      Geometry3D::Pointer geo3D = Geometry3D::New();
      m_GeometryVector.push_back(dynamic_cast<BaseGeometry*>(geo3D.GetPointer()));
    }
  } else
  {
    while (m_GeometryVector.size() < size)
    {
      BaseGeometry::Pointer clone = dynamic_cast<BaseGeometry*>(m_GeometryVector[0]->Clone().GetPointer());
      m_GeometryVector.push_back(clone);
    }
  }
}

void mitk::ProportionalTimeGeometry::SetTimeStepGeometry(BaseGeometry* geometry, TimeStepType timeStep)
{
  assert(timeStep<=m_GeometryVector.size());

  if (timeStep == m_GeometryVector.size())
    m_GeometryVector.push_back(geometry);

  m_GeometryVector[timeStep] = geometry;
}

itk::LightObject::Pointer mitk::ProportionalTimeGeometry::InternalClone() const
{
  itk::LightObject::Pointer parent = Superclass::InternalClone();
  ProportionalTimeGeometry::Pointer newTimeGeometry =
    dynamic_cast<ProportionalTimeGeometry * > (parent.GetPointer());
  newTimeGeometry->m_FirstTimePoint = this->m_FirstTimePoint;
  newTimeGeometry->m_StepDuration = this->m_StepDuration;
  newTimeGeometry->m_GeometryVector.clear();
  newTimeGeometry->Expand(this->CountTimeSteps());
  for (TimeStepType i =0; i < CountTimeSteps(); ++i)
  {
    BaseGeometry::Pointer tempGeometry = GetGeometryForTimeStep(i)->Clone();
    newTimeGeometry->SetTimeStepGeometry(tempGeometry, i);
  }
  return parent;
}

void mitk::ProportionalTimeGeometry::Initialize (BaseGeometry* geometry, TimeStepType timeSteps)
{
  timeSteps = (timeSteps > 0) ? timeSteps : 1;
  m_FirstTimePoint = 0.0;
  m_StepDuration = 1.0;
  if (timeSteps < 2)
  {
    m_FirstTimePoint = -std::numeric_limits<double>::max();
    m_StepDuration = std::numeric_limits<mitk::TimePointType>().infinity();
  }

  this->ReserveSpaceForGeometries(timeSteps);
  try{
  for (TimeStepType currentStep = 0; currentStep < timeSteps; ++currentStep)
  {
    BaseGeometry::Pointer clonedGeometry = geometry->Clone();
    this->SetTimeStepGeometry(clonedGeometry, currentStep);
  }
  }
  catch (...)
  {
    MITK_INFO << "Cloning of geometry produced an error!";
  }
  Update();
}

void mitk::ProportionalTimeGeometry::Initialize (TimeStepType timeSteps)
{
  mitk::Geometry3D::Pointer geo3D = Geometry3D::New();
  mitk::BaseGeometry::Pointer geometry = dynamic_cast<BaseGeometry*>(geo3D.GetPointer());
  geometry->Initialize();

  this->Initialize(geometry.GetPointer(), timeSteps);
}

void mitk::ProportionalTimeGeometry::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << " TimeSteps: " << this->CountTimeSteps() << std::endl;
  os << indent << " FirstTimePoint: " << this->GetFirstTimePoint() << std::endl;
  os << indent << " StepDuration: " << this->GetStepDuration() << " ms" << std::endl;
  os << indent << " Time Bounds: " << this->GetTimeBounds()[0] << " - " << this->GetTimeBounds()[1] << std::endl;

  os << std::endl;
  os << indent << " GetGeometryForTimeStep(0): ";
  if(GetGeometryForTimeStep(0).IsNull())
    os << "NULL" << std::endl;
  else
    GetGeometryForTimeStep(0)->Print(os, indent);
}
