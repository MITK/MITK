/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USLISTENERFUNCTORS_P_H
#define USLISTENERFUNCTORS_P_H

#include <usServiceEvent.h>
#include <usModuleEvent.h>

#include <algorithm>
#include <cstring>
#include <functional>

#define US_MODULE_LISTENER_FUNCTOR std::function<void(const us::ModuleEvent&)>
#define US_SERVICE_LISTENER_FUNCTOR std::function<void(const us::ServiceEvent&)>

namespace us {
  template<class X>
  US_MODULE_LISTENER_FUNCTOR ModuleListenerMemberFunctor(X* x, void (X::*memFn)(const us::ModuleEvent))
  { return std::bind(std::mem_fn(memFn), x, std::placeholders::_1); }

  struct ModuleListenerCompare
  {
    bool operator()(const std::pair<US_MODULE_LISTENER_FUNCTOR, void*>& p1,
                    const std::pair<US_MODULE_LISTENER_FUNCTOR, void*>& p2) const
    {
      return p1.second == p2.second &&
             p1.first.target<void(const us::ModuleEvent&)>() == p2.first.target<void(const us::ModuleEvent&)>();
    }
  };

  template<class X>
  US_SERVICE_LISTENER_FUNCTOR ServiceListenerMemberFunctor(X* x, void (X::*memFn)(const us::ServiceEvent))
  { return std::bind(std::mem_fn(memFn), x, std::placeholders::_1); }

  struct ServiceListenerCompare
  {
    bool operator()(const US_SERVICE_LISTENER_FUNCTOR& f1,
                    const US_SERVICE_LISTENER_FUNCTOR& f2) const
    {
      return f1.target<void(const us::ServiceEvent&)>() == f2.target<void(const us::ServiceEvent&)>();
    }
  };
}

namespace std {
template<> struct hash<US_SERVICE_LISTENER_FUNCTOR> { std::size_t operator()(const US_SERVICE_LISTENER_FUNCTOR& arg) const {
  void(*targetFunc)(const us::ServiceEvent&) = arg.target<void(const us::ServiceEvent&)>();
  void* targetPtr = nullptr;
  std::memcpy(&targetPtr, &targetFunc, sizeof(void*));
  return std::hash<void*>()(targetPtr);
} };
}

#endif // USLISTENERFUNCTORS_P_H
