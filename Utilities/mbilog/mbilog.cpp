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

#include "mbilog.h"

static std::list<mbilog::BackendBase*> backends;


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
  mbilog::BackendCout* dummyBackend = NULL;
  if(backends.empty())
  {
    dummyBackend = new mbilog::BackendCout();
    dummyBackend->SetFull(false);
    RegisterBackend(dummyBackend);
  }

  //iterate through all registered images and call the ProcessMessage() methods of the backends
  std::list<mbilog::BackendBase*>::iterator i;
  for(i = backends.begin(); i != backends.end(); i++)
    (*i)->ProcessMessage(l);

  //if there was added a dummy backend remove it now
  if (dummyBackend != NULL)
    {
    UnregisterBackend(dummyBackend);
    delete dummyBackend;
    }
}


