/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USSERVICEHOOKS_P_H
#define USSERVICEHOOKS_P_H

#include <usServiceTracker.h>
#include "usServiceListeners_p.h"

namespace us {

struct ServiceListenerHook;

/**
 * \brief Internal helper that invokes registered service hook services.
 *
 * This class is not part of the public API. It tracks ServiceListenerHook
 * services and uses FindHook / EventListenerHook services to filter service
 * references and event receivers.
 *
 * \sa CoreModuleContext ModuleHooks ServiceListenerHook
 */
class ServiceHooks : private MultiThreaded<>, private ServiceTrackerCustomizer<ServiceListenerHook>
{

private:

  CoreModuleContext* coreCtx;                                ///< \brief Owning core context.
  ServiceTracker<ServiceListenerHook>* listenerHookTracker;  ///< \brief Tracker for listener hook services.

  bool bOpen; ///< \brief Whether the hooks subsystem is open.

  /** \brief Called when a ServiceListenerHook is registered. */
  TrackedType AddingService(const ServiceReferenceType& reference) override;
  /** \brief Called when a tracked ServiceListenerHook is modified. */
  void ModifiedService(const ServiceReferenceType& reference, TrackedType service) override;
  /** \brief Called when a tracked ServiceListenerHook is unregistered. */
  void RemovedService(const ServiceReferenceType& reference, TrackedType service) override;

public:

  /**
   * \brief Construct ServiceHooks bound to the given core context.
   *
   * \param[in] coreCtx The core module context that owns the hooks.
   */
  ServiceHooks(CoreModuleContext* coreCtx);

  /** \brief Destructor. */
  ~ServiceHooks() override;

  /** \brief Open the hooks subsystem and start tracking listener hooks. */
  void Open();

  /** \brief Close the hooks subsystem and stop tracking listener hooks. */
  void Close();

  /**
   * \brief Check whether the hooks subsystem is open.
   *
   * \return \c true if Open() has been called and Close() has not.
   */
  bool IsOpen() const;

  /**
   * \brief Apply FindHook services to filter service references.
   *
   * \param[in] mc The calling module context.
   * \param[in] service The service interface name.
   * \param[in] filter The LDAP filter string.
   * \param[in,out] refs The service reference vector; entries may be removed by hooks.
   */
  void FilterServiceReferences(ModuleContext* mc, const std::string& service,
                               const std::string& filter, std::vector<ServiceReferenceBase>& refs);

  /**
   * \brief Apply EventListenerHook services to filter service event receivers.
   *
   * \param[in] evt The service event being dispatched.
   * \param[in,out] receivers The listener entries; entries may be removed by hooks.
   */
  void FilterServiceEventReceivers(const ServiceEvent& evt,
                                   ServiceListeners::ServiceListenerEntries& receivers);

  /**
   * \brief Notify listener hooks that a service listener was registered.
   *
   * \param[in] sle The newly registered service listener entry.
   */
  void HandleServiceListenerReg(const ServiceListenerEntry& sle);

  /**
   * \brief Notify listener hooks that a service listener was unregistered.
   *
   * \param[in] sle The unregistered service listener entry.
   */
  void HandleServiceListenerUnreg(const ServiceListenerEntry& sle);

  /**
   * \brief Notify listener hooks that multiple service listeners were unregistered.
   *
   * \param[in] set The unregistered service listener entries.
   */
  void HandleServiceListenerUnreg(const std::vector<ServiceListenerEntry>& set);

};

}

#endif // USSERVICEHOOKS_P_H
