/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usUtils_p.h>

#include "usServiceListeners_p.h"
#include "usServiceReferenceBasePrivate.h"

#include "usCoreModuleContext_p.h"
#include <usModule.h>
#include <usModuleContext.h>


namespace us {

const int ServiceListeners::OBJECTCLASS_IX = 0;
const int ServiceListeners::SERVICE_ID_IX = 1;

ServiceListeners::ServiceListeners(CoreModuleContext* coreCtx)
  : coreCtx(coreCtx)
{
  hashedServiceKeys.push_back(ServiceConstants::OBJECTCLASS());
  hashedServiceKeys.push_back(ServiceConstants::SERVICE_ID());
}

void ServiceListeners::AddServiceListener(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& listener,
                                          void* data, const std::string& filter)
{
  // Hook callbacks run user code that can re-enter the listener API, which
  // would deadlock on our non-recursive mutex. Scope the lock to the data
  // mutations and dispatch the hooks after releasing it.
  ServiceListenerEntry sle(mc, listener, data, filter);
  std::optional<ServiceListenerEntry> replaced;
  {
    Lock lock(this);
    replaced = RemoveServiceListener_unlocked(sle);
    serviceSet.insert(sle);
    CheckSimple(sle);
  }
  if (replaced)
  {
    coreCtx->serviceHooks.HandleServiceListenerUnreg(*replaced);
  }
  coreCtx->serviceHooks.HandleServiceListenerReg(sle);
}

void ServiceListeners::RemoveServiceListener(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& listener,
                                             void* data)
{
  ServiceListenerEntry entryToRemove(mc, listener, data);
  std::optional<ServiceListenerEntry> removed;
  {
    Lock lock(this);
    removed = RemoveServiceListener_unlocked(entryToRemove);
  }
  if (removed)
  {
    coreCtx->serviceHooks.HandleServiceListenerUnreg(*removed);
  }
}

std::optional<ServiceListenerEntry>
ServiceListeners::RemoveServiceListener_unlocked(const ServiceListenerEntry& entryToRemove)
{
  ServiceListenerEntries::const_iterator it = serviceSet.find(entryToRemove);
  if (it == serviceSet.end())
  {
    return std::nullopt;
  }
  it->SetRemoved(true);
  ServiceListenerEntry removed = *it;
  RemoveFromCache(*it);
  serviceSet.erase(it);
  return removed;
}

void ServiceListeners::AddModuleListener(ModuleContext* mc, const ModuleListener& listener, void* data)
{
  MutexLock lock(moduleListenerMapMutex);
  ModuleListenerMap::value_type::second_type& listeners = moduleListenerMap[mc];
  if (std::find_if(listeners.begin(), listeners.end(), std::bind(ModuleListenerCompare(), std::make_pair(listener, data), std::placeholders::_1)) == listeners.end())
  {
    listeners.push_back(std::make_pair(listener, data));
  }
}

void ServiceListeners::RemoveModuleListener(ModuleContext* mc, const ModuleListener& listener, void* data)
{
  MutexLock lock(moduleListenerMapMutex);
  moduleListenerMap[mc].remove_if(std::bind(ModuleListenerCompare(), std::make_pair(listener, data), std::placeholders::_1));
}

void ServiceListeners::ModuleChanged(const ModuleEvent& evt)
{
  ModuleListenerMap filteredModuleListeners;
  coreCtx->moduleHooks.FilterModuleEventReceivers(evt, filteredModuleListeners);

  for(ModuleListenerMap::iterator iter = filteredModuleListeners.begin(), end = filteredModuleListeners.end();
      iter != end; ++iter)
  {
    for (ModuleListenerMap::mapped_type::iterator iter2 = iter->second.begin(), end2 = iter->second.end();
         iter2 != end2; ++iter2)
    {
      try
      {
        (iter2->first)(evt);
      }
      catch (const std::exception& e)
      {
        MITK_WARN << "Module listener threw an exception: " << e.what();
      }
    }
  }
}

void ServiceListeners::RemoveAllListeners(ModuleContext* mc)
{
  {
    Lock lock(this);
    for (ServiceListenerEntries::iterator it = serviceSet.begin();
         it != serviceSet.end(); )
    {

      if (it->GetModuleContext() == mc)
      {
        RemoveFromCache(*it);
        serviceSet.erase(it++);
      }
      else
      {
        ++it;
      }
    }
  }

  {
    MutexLock lock(moduleListenerMapMutex);
    moduleListenerMap.erase(mc);
  }
}

void ServiceListeners::HooksModuleStopped(ModuleContext* mc)
{
  std::vector<ServiceListenerEntry> entries;
  {
    Lock lock(this);
    for (ServiceListenerEntries::iterator it = serviceSet.begin();
         it != serviceSet.end(); ++it)
    {
      if (it->GetModuleContext() == mc)
      {
        entries.push_back(*it);
      }
    }
  }
  // Dispatch the hook callback outside the lock; it runs user code that
  // may re-enter the listener API.
  coreCtx->serviceHooks.HandleServiceListenerUnreg(entries);
}

void ServiceListeners::ServiceChanged(ServiceListenerEntries& receivers,
                                      const ServiceEvent& evt)
{
  ServiceListenerEntries matchBefore;
  ServiceChanged(receivers, evt, matchBefore);
}

void ServiceListeners::ServiceChanged(ServiceListenerEntries& receivers,
                                      const ServiceEvent& evt,
                                      ServiceListenerEntries& matchBefore)
{
  if (!matchBefore.empty())
  {
    for (ServiceListenerEntries::const_iterator l = receivers.begin();
         l != receivers.end(); ++l)
    {
      matchBefore.erase(*l);
    }
  }

  for (ServiceListenerEntries::const_iterator l = receivers.begin();
       l != receivers.end(); ++l)
  {
    if (!l->IsRemoved())
    {
      try
      {
        l->CallDelegate(evt);
      }
      catch (...)
      {
        MITK_WARN << "Service listener"
                << " in " << l->GetModuleContext()->GetModule()->GetName()
                << " threw an exception!";
      }
    }
  }
}

void ServiceListeners::GetMatchingServiceListeners(const ServiceEvent& evt, ServiceListenerEntries& set,
                                                   bool lockProps)
{
  // Snapshot the current listener set under the lock so we can invoke the
  // event-hook callback without holding it: user hooks may re-enter the
  // listener API and would deadlock on our non-recursive mutex.
  ServiceListenerEntries receivers;
  {
    Lock lock(this);
    receivers = serviceSet;
  }
  coreCtx->serviceHooks.FilterServiceEventReceivers(evt, receivers);

  // Re-acquire the lock for reading complicatedListeners and the simple-filter
  // cache. Concurrent modifications between the two critical sections are
  // tolerated: receivers holds a stable local copy, and any listener removed
  // concurrently is already filtered out by the IsRemoved() check at dispatch.
  Lock lock(this);

  // Check complicated or empty listener filters
  for (std::list<ServiceListenerEntry>::const_iterator sse = complicatedListeners.begin();
       sse != complicatedListeners.end(); ++sse)
  {
    if (receivers.count(*sse) == 0) continue;
    const LDAPExpr& ldapExpr = sse->GetLDAPExpr();
    if (ldapExpr.IsNull() ||
        ldapExpr.Evaluate(evt.GetServiceReference().d->GetProperties(), false))
    {
      set.insert(*sse);
    }
  }

  // Check the cache
  const std::vector<std::string> c(any_cast<std::vector<std::string> >
                                 (evt.GetServiceReference().d->GetProperty(ServiceConstants::OBJECTCLASS(), lockProps)));
  for (std::vector<std::string>::const_iterator objClass = c.begin();
       objClass != c.end(); ++objClass)
  {
    AddToSet(set, receivers, OBJECTCLASS_IX, *objClass);
  }

  long service_id = any_cast<long>(evt.GetServiceReference().d->GetProperty(ServiceConstants::SERVICE_ID(), lockProps));
  std::stringstream ss;
  ss << service_id;
  AddToSet(set, receivers, SERVICE_ID_IX, ss.str());
}

std::vector<ServiceListenerHook::ListenerInfo> ServiceListeners::GetListenerInfoCollection() const
{
  Lock lock(this);
  std::vector<ServiceListenerHook::ListenerInfo> result;
  result.reserve(serviceSet.size());
  for (ServiceListenerEntries::const_iterator iter = serviceSet.begin(),
       iterEnd = serviceSet.end(); iter != iterEnd; ++iter)
  {
    result.push_back(*iter);
  }
  return result;
}

void ServiceListeners::RemoveFromCache(const ServiceListenerEntry& sle)
{
  if (!sle.GetLocalCache().empty())
  {
    for (std::size_t i = 0; i < hashedServiceKeys.size(); ++i)
    {
      CacheType& keymap = cache[i];
      std::vector<std::string>& l = sle.GetLocalCache()[i];
      for (std::vector<std::string>::const_iterator it = l.begin();
           it != l.end(); ++it)
      {
        std::list<ServiceListenerEntry>& sles = keymap[*it];
        sles.remove(sle);
        if (sles.empty())
        {
          keymap.erase(*it);
        }
      }
    }
  }
  else
  {
    complicatedListeners.remove(sle);
  }
}

