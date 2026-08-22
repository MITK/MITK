/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkProgressService.h"

#include <algorithm>

namespace
{
  unsigned int ClampProgress(unsigned int progress, unsigned int steps)
  {
    return 0 != steps
      ? std::min(progress, steps)
      : 0;
  }
}

mitk::ProgressService::ProgressService()
  : m_NextId(0),
    m_NextSequence(0)
{
}

mitk::ProgressService::~ProgressService()
{
}

void mitk::ProgressService::Notify(const Registrations& registrations, const ProgressTaskInfo& info)
{
  for (const auto& registration : registrations)
  {
    // Blocks RemoveListener() until this listener returns, which is what makes
    // it safe to destroy a listener right after removing it. Never take
    // m_Mutex while holding this one; RemoveListener() acquires them in the
    // opposite order and the pair would deadlock.
    std::scoped_lock lock(registration->Mutex);

    if (nullptr != registration->Listener)
      registration->Listener->OnTaskUpdated(info);
  }
}

bool mitk::ProgressService::AddListener(IProgressListener* listener)
{
  if (nullptr == listener)
    return false;

  std::shared_ptr<Registration> registration;
  std::vector<ProgressTaskInfo> activeTasks;

  {
    std::scoped_lock lock(m_Mutex);

    auto it = std::find_if(m_Registrations.begin(), m_Registrations.end(), [listener](const auto& candidate) {
      return candidate->Listener == listener;
    });

    if (it != m_Registrations.end())
      return true;

    registration = std::make_shared<Registration>();
    registration->Listener = listener;
    m_Registrations.push_back(registration);

    activeTasks.reserve(m_Tasks.size());

    for (const auto& [id, task] : m_Tasks)
      activeTasks.push_back(task.Info);
  }

  // Catch the new listener up on what is already running. Any newer snapshot
  // that overtakes this replay wins, as its sequence number is higher.
  for (const auto& info : activeTasks)
    Notify({ registration }, info);

  return true;
}

bool mitk::ProgressService::RemoveListener(const IProgressListener* listener)
{
  if (nullptr == listener)
    return false;

  std::shared_ptr<Registration> registration;

  {
    std::scoped_lock lock(m_Mutex);

    auto it = std::find_if(m_Registrations.begin(), m_Registrations.end(), [listener](const auto& candidate) {
      return candidate->Listener == listener;
    });

    if (it == m_Registrations.end())
      return false;

    registration = *it;
    m_Registrations.erase(it);
  }

  // m_Mutex is released above on purpose: Notify() takes the registration
  // mutex without holding m_Mutex, so holding both here would invert the order
  // and deadlock.
  std::scoped_lock lock(registration->Mutex);
  registration->Listener = nullptr;

  return true;
}

std::vector<mitk::ProgressTaskInfo> mitk::ProgressService::GetActiveTasks() const
{
  std::scoped_lock lock(m_Mutex);

  std::vector<ProgressTaskInfo> activeTasks;
  activeTasks.reserve(m_Tasks.size());

  for (const auto& [id, task] : m_Tasks)
    activeTasks.push_back(task.Info);

  return activeTasks;
}

void mitk::ProgressService::RequestCancel(ProgressTaskId id)
{
  ProgressTaskInfo info;
  Registrations registrations;

  {
    std::scoped_lock lock(m_Mutex);

    auto it = m_Tasks.find(id);

    if (it == m_Tasks.end() || !it->second.Info.Cancelable || it->second.Info.CancelRequested)
      return;

    it->second.State->CancelRequested.store(true, std::memory_order_relaxed);
    it->second.Info.CancelRequested = true;
    it->second.Info.Sequence = ++m_NextSequence;

    info = it->second.Info;
    registrations = m_Registrations;
  }

  Notify(registrations, info);
}

std::shared_ptr<mitk::ProgressTaskState> mitk::ProgressService::StartTask(const std::string& name,
                                                                         unsigned int steps,
                                                                         bool cancelable)
{
  ProgressTaskInfo info;
  Registrations registrations;
  std::shared_ptr<ProgressTaskState> state;

  {
    std::scoped_lock lock(m_Mutex);

    const auto id = ++m_NextId;
    state = std::make_shared<ProgressTaskState>(id);

    info.Id = id;
    info.Sequence = ++m_NextSequence;
    info.Name = name;
    info.StepsToDo = steps;
    info.Cancelable = cancelable;

    m_Tasks[id] = Task{ state, info };
    registrations = m_Registrations;
  }

  Notify(registrations, info);

  return state;
}

void mitk::ProgressService::UpdateTask(ProgressTaskId id,
                                       const std::string& name,
                                       unsigned int steps,
                                       unsigned int progress)
{
  ProgressTaskInfo info;
  Registrations registrations;

  {
    std::scoped_lock lock(m_Mutex);

    auto it = m_Tasks.find(id);

    if (it == m_Tasks.end())
      return;

    it->second.Info.Name = name;
    it->second.Info.StepsToDo = steps;
    it->second.Info.Progress = ClampProgress(progress, steps);
    it->second.Info.Sequence = ++m_NextSequence;

    info = it->second.Info;
    registrations = m_Registrations;
  }

  Notify(registrations, info);
}

void mitk::ProgressService::FinishTask(ProgressTaskId id)
{
  ProgressTaskInfo info;
  Registrations registrations;

  {
    std::scoped_lock lock(m_Mutex);

    auto it = m_Tasks.find(id);

    if (it == m_Tasks.end())
      return;

    info = it->second.Info;
    info.Sequence = ++m_NextSequence;
    info.Finished = true;

    // A task abandoned halfway still ends on a full bar. Reporting the step it
    // died on would leave the user staring at an operation that never
    // completes.
    info.Progress = info.StepsToDo;

    m_Tasks.erase(it);
    registrations = m_Registrations;
  }

  Notify(registrations, info);
}
