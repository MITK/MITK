/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "usTestUtilModuleListener.h"

#include "usUtils_p.h"

namespace us {

TestModuleListener::TestModuleListener()
  : serviceEvents(), moduleEvents()
{}

void TestModuleListener::ModuleChanged(const ModuleEvent event)
{
  moduleEvents.push_back(event);
}

void TestModuleListener::ServiceChanged(const ServiceEvent event)
{
  serviceEvents.push_back(event);
}

ModuleEvent TestModuleListener::GetModuleEvent() const
{
  if (moduleEvents.empty())
  {
    return ModuleEvent();
  }
  return moduleEvents.back();
}

ServiceEvent TestModuleListener::GetServiceEvent() const
{
  if (serviceEvents.empty())
  {
    return ServiceEvent();
  }
  return serviceEvents.back();
}

bool TestModuleListener::CheckListenerEvents(
    bool pexp, ModuleEvent::Type ptype,
    bool sexp, ServiceEvent::Type stype,
    Module* moduleX, ServiceReferenceU* servX)
{
  std::vector<ModuleEvent> pEvts;
  std::vector<ServiceEvent> seEvts;

  if (pexp) pEvts.push_back(ModuleEvent(ptype, moduleX));
  if (sexp) seEvts.push_back(ServiceEvent(stype, *servX));

  return CheckListenerEvents(pEvts, seEvts);
}

bool TestModuleListener::CheckListenerEvents(const std::vector<ModuleEvent>& pEvts)
{
  bool listenState = true; // assume everything will work

  if (pEvts.size() != moduleEvents.size())
  {
    listenState = false;

    const std::size_t max = pEvts.size() > moduleEvents.size() ? pEvts.size() : moduleEvents.size();
    for (std::size_t i = 0; i < max; ++i)
    {
      const ModuleEvent& pE = i < pEvts.size() ? pEvts[i] : ModuleEvent();
      const ModuleEvent& pR = i < moduleEvents.size() ? moduleEvents[i] : ModuleEvent();
    }
  }
  else
  {
    for (std::size_t i = 0; i < pEvts.size(); ++i)
    {
      const ModuleEvent& pE = pEvts[i];
      const ModuleEvent& pR = moduleEvents[i];
      if (pE.GetType() != pR.GetType()
          || pE.GetModule() != pR.GetModule())
      {
        listenState = false;
      }
    }
  }

  moduleEvents.clear();
  return listenState;
}

bool TestModuleListener::CheckListenerEvents(const std::vector<ServiceEvent>& seEvts)
{
  bool listenState = true; // assume everything will work

  if (seEvts.size() != serviceEvents.size())
  {
    listenState = false;

    const std::size_t max = seEvts.size() > serviceEvents.size() ? seEvts.size() : serviceEvents.size();
    for (std::size_t i = 0; i < max; ++i)
    {
      const ServiceEvent& seE = i < seEvts.size() ? seEvts[i] : ServiceEvent();
      const ServiceEvent& seR = i < serviceEvents.size() ? serviceEvents[i] : ServiceEvent();
    }
  }
  else
  {
    for (std::size_t i = 0; i < seEvts.size(); ++i)
    {
      const ServiceEvent& seE = seEvts[i];
      const ServiceEvent& seR = serviceEvents[i];
      if (seE.GetType() != seR.GetType()
          || (!(seE.GetServiceReference() == seR.GetServiceReference())))
      {
        listenState = false;
      }
    }
  }

  serviceEvents.clear();
  return listenState;
}

bool TestModuleListener::CheckListenerEvents(
    const std::vector<ModuleEvent>& pEvts,
    const std::vector<ServiceEvent>& seEvts)
{
  if (!CheckListenerEvents(pEvts)) return false;
  return CheckListenerEvents(seEvts);
}

}
