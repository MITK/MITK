/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleEvent.h>

#include <usModule.h>

namespace us {

class ModuleEventData : public SharedData
{
public:

  ModuleEventData(ModuleEvent::Type type, Module* module)
    : type(type), module(module)
  {

  }

  ModuleEventData(const ModuleEventData& other)
    : SharedData(other), type(other.type), module(other.module)
  {

  }

  const ModuleEvent::Type type;
  Module* const module;

private:

  // purposely not implemented
  ModuleEventData& operator=(const ModuleEventData&);
};

ModuleEvent::ModuleEvent()
  : d(nullptr)
{

}

ModuleEvent::~ModuleEvent()
{

}

bool ModuleEvent::IsNull() const
{
  return !d;
}

ModuleEvent::ModuleEvent(Type type, Module* module)
  : d(new ModuleEventData(type, module))
{

}

ModuleEvent::ModuleEvent(const ModuleEvent& other)
  : d(other.d)
{

}

ModuleEvent& ModuleEvent::operator=(const ModuleEvent& other)
{
  d = other.d;
  return *this;
}

Module* ModuleEvent::GetModule() const
{
  return d->module;
}

ModuleEvent::Type ModuleEvent::GetType() const
{
  return d->type;
}

std::ostream& operator<<(std::ostream& os, ModuleEvent::Type eventType)
{
  switch (eventType)
  {
  case ModuleEvent::LOADED:    return os << "LOADED";
  case ModuleEvent::UNLOADED:  return os << "UNLOADED";
  case ModuleEvent::LOADING:   return os << "LOADING";
  case ModuleEvent::UNLOADING: return os << "UNLOADING";

  default: return os << "Unknown module event type (" << static_cast<int>(eventType) << ")";
  }
}

std::ostream& operator<<(std::ostream& os, const ModuleEvent& event)
{
  if (event.IsNull()) return os << "NONE";

  Module* m = event.GetModule();
  os << event.GetType() << " #" << m->GetModuleId() << " (" << m->GetLocation() << ")";
  return os;
}

}
