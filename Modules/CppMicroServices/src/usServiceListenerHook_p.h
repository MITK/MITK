/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USSERVICELISTENERHOOK_P_H
#define USSERVICELISTENERHOOK_P_H

#include <usServiceListenerHook.h>
#include "usServiceListenerEntry_p.h"
#include <usSharedData.h>

namespace us {

class ServiceListenerHook::ListenerInfoData : public SharedData
{
public:
  ListenerInfoData(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& l,
                   void* data, const std::string& filter);

  virtual ~ListenerInfoData();

  ModuleContext* const mc;
  ServiceListenerEntry::ServiceListener listener;
  void* data;
  std::string filter;
  bool bRemoved;
};

}

#endif // USSERVICELISTENERHOOK_P_H
