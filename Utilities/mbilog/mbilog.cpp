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

#include <list>
#include <set>

#include "mbilog.h"

static std::list<mbilog::BackendBase*> backends;
static std::set<mbilog::OutputType> disabledBackendTypes;


namespace mbilog {
static const std::string NA_STRING = "n/a";
}

void mbilog::RegisterBackend(mbilog::BackendBase* backend)
{
  backends.push_back(backend);
}

void mbilog::UnregisterBackend(mbilog::BackendBase* backend)
{
  backends.remove(backend);
}

void mbilog::DistributeToBackends(mbilog::LogMessage &l)
{
  //Crop Message
  {
    std::string::size_type i = l.message.find_last_not_of(" \t\f\v\n\r");
    l.message = (i != std::string::npos) ? l.message.substr(0, i+1) : "";
  }

  //create dummy backend if there is no backend registered (so we have an output anyway)
  static mbilog::BackendCout* dummyBackend = NULL;

  if(backends.empty() && (dummyBackend == NULL))
  {
    dummyBackend = new mbilog::BackendCout();
    dummyBackend->SetFull(false);
    RegisterBackend(dummyBackend);
  }
  else if((backends.size()>1) && (dummyBackend != NULL))
  {
    //if there was added another backend remove the dummy backend and delete it
    UnregisterBackend(dummyBackend);
    delete dummyBackend;
    dummyBackend = NULL;
  }

  //iterate through all registered images and call the ProcessMessage() methods of the backends
  std::list<mbilog::BackendBase*>::iterator i;
  for(i = backends.begin(); i != backends.end(); i++)
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


