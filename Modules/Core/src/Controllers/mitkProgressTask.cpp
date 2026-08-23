/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkProgressTask.h>

#include <mitkCoreServices.h>
#include <mitkIProgressService.h>

#include <algorithm>
#include <limits>
#include <utility>

namespace
{
  mitk::IProgressService* AcquireProgressService()
  {
    try
    {
      return mitk::CoreServices::GetProgressService();
    }
    catch (...)
    {
      // Reporting progress must never be the reason an operation fails, so an
      // unavailable service degrades to no reporting at all.
      return nullptr;
    }
  }
}

mitk::ProgressTask::ProgressTask(const std::string& name, unsigned int steps, bool cancelable)
  : m_Service(AcquireProgressService()),
    m_Name(name),
    m_StepsToDo(steps),
    m_Progress(0)
{
  if (nullptr != m_Service)
    m_State = m_Service->StartTask(name, steps, cancelable);
}

mitk::ProgressTask::ProgressTask(std::function<void(float)> report, unsigned int steps)
  : m_Service(nullptr),
    m_Report(std::move(report)),
    m_StepsToDo(steps),
    m_Progress(0)
{
}

mitk::ProgressTask::~ProgressTask()
{
  this->Finish();
}

mitk::ProgressTask::ProgressTask(ProgressTask&& other) noexcept
  : m_Service(other.m_Service),
    m_State(std::move(other.m_State)),
    m_Report(std::move(other.m_Report)),
    m_Name(std::move(other.m_Name)),
    m_StepsToDo(other.m_StepsToDo),
    m_Progress(other.m_Progress)
{
  other.m_Service = nullptr;
  other.m_StepsToDo = 0;
  other.m_Progress = 0;
}

mitk::ProgressTask& mitk::ProgressTask::operator=(ProgressTask&& other) noexcept
{
  if (this != &other)
  {
    this->Finish();

    m_Service = other.m_Service;
    m_State = std::move(other.m_State);
    m_Report = std::move(other.m_Report);
    m_Name = std::move(other.m_Name);
    m_StepsToDo = other.m_StepsToDo;
    m_Progress = other.m_Progress;

    other.m_Service = nullptr;
    other.m_StepsToDo = 0;
    other.m_Progress = 0;
  }

  return *this;
}

mitk::ProgressTaskId mitk::ProgressTask::GetId() const
{
  return m_State
    ? m_State->Id
    : 0;
}

void mitk::ProgressTask::SetName(const std::string& name)
{
  m_Name = name;
  this->Publish();
}

void mitk::ProgressTask::AddStepsToDo(unsigned int steps)
{
  if (0 == steps)
    return;

  constexpr auto maxSteps = std::numeric_limits<unsigned int>::max();

  // Saturate instead of wrapping: a wrapped total would make the task appear
  // to jump backwards.
  m_StepsToDo = maxSteps - m_StepsToDo < steps
    ? maxSteps
    : m_StepsToDo + steps;

  this->Publish();
}

void mitk::ProgressTask::Progress(unsigned int steps)
{
  const auto remaining = m_StepsToDo > m_Progress
    ? m_StepsToDo - m_Progress
    : 0u;

  this->SetProgress(m_Progress + std::min(steps, remaining));
}

void mitk::ProgressTask::SetProgress(unsigned int progress)
{
  m_Progress = 0 != m_StepsToDo
    ? std::min(progress, m_StepsToDo)
    : 0;

  this->Publish();
}

unsigned int mitk::ProgressTask::GetStepsToDo() const
{
  return m_StepsToDo;
}

bool mitk::ProgressTask::IsCancelRequested() const
{
  return m_State && m_State->CancelRequested.load(std::memory_order_relaxed);
}

void mitk::ProgressTask::Finish() noexcept
{
  if (m_Report)
  {
    try
    {
      m_Report(1.0f);
    }
    catch (...)
    {
    }

    m_Report = nullptr;
    return;
  }

  if (nullptr == m_Service)
    return;

  try
  {
    if (m_State)
      m_Service->FinishTask(m_State->Id);

    m_State.reset();

    CoreServices::Unget(m_Service);
  }
  catch (...)
  {
    // Nothing actionable is left to do while a task is being torn down, and
    // this runs during stack unwinding often enough that throwing would turn
    // an ordinary error into a std::terminate.
  }

  m_Service = nullptr;
}

void mitk::ProgressTask::Publish()
{
  if (m_Report)
  {
    m_Report(0 != m_StepsToDo
      ? static_cast<float>(m_Progress) / m_StepsToDo
      : 0.0f);

    return;
  }

  if (nullptr != m_Service && m_State)
    m_Service->UpdateTask(m_State->Id, m_Name, m_StepsToDo, m_Progress);
}
