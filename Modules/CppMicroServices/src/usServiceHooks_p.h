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

class ServiceHooks : private MultiThreaded<>, private ServiceTrackerCustomizer<ServiceListenerHook>
{

private:

  CoreModuleContext* coreCtx;
  ServiceTracker<ServiceListenerHook>* listenerHookTracker;

  bool bOpen;

  TrackedType AddingService(const ServiceReferenceType& reference) override;
  void ModifiedService(const ServiceReferenceType& reference, TrackedType service) override;
  void RemovedService(const ServiceReferenceType& reference, TrackedType service) override;

public:

  ServiceHooks(CoreModuleContext* coreCtx);
  ~ServiceHooks() override;

  void Open();

  void Close();

  bool IsOpen() const;

  void FilterServiceReferences(ModuleContext* mc, const std::string& service,
                               const std::string& filter, std::vector<ServiceReferenceBase>& refs);

  void FilterServiceEventReceivers(const ServiceEvent& evt,
                                   ServiceListeners::ServiceListenerEntries& receivers);

  void HandleServiceListenerReg(const ServiceListenerEntry& sle);

  void HandleServiceListenerUnreg(const ServiceListenerEntry& sle);

  void HandleServiceListenerUnreg(const std::vector<ServiceListenerEntry>& set);

};

}

#endif // USSERVICEHOOKS_P_H
