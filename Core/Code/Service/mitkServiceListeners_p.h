/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKSERVICELISTENERS_H
#define MITKSERVICELISTENERS_H

#include <list>
#include <string>
#include <set>

#include <mitkConfig.h>

#ifdef MITK_HAS_UNORDERED_MAP_H
#include <unordered_map>
#else
#include "mitkItkHashMap.h"
#endif

#ifdef MITK_HAS_UNORDERED_SET_H
#include <unordered_set>
#else
#include "mitkItkHashSet.h"
#endif

#include <itkMutexLockHolder.h>

#include "mitkServiceUtils.h"
#include "mitkServiceListenerEntry_p.h"

namespace mitk {

class CoreModuleContext;
class ModuleContext;

/**
 * Here we handle all listeners that modules have registered.
 *
 */
class ServiceListeners {

public:

  typedef Message1<const ModuleEvent&> ModuleMessage;

#ifdef MITK_HAS_UNORDERED_MAP_H
  typedef std::unordered_map<std::string, std::list<ServiceListenerEntry> > CacheType;
  typedef std::unordered_map<ModuleContext*, ModuleMessage> ModuleListenerMap;
#else
  typedef itk::hash_map<std::string, std::list<ServiceListenerEntry> > CacheType;
  typedef itk::hash_map<ModuleContext*, ModuleMessage> ModuleListenerMap;
#endif

#ifdef MITK_HAS_UNORDERED_SET_H
  typedef std::unordered_set<ServiceListenerEntry> ServiceListenerEntries;
#else
  typedef itk::hash_set<ServiceListenerEntry> ServiceListenerEntries;
#endif

private:

  std::vector<std::string> hashedServiceKeys;
  static const int OBJECTCLASS_IX; // = 0;
  static const int SERVICE_ID_IX; // = 1;

  /* Service listeners with complicated or empty filters */
  std::list<ServiceListenerEntry> complicatedListeners;

  /* Service listeners with "simple" filters are cached. */
  CacheType cache[2];

  ServiceListenerEntries serviceSet;
  ModuleListenerMap moduleListenerMap;

  typedef itk::SimpleFastMutexLock MutexType;
  typedef itk::MutexLockHolder<MutexType> MutexLocker;

  MutexType mutex;
  MutexType moduleListenerMapMutex;

public:

  ServiceListeners();

  /**
   * Add a new service listener. If an old one exists, and it has the
   * same owning module, the old listener is removed first.
   *
   * @param mc The module context adding this listener.
   * @param listener The service listener to add.
   * @param filter An LDAP filter string to check when a service is modified.
   * @exception org.osgi.framework.InvalidSyntaxException
   * If the filter is not a correct LDAP expression.
   */
  void AddServiceListener(ModuleContext* mc, const ServiceListenerDelegate& listener,
                          const std::string& filter);

  /**
   * Remove service listener from current framework. Silently ignore
   * if listener doesn't exist. If listener is registered more than
   * once remove all instances.
   *
   * @param mc The module context who wants to remove listener.
   * @param listener Object to remove.
   */
  void RemoveServiceListener(ModuleContext* mc, const ServiceListenerDelegate& listener);

  /**
   * Add a new service listener.
   *
   * @param mc The module context adding this listener.
   * @param listener The service listener to add.
   * @param filter An LDAP filter string to check when a service is modified.
   * @exception org.osgi.framework.InvalidSyntaxException
   * If the filter is not a correct LDAP expression.
   */
  void AddModuleListener(ModuleContext* mc, const ModuleListenerDelegate& listener);

  /**
   * Remove service listener from current framework. Silently ignore
   * if listener doesn't exist.
   *
   * @param mc The module context who wants to remove listener.
   * @param listener Object to remove.
   */
  void RemoveModuleListener(ModuleContext* mc, const ModuleListenerDelegate& listener);

  /**
   * Remove all listener registered by a module in the current framework.
   *
   * @param mc Module context which listeners we want to remove.
   */
  void RemoveAllListeners(ModuleContext* mc);

  /**
   * Receive notification that a service has had a change occur in its lifecycle.
   *
   * @see org.osgi.framework.ServiceListener#serviceChanged
   */
  void ServiceChanged(const ServiceListenerEntries& receivers,
                      const ServiceEvent& evt,
                      ServiceListenerEntries& matchBefore);

  void ServiceChanged(const ServiceListenerEntries& receivers,
                      const ServiceEvent& evt);

  /**
   *
   *
   */
  void GetMatchingServiceListeners(const ServiceReference& sr, ServiceListenerEntries& listeners,
                                   bool lockProps = true);

  void ModuleChanged(const ModuleEvent& evt);

private:

  void RemoveServiceListener_unlocked(const ServiceListenerEntry& entryToRemove);

  /**
   * Remove all references to a service listener from the service listener
   * cache.
   */
  void RemoveFromCache(const ServiceListenerEntry& sle);

  /**
   * Checks if the specified service listener's filter is simple enough
   * to cache.
   */
  void CheckSimple(const ServiceListenerEntry& sle);

  void AddToSet(ServiceListenerEntries& set, int cache_ix, const std::string& val);

};

}

#endif // MITKSERVICELISTENERS_H
