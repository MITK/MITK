/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usServiceEventListenerHook.h>
#include <usServiceListenerHook.h>
#include <usServiceRegistration.h>

#include "usTestingMacros.h"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <thread>

using namespace us;

namespace {

// Run `fn` on a worker thread and wait up to `timeoutSeconds` for it to
// finish. If it does not finish, treat that as a deadlock: we cannot safely
// join a thread stuck on a mutex, and we cannot rely on graceful shutdown
// either (static destructors in CppMicroServices would themselves block on
// the held lock). Print a failure message and hard-exit the process so the
// test reports failure quickly instead of waiting out the CTest timeout.
void RunWithDeadlockGuard(const char* description,
                          std::function<void()> fn,
                          int timeoutSeconds = 10)
{
  std::atomic<bool> completed{false};
  std::thread worker([&]
  {
    fn();
    completed.store(true);
  });

  const auto deadline = std::chrono::steady_clock::now()
                        + std::chrono::seconds(timeoutSeconds);
  while (std::chrono::steady_clock::now() < deadline)
  {
    if (completed.load())
    {
      worker.join();
      return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  worker.detach();
  std::cerr << "[FAILED] Timed out waiting for: " << description
            << " (probable deadlock in listener/hook re-entrance path)"
            << std::endl;
  std::_Exit(EXIT_FAILURE);
}

struct NoopListener
{
  void ServiceChanged(const ServiceEvent) {}
};

struct DummyInterface {};
struct InnerInterface {};

struct DummyService : public DummyInterface {};
struct InnerService : public InnerInterface {};

// ServiceListenerHook whose Added()/Removed() callbacks re-enter the
// listener API. Before the lock fix, ServiceListeners held its
// non-recursive mutex across the hook dispatch, so these calls
// self-deadlocked on the same thread.
//
// `armed` is flipped on by the test only after the initial hook-
// registration dispatch (which runs unlocked via the tracker's
// AddingService customizer) has settled. That ensures the re-entry
// happens during a subsequent AddServiceListener call, which *does*
// hold the lock in the broken state.
class ReentrantListenerHook : public ServiceListenerHook
{
public:

  void Added(const std::vector<ListenerInfo>& /*listeners*/) override
  {
    ++addedCount;
    if (!armed || reentered) return;
    reentered = true;

    ModuleContext* ctx = GetModuleContext();
    NoopListener dummy;
    ctx->AddServiceListener(&dummy, &NoopListener::ServiceChanged);
    ctx->RemoveServiceListener(&dummy, &NoopListener::ServiceChanged);
  }

  void Removed(const std::vector<ListenerInfo>& /*listeners*/) override
  {
    ++removedCount;
  }

  std::atomic<bool> armed{false};
  std::atomic<int> addedCount{0};
  std::atomic<int> removedCount{0};
  std::atomic<bool> reentered{false};
};

// ServiceEventListenerHook whose Event() callback registers another service.
// Service registration internally calls GetMatchingServiceListeners, which
// previously held the same mutex we are already inside of.
class ReentrantEventHook : public ServiceEventListenerHook
{
public:

  void Event(const ServiceEvent& /*event*/, ShrinkableMapType& /*listeners*/) override
  {
    ++eventCount;
    if (!armed || reentered) return;
    reentered = true;

    innerReg = GetModuleContext()->RegisterService<InnerInterface>(&innerService);
  }

  std::atomic<bool> armed{false};
  std::atomic<int> eventCount{0};
  std::atomic<bool> reentered{false};
  InnerService innerService;
  ServiceRegistrationU innerReg;
};

} // namespace

int usHookReentranceTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("HookReentranceTest")

  // Regression: ServiceListeners held its lock while dispatching
  // ServiceListenerHook::Added/Removed. A hook that re-enters
  // AddServiceListener or RemoveServiceListener from its callback would
  // deadlock on the non-recursive mutex.
  RunWithDeadlockGuard("ServiceListenerHook re-entry", []
  {
    ModuleContext* ctx = GetModuleContext();

    ReentrantListenerHook hook;
    ServiceRegistration<ServiceListenerHook> hookReg =
        ctx->RegisterService<ServiceListenerHook>(&hook);

    // The initial Added() dispatch runs through the tracker's
    // AddingService customizer with no ServiceListeners lock held, so the
    // re-entry path would not exercise the deadlock. Arm the hook only
    // afterwards, so the next AddServiceListener triggers Added() through
    // the lock-holding HandleServiceListenerReg path.
    hook.armed = true;

    NoopListener listener;
    ctx->AddServiceListener(&listener, &NoopListener::ServiceChanged);
    ctx->RemoveServiceListener(&listener, &NoopListener::ServiceChanged);

    hookReg.Unregister();

    US_TEST_CONDITION(hook.reentered, "ServiceListenerHook re-entered the API")
    US_TEST_CONDITION(hook.addedCount >= 2, "Added() was called for initial + armed registration")
  });

  // Regression: GetMatchingServiceListeners held its lock while dispatching
  // ServiceEventListenerHook::Event. A hook whose Event() registers a
  // service triggers an inner GetMatchingServiceListeners on the same
  // thread, which would deadlock on the non-recursive mutex.
  RunWithDeadlockGuard("ServiceEventListenerHook re-entry", []
  {
    ModuleContext* ctx = GetModuleContext();

    ReentrantEventHook hook;
    ServiceRegistration<ServiceEventListenerHook> hookReg =
        ctx->RegisterService<ServiceEventListenerHook>(&hook);

    hook.armed = true;

    DummyService outerService;
    ServiceRegistration<DummyInterface> outerReg =
        ctx->RegisterService<DummyInterface>(&outerService);

    outerReg.Unregister();
    if (hook.innerReg)
    {
      hook.innerReg.Unregister();
    }
    hookReg.Unregister();

    US_TEST_CONDITION(hook.reentered, "ServiceEventListenerHook re-entered the API")
    US_TEST_CONDITION(hook.eventCount > 0, "Event() was called at least once")
  });

  US_TEST_END()
}
