/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLog.h>
#include <mitkLogBackendCout.h>

#include <list>
#include <set>

static std::list<mitk::LogBackendBase*> backends;
static std::set<mitk::LogBackendBase::OutputType> disabledBackendTypes;

void mitk::RegisterBackend(LogBackendBase* backend)
{
  backends.push_back(backend);
}

void mitk::UnregisterBackend(LogBackendBase* backend)
{
  backends.remove(backend);
}

void mitk::DistributeToBackends(LogMessage& message)
{
  // Crop Message
  {
    std::string::size_type i = message.Message.find_last_not_of(" \t\f\v\n\r");

    message.Message = i != std::string::npos
      ? message.Message.substr(0, i + 1)
      : "";
  }

  // create dummy backend if there is no backend registered (so we have an output anyway)
  static LogBackendCout* dummyBackend = nullptr;

  if (backends.empty() && dummyBackend == nullptr)
  {
    dummyBackend = new LogBackendCout;
    RegisterBackend(dummyBackend);
  }
  else if (backends.size() > 1 && dummyBackend != nullptr)
  {
    // if there was added another backend remove the dummy backend and delete it
    UnregisterBackend(dummyBackend);
    delete dummyBackend;
    dummyBackend = nullptr;
  }

  // iterate through all registered images and call the ProcessMessage() methods of the backends
  for (auto i = backends.begin(); i != backends.end(); ++i)
  {
    if (IsBackendEnabled((*i)->GetOutputType()))
      (*i)->ProcessMessage(message);
  }
}

void mitk::EnableBackends(LogBackendBase::OutputType type)
{
  disabledBackendTypes.erase(type);
}

void mitk::DisableBackends(LogBackendBase::OutputType type)
{
  disabledBackendTypes.insert(type);
}

bool mitk::IsBackendEnabled(LogBackendBase::OutputType type)
{
  return disabledBackendTypes.find(type) == disabledBackendTypes.end();
}
