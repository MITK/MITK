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

#include "mitkServiceEvent.h"

#include "mitkServiceProperties.h"

namespace mitk {

class ServiceEventData : public SharedData
{
public:

  ServiceEventData(const ServiceEvent::Type& type, const ServiceReference& reference)
    : type(type), reference(reference)
  {

  }

  ServiceEventData(const ServiceEventData& other)
    : SharedData(other), type(other.type), reference(other.reference)
  {

  }

  const ServiceEvent::Type type;
  const ServiceReference reference;
};

ServiceEvent::ServiceEvent()
  : d(0)
{

}

ServiceEvent::~ServiceEvent()
{

}

bool ServiceEvent::IsNull() const
{
  return !d;
}

ServiceEvent::ServiceEvent(Type type, const ServiceReference& reference)
  : d(new ServiceEventData(type, reference))
{

}

ServiceEvent::ServiceEvent(const ServiceEvent& other)
  : d(other.d)
{

}

ServiceEvent& ServiceEvent::operator=(const ServiceEvent& other)
{
  d = other.d;
  return *this;
}

ServiceReference ServiceEvent::GetServiceReference() const
{
  return d->reference;
}

ServiceEvent::Type ServiceEvent::GetType() const
{
  return d->type;
}

}

std::ostream& operator<<(std::ostream& os, const mitk::ServiceEvent::Type& type)
{
  switch(type)
  {
  case mitk::ServiceEvent::MODIFIED:          return os << "MODIFIED";
  case mitk::ServiceEvent::MODIFIED_ENDMATCH: return os << "MODIFIED_ENDMATCH";
  case mitk::ServiceEvent::REGISTERED:        return os << "REGISTERED";
  case mitk::ServiceEvent::UNREGISTERING:     return os << "UNREGISTERING";

  default: return os << "unknown service event type (" << static_cast<int>(type) << ")";
  }
}

std::ostream& operator<<(std::ostream& os, const mitk::ServiceEvent& event)
{
  if (event.IsNull()) return os << "NONE";

  os << event.GetType();

  mitk::ServiceReference sr = event.GetServiceReference();
  if (sr)
  {
    // Some events will not have a service reference
    long int sid = mitk::any_cast<long int>(sr.GetProperty(mitk::ServiceConstants::SERVICE_ID()));
    os << " " << sid;

    mitk::Any classes = sr.GetProperty(mitk::ServiceConstants::OBJECTCLASS());
    os << " objectClass=" << classes << ")";
  }

  return os;
}
