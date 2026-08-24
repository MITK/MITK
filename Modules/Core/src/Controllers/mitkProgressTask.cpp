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
  if (nullptr == m_Service)
    return;

  try
  {
    m_State = m_Service->StartTask(name, steps, cancelable);
  }
  catch (...)
  {
    // The destructor never runs for an object whose constructor threw, so the
    // service reference has to be given back here or it is held for the life
    // of the process. Degrades to no reporting for the same reason as above.
    this->Finish();
  }
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

  // A moved-from std::function is valid but not necessarily empty, and one
  // that survived the move would let the moved-from handle report completion
  // a second time when it is destroyed.
  other.m_Report = nullptr;

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
    other.m_Report = nullptr;
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
  // Never backwards, for the same reason the service enforces it on what it
  // publishes: phases mapped onto shares of one budget report an absolute
  // value each and do not always end in the order they began. Enforced here as
  // well so that the handle agrees with what the listeners were told; a handle
  // left behind the published value would compute the increments of a later
  // Progress() call from a number nobody ever saw.
  m_Progress = 0 != m_StepsToDo
    ? std::min(std::max(progress, m_Progress), m_StepsToDo)
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
  try
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
  catch (...)
  {
    // Reported from inside ITK, VTK and Poco observer dispatch, none of which
    // expects an exception to come back out of a progress callback. Reporting
    // progress must never be the reason an operation fails, which is what the
    // constructor and Finish() already assume.
  }
}

mitk::ProgressTask mitk::MakeProgressShare(ProgressTask* task, unsigned int steps)
{
  if (nullptr == task)
  {
    // An empty callback and no service: reports nowhere, so that a nested
    // operation can take a share unconditionally without checking first.
    return ProgressTask(std::function<void(float)>(), steps);
  }

  return ProgressTask([task, steps, reported = 0u](float progress) mutable
    {
      const auto reached =
        static_cast<unsigned int>(std::clamp(progress, 0.0f, 1.0f) * steps);

      if (reached > reported)
      {
        task->Progress(reached - reported);
        reported = reached;
      }
    },
    steps);
}
