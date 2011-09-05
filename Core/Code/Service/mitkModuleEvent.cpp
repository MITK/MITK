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

#include "mitkModuleEvent.h"

#include "mitkModule.h"

namespace mitk {

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
};

ModuleEvent::ModuleEvent()
  : d(0)
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

}

std::ostream& operator<<(std::ostream& os, mitk::ModuleEvent::Type eventType)
{
  switch (eventType)
  {
  case mitk::ModuleEvent::LOADED:    return os << "LOADED";
  case mitk::ModuleEvent::UNLOADED:  return os << "UNLOADED";
  case mitk::ModuleEvent::LOADING:   return os << "LOADING";
  case mitk::ModuleEvent::UNLOADING: return os << "UNLOADING";

  default: return os << "Unknown module event type (" << static_cast<int>(eventType) << ")";
  }
}

std::ostream& operator<<(std::ostream& os, const mitk::ModuleEvent& event)
{
  if (event.IsNull()) return os << "NONE";

  mitk::Module* m = event.GetModule();
  os << event.GetType() << " #" << m->GetModuleId() << " (" << m->GetLocation() << ")";
  return os;
}
