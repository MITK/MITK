/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTimeNavigationController.h>

#include <mitkRenderingManager.h>
#include <mitkVtkPropRenderer.h>

  mitk::TimeNavigationController::TimeNavigationController()
    : BaseController()
    , m_InputWorldTimeGeometry(mitk::TimeGeometry::ConstPointer())
    , m_BlockUpdate(false)
{
  typedef itk::SimpleMemberCommand<TimeNavigationController> SNCCommandType;
  SNCCommandType::Pointer timeStepperChangedCommand;

  timeStepperChangedCommand = SNCCommandType::New();
  timeStepperChangedCommand->SetCallbackFunction(this, &TimeNavigationController::SendTime);
  m_Stepper->AddObserver(itk::ModifiedEvent(), timeStepperChangedCommand);
  m_Stepper->SetUnitName("ms");
}

mitk::TimeNavigationController::~TimeNavigationController() {}

void mitk::TimeNavigationController::SetInputWorldTimeGeometry(const TimeGeometry* geometry)
{
  if (geometry != nullptr)
  {
    if (geometry->GetBoundingBoxInWorld()->GetDiagonalLength2() < eps)
    {
      itkWarningMacro("setting an empty bounding-box");
      geometry = nullptr;
    }
  }
  if (m_InputWorldTimeGeometry != geometry)
  {
    m_InputWorldTimeGeometry = geometry;
    this->Modified();
  }
}

void mitk::TimeNavigationController::Update()
{
  if (m_BlockUpdate)
  {
    return;
  }

  if (m_InputWorldTimeGeometry.IsNull())
  {
    return;
  }

  if (0 == m_InputWorldTimeGeometry->CountTimeSteps())
  {
    return;
  }

  m_BlockUpdate = true;

  if (m_LastUpdateTime < m_InputWorldTimeGeometry->GetMTime())
  {
    Modified();
  }

  if (m_LastUpdateTime < GetMTime())
  {
    m_LastUpdateTime = GetMTime();

    TimeStepType inputTimeSteps = m_InputWorldTimeGeometry->CountTimeSteps();
    const TimeBounds &timeBounds = m_InputWorldTimeGeometry->GetTimeBounds();
    m_Stepper->SetSteps(inputTimeSteps);
    m_Stepper->SetPos(0);
    m_Stepper->SetRange(timeBounds[0], timeBounds[1]);
  }

  // unblock update; we may do this now, because if m_BlockUpdate was already
  // true before this method was entered, then we will never come here.
  m_BlockUpdate = false;

  // Send the time. Do this even if nothing was changed, because maybe
  // Update() was only called to re-send the old time data.
  this->SendTime();
}

void mitk::TimeNavigationController::SendTime()
{
  if (!m_BlockUpdate)
  {
    if (m_InputWorldTimeGeometry.IsNotNull())
    {
      this->InvokeEvent(TimeEvent(m_Stepper->GetPos()));
      RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

mitk::TimeStepType mitk::TimeNavigationController::GetSelectedTimeStep() const
{
  return m_Stepper->GetPos();
}

mitk::TimePointType mitk::TimeNavigationController::GetSelectedTimePoint() const
{
  const auto timeStep = this->GetSelectedTimeStep();

  if (m_InputWorldTimeGeometry.IsNull())
  {
    return 0.0;
  }

  if (!m_InputWorldTimeGeometry->IsValidTimeStep(timeStep))
  {
    mitkThrow() << "SliceNavigationController is in an invalid state. It has a time step"
                << "selected that is not covered by its time geometry. Selected time step: " << timeStep
                << "; TimeGeometry steps count: " << m_InputWorldTimeGeometry->CountTimeSteps();
  }

  return m_InputWorldTimeGeometry->TimeStepToTimePoint(timeStep);
}
