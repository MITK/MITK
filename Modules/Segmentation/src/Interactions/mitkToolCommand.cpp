/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkToolCommand.h>

#include <mitkProgressTask.h>

#include <itkProcessObject.h>

#include <algorithm>

mitk::ToolCommand::ToolCommand()
  : m_ProgressTask(nullptr),
    m_ShareIndex(0),
    m_ShareCount(1)
{
}

void mitk::ToolCommand::SetProgressTask(ProgressTask *task)
{
  m_ProgressTask = task;

  // A share belongs to the run it was set for, and a new task means a new
  // operation. Reset here rather than left to the caller, which would only
  // ever show up as a bar that stops halfway.
  m_ShareIndex = 0;
  m_ShareCount = 1;
}

void mitk::ToolCommand::SetShare(unsigned int index, unsigned int count)
{
  m_ShareCount = std::max(count, 1u);
  m_ShareIndex = std::min(index, m_ShareCount - 1);
}

void mitk::ToolCommand::Execute(itk::Object *caller, const itk::EventObject &event)
{
  const itk::Object *constCaller = caller;
  this->Execute(constCaller, event);

  if (nullptr != m_ProgressTask && m_ProgressTask->IsCancelRequested())
  {
    // Aborting makes the filter throw itk::ProcessAborted once it reaches the
    // next check, which callers have to tell apart from a genuine failure.
    if (auto *process = dynamic_cast<itk::ProcessObject *>(caller); nullptr != process)
      process->SetAbortGenerateData(true);
  }
}

void mitk::ToolCommand::Execute(const itk::Object *caller, const itk::EventObject &)
{
  if (nullptr == m_ProgressTask)
    return;

  const auto *process = dynamic_cast<const itk::ProcessObject *>(caller);

  if (nullptr == process)
    return;

  // The filter reports a fraction of its own run, which is mapped onto this
  // run's share of whatever budget the task was given. Counting one step per
  // event, as before, made the reported progress depend on how chatty a filter
  // happens to be.
  const auto fraction = (m_ShareIndex + std::clamp(process->GetProgress(), 0.0f, 1.0f)) / m_ShareCount;

  m_ProgressTask->SetProgress(static_cast<unsigned int>(fraction * m_ProgressTask->GetStepsToDo()));
}
