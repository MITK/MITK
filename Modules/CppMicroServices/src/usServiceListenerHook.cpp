/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usServiceListenerHook.h>
#include "usServiceListenerHook_p.h"

namespace us {

ServiceListenerHook::~ServiceListenerHook()
{
}

ServiceListenerHook::ListenerInfoData::ListenerInfoData(
    ModuleContext* mc, const ServiceListenerEntry::ServiceListener& l,
    void* data, const std::string& filter)
  : mc(mc)
  , listener(l)
  , data(data)
  , filter(filter)
  , bRemoved(false)
{
}

ServiceListenerHook::ListenerInfoData::~ListenerInfoData()
{
}

ServiceListenerHook::ListenerInfo::ListenerInfo(ListenerInfoData* data)
  : d(data)
{
}

ServiceListenerHook::ListenerInfo::ListenerInfo()
  : d(nullptr)
{
}

ServiceListenerHook::ListenerInfo::ListenerInfo(const ListenerInfo& other)
  : d(other.d)
{
}

ServiceListenerHook::ListenerInfo::~ListenerInfo()
{
}

ServiceListenerHook::ListenerInfo& ServiceListenerHook::ListenerInfo::operator=(const ListenerInfo& other)
{
  d = other.d;
  return *this;
}

bool ServiceListenerHook::ListenerInfo::IsNull() const
{
  return !d;
}

ModuleContext* ServiceListenerHook::ListenerInfo::GetModuleContext() const
{
  return d->mc;
}

std::string ServiceListenerHook::ListenerInfo::GetFilter() const
{
  return d->filter;
}

bool ServiceListenerHook::ListenerInfo::IsRemoved() const
{
  return d->bRemoved;
}

bool ServiceListenerHook::ListenerInfo::operator==(const ListenerInfo& other) const
{
  return d == other.d;
}

}
