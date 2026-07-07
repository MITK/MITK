/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USTESTUTILMODULELISTENER_H
#define USTESTUTILMODULELISTENER_H

#include "usModuleEvent.h"
#include "usServiceEvent.h"
#include "usModuleContext.h"

#include "usTestingMacros.h"

namespace us {

class ModuleContext;

class TestModuleListener {

public:

  TestModuleListener();

  void ModuleChanged(const ModuleEvent event);

  void ServiceChanged(const ServiceEvent event);

  ModuleEvent GetModuleEvent() const;

  ServiceEvent GetServiceEvent() const;

  bool CheckListenerEvents(
      bool pexp, ModuleEvent::Type ptype,
      bool sexp, ServiceEvent::Type stype,
      Module* moduleX, ServiceReferenceU* servX);

  bool CheckListenerEvents(const std::vector<ModuleEvent>& pEvts);

  bool CheckListenerEvents(const std::vector<ServiceEvent>& seEvts);

  bool CheckListenerEvents(const std::vector<ModuleEvent>& pEvts,
                           const std::vector<ServiceEvent>& seEvts);

private:

  std::vector<ServiceEvent> serviceEvents;
  std::vector<ModuleEvent> moduleEvents;
};

template<class Receiver>
class ModuleListenerRegistrationHelper
{

public:

  typedef void(Receiver::*CallbackType)(const ModuleEvent);

  ModuleListenerRegistrationHelper(ModuleContext* context, Receiver* receiver, CallbackType callback)
    : context(context)
    , receiver(receiver)
    , callback(callback)
  {
    try
    {
      context->AddModuleListener(receiver, callback);
    }
    catch (const std::logic_error& ise)
    {
      US_TEST_OUTPUT( << "module listener registration failed " << ise.what() );
      throw;
    }
  }

  ~ModuleListenerRegistrationHelper()
  {
    context->RemoveModuleListener(receiver, callback);
  }

private:

  ModuleContext* context;
  Receiver* receiver;
  CallbackType callback;
};

template<class Receiver>
class ServiceListenerRegistrationHelper
{

public:

  typedef void(Receiver::*CallbackType)(const ServiceEvent);

  ServiceListenerRegistrationHelper(ModuleContext* context, Receiver* receiver, CallbackType callback)
    : context(context)
    , receiver(receiver)
    , callback(callback)
  {
    try
    {
      context->AddServiceListener(receiver, callback);
    }
    catch (const std::logic_error& ise)
    {
      US_TEST_OUTPUT( << "service listener registration failed " << ise.what() );
      throw;
    }
  }

  ~ServiceListenerRegistrationHelper()
  {
    context->RemoveServiceListener(receiver, callback);
  }

private:

  ModuleContext* context;
  Receiver* receiver;
  CallbackType callback;
};

}

#endif // USTESTUTILMODULELISTENER_H
