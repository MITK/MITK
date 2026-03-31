/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USSERVICELISTENERS_H
#define USSERVICELISTENERS_H

#include <list>
#include <string>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <usGlobalConfig.h>

#include "usServiceListenerEntry_p.h"

namespace us {

class CoreModuleContext;
class ModuleContext;

/**
 * \brief Manages all service and module listeners registered by modules.
 *
 * \sa ServiceListenerEntry CoreModuleContext
 */
class ServiceListeners : private MultiThreaded<>
{

public:

  typedef US_MODULE_LISTENER_FUNCTOR ModuleListener;
  typedef std::unordered_map<ModuleContext*, std::list<std::pair<ModuleListener,void*> > > ModuleListenerMap;

  /** \brief Map from module context to its registered module listeners. */
  ModuleListenerMap moduleListenerMap;

  /** \brief Mutex protecting moduleListenerMap. */
  Mutex moduleListenerMapMutex;

  typedef std::unordered_map<std::string, std::list<ServiceListenerEntry> > CacheType;
  typedef std::unordered_set<ServiceListenerEntry> ServiceListenerEntries;

private:

  std::vector<std::string> hashedServiceKeys;
  static const int OBJECTCLASS_IX; // = 0;
  static const int SERVICE_ID_IX; // = 1;

  /* Service listeners with complicated or empty filters */
  std::list<ServiceListenerEntry> complicatedListeners;

  /* Service listeners with "simple" filters are cached. */
  CacheType cache[2];

  ServiceListenerEntries serviceSet;

  CoreModuleContext* coreCtx;

public:

  /** \brief Construct a ServiceListeners instance.
   *  \param[in] coreCtx The core module context owning this instance.
   */
  ServiceListeners(CoreModuleContext* coreCtx);

  /**
   * \brief Add a new service listener.
   *
   * If an old one exists, and it has the same owning module, the old
   * listener is removed first.
   *
   * \param[in] mc The module context adding this listener.
   * \param[in] listener The service listener to add.
   * \param[in] data Additional data to distinguish ServiceListener objects.
   * \param[in] filter An LDAP filter string to check when a service is modified.
   * \throws std::invalid_argument If the filter is not a correct LDAP expression.
   */
  void AddServiceListener(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& listener,
                          void* data, const std::string& filter);

  /**
   * \brief Remove a service listener from the current framework.
   *
   * Silently ignores if listener doesn't exist. If listener is registered
   * more than once, removes all instances.
   *
   * \param[in] mc The module context who wants to remove the listener.
   * \param[in] listener The listener object to remove.
   * \param[in] data Additional data to distinguish ServiceListener objects.
   */
  void RemoveServiceListener(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& listener,
                             void* data);

  /**
   * \brief Add a new module listener.
   *
   * \param[in] mc The module context adding this listener.
   * \param[in] listener The module listener to add.
   * \param[in] data Additional data to distinguish ModuleListener objects.
   */
  void AddModuleListener(ModuleContext* mc, const ModuleListener& listener, void* data);

  /**
   * \brief Remove a module listener from the current framework.
   *
   * Silently ignores if listener doesn't exist.
   *
   * \param[in] mc The module context who wants to remove the listener.
   * \param[in] listener The listener object to remove.
   * \param[in] data Additional data to distinguish ModuleListener objects.
   */
  void RemoveModuleListener(ModuleContext* mc, const ModuleListener& listener, void* data);

  /** \brief Notify all registered module listeners about a module event.
   *  \param[in] evt The module event to deliver.
   */
  void ModuleChanged(const ModuleEvent& evt);

  /**
   * \brief Remove all listeners registered by a module in the current framework.
   *
   * \param[in] mc Module context whose listeners should be removed.
   */
  void RemoveAllListeners(ModuleContext* mc);

  /**
   * \brief Notify hooks that a module is about to be stopped.
   *
   * \param[in] mc Module context whose listeners are about to be removed.
   */
  void HooksModuleStopped(ModuleContext* mc);

  /**
   * \brief Receive notification that a service has had a lifecycle change.
   *
   * \param[in] receivers The set of listener entries that should receive the event.
   * \param[in] evt The service event describing the change.
   * \param[in] matchBefore The set of listeners that matched before the change.
   *
   * \sa ServiceEvent
   */
  void ServiceChanged(ServiceListenerEntries& receivers,
                      const ServiceEvent& evt,
                      ServiceListenerEntries& matchBefore);

  /** \brief Overload of ServiceChanged without matchBefore tracking.
   *  \param[in] receivers The set of listener entries that should receive the event.
   *  \param[in] evt The service event describing the change.
   */
  void ServiceChanged(ServiceListenerEntries& receivers,
                      const ServiceEvent& evt);

  /**
   * \brief Get all service listeners matching a given service event.
   *
   * \param[in] evt The service event to match against.
   * \param[out] listeners The set to populate with matching listener entries.
   * \param[in] lockProps Whether to lock service properties during matching.
   */
  void GetMatchingServiceListeners(const ServiceEvent& evt, ServiceListenerEntries& listeners,
                                   bool lockProps = true);

  /** \brief Get a snapshot of all currently registered listener info objects.
   *  \return A vector of ListenerInfo objects.
   */
  std::vector<ServiceListenerHook::ListenerInfo> GetListenerInfoCollection() const;

private:

  /** \brief Remove a service listener without acquiring the lock.
   *  \param[in] entryToRemove The listener entry to remove.
   */
  void RemoveServiceListener_unlocked(const ServiceListenerEntry& entryToRemove);

  /**
   * \brief Remove all references to a service listener from the cache.
   *
   * \param[in] sle The service listener entry to remove from cache.
   */
  void RemoveFromCache(const ServiceListenerEntry& sle);

  /**
   * \brief Check if a service listener's filter is simple enough to cache.
   *
   * \param[in] sle The service listener entry to check.
   */
  void CheckSimple(const ServiceListenerEntry& sle);

  /** \brief Add matching listeners from a cache entry to the result set.
   *  \param[in,out] set The set to add matching listeners to.
   *  \param[in] receivers The candidate listener entries.
   *  \param[in] cache_ix The cache index to use (OBJECTCLASS_IX or SERVICE_ID_IX).
   *  \param[in] val The cache key value to look up.
   */
  void AddToSet(ServiceListenerEntries& set, const ServiceListenerEntries& receivers, int cache_ix, const std::string& val);

};

}

#endif // USSERVICELISTENERS_H