 void ServiceListeners::CheckSimple(const ServiceListenerEntry& sle) {
   if (sle.GetLDAPExpr().IsNull())
   {
     complicatedListeners.push_back(sle);
   }
   else
   {
     LDAPExpr::LocalCache local_cache;
     if (sle.GetLDAPExpr().IsSimple(hashedServiceKeys, local_cache, false))
     {
       sle.GetLocalCache() = local_cache;
       for (std::size_t i = 0; i < hashedServiceKeys.size(); ++i)
       {
         for (std::vector<std::string>::const_iterator it = local_cache[i].begin();
              it != local_cache[i].end(); ++it)
         {
           std::list<ServiceListenerEntry>& sles = cache[i][*it];
           sles.push_back(sle);
         }
       }
     }
     else
     {
       complicatedListeners.push_back(sle);
     }
   }
 }

void ServiceListeners::AddToSet(ServiceListenerEntries& set,
                                const ServiceListenerEntries& receivers,
                                int cache_ix, const std::string& val)
{
  std::list<ServiceListenerEntry>& l = cache[cache_ix][val];
  if (!l.empty())
  {

    for (std::list<ServiceListenerEntry>::const_iterator entry = l.begin();
         entry != l.end(); ++entry)
    {
      if (receivers.count(*entry))
      {
        set.insert(*entry);
      }
    }
  }
  else
  {
  }
}

}
