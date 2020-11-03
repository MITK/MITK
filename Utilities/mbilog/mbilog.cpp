/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <list>
#include <set>

#include "mbilog.h"

static std::list<mbilog::BackendBase *> backends;
static std::set<mbilog::OutputType> disabledBackendTypes;

namespace mbilog
{
  static const std::string NA_STRING = "n/a";
}

void mbilog::RegisterBackend(mbilog::BackendBase *backend)
{
  backends.push_back(backend);
}

void mbilog::UnregisterBackend(mbilog::BackendBase *backend)
{
  backends.remove(backend);
}

void mbilog::DistributeToBackends(mbilog::LogMessage &l)
{
  // Crop Message
  {
    std::string::size_type i = l.message.find_last_not_of(" \t\f\v\n\r");
    l.message = (i != std::string::npos) ? l.message.substr(0, i + 1) : "";
  }

  // create dummy backend if there is no backend registered (so we have an output anyway)
  static mbilog::BackendCout *dummyBackend = nullptr;

  if (backends.empty() && (dummyBackend == nullptr))
  {
    dummyBackend = new mbilog::BackendCout();
    dummyBackend->SetFull(false);
    RegisterBackend(dummyBackend);
  }
  else if ((backends.size() > 1) && (dummyBackend != nullptr))
  {
    // if there was added another backend remove the dummy backend and delete it
    UnregisterBackend(dummyBackend);
    delete dummyBackend;
    dummyBackend = nullptr;
  }

  // iterate through all registered images and call the ProcessMessage() methods of the backends
  std::list<mbilog::BackendBase *>::iterator i;
  for (i = backends.begin(); i != backends.end(); i++)
  {
    if (IsBackendEnabled((*i)->GetOutputType()))
      (*i)->ProcessMessage(l);
  }
}

void mbilog::EnableBackends(OutputType type)
{
  disabledBackendTypes.erase(type);
}

void mbilog::DisableBackends(OutputType type)
{
  disabledBackendTypes.insert(type);
}

bool mbilog::IsBackendEnabled(OutputType type)
{
  return disabledBackendTypes.find(type) == disabledBackendTypes.end();
}
