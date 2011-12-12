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

#include "mitkServiceReference.h"
#include "mitkServiceReferencePrivate.h"
#include "mitkServiceRegistrationPrivate.h"

#include "mitkModule.h"
#include "mitkModulePrivate.h"

#include <itkMutexLockHolder.h>

namespace mitk {

typedef ServiceRegistrationPrivate::MutexType MutexType;
typedef itk::MutexLockHolder<MutexType> MutexLocker;

ServiceReference::ServiceReference()
  : d(new ServiceReferencePrivate(0))
{

}

ServiceReference::ServiceReference(const ServiceReference& ref)
  : d(ref.d)
{
  d->ref.Ref();
}

ServiceReference::ServiceReference(ServiceRegistrationPrivate* reg)
  : d(new ServiceReferencePrivate(reg))
{

}

ServiceReference::operator bool() const
{
  return GetModule() != 0;
}

ServiceReference& ServiceReference::operator=(int null)
{
  if (null == 0)
  {
    if (!d->ref.Deref())
      delete d;
    d = new ServiceReferencePrivate(0);
  }
  return *this;
}

ServiceReference::~ServiceReference()
{
  if (!d->ref.Deref())
    delete d;
}

Any ServiceReference::GetProperty(const std::string& key) const
{
  MutexLocker lock(d->registration->propsLock);

  ServiceProperties::const_iterator iter = d->registration->properties.find(key);
  if (iter != d->registration->properties.end())
    return iter->second;
  return Any();
}

void ServiceReference::GetPropertyKeys(std::vector<std::string>& keys) const
{
  MutexLocker lock(d->registration->propsLock);

  ServiceProperties::const_iterator iterEnd = d->registration->properties.end();
  for (ServiceProperties::const_iterator iter = d->registration->properties.begin();
       iter != iterEnd; ++iter)
  {
    keys.push_back(iter->first);
  }
}

Module* ServiceReference::GetModule() const
{
  if (d->registration == 0 || d->registration->module == 0)
  {
    return 0;
  }

  return d->registration->module->q;
}

void ServiceReference::GetUsingModules(std::vector<Module*>& modules) const
{
  MutexLocker lock(d->registration->propsLock);

  ServiceRegistrationPrivate::ModuleToRefsMap::const_iterator end = d->registration->dependents.end();
  for (ServiceRegistrationPrivate::ModuleToRefsMap::const_iterator iter = d->registration->dependents.begin();
       iter != end; ++iter)
  {
    modules.push_back(iter->first);
  }
}

bool ServiceReference::operator<(const ServiceReference& reference) const
{
  int r1 = 0;
  int r2 = 0;

  mitk::Any anyR1 = GetProperty(ServiceConstants::SERVICE_RANKING());
  mitk::Any anyR2 = reference.GetProperty(ServiceConstants::SERVICE_RANKING());
  if (anyR1.Type() == typeid(int)) r1 = any_cast<int>(anyR1);
  if (anyR2.Type() == typeid(int)) r2 = any_cast<int>(anyR2);

  if (r1 != r2)
  {
    // use ranking if ranking differs
    return r1 < r2;
  }
  else
  {
    long int id1 = any_cast<long int>(GetProperty(ServiceConstants::SERVICE_ID()));
    long int id2 = any_cast<long int>(reference.GetProperty(ServiceConstants::SERVICE_ID()));

    // otherwise compare using IDs,
    // is less than if it has a higher ID.
    return id2 < id1;
  }
}

bool ServiceReference::operator==(const ServiceReference& reference) const
{
  return d->registration == reference.d->registration;
}

ServiceReference& ServiceReference::operator=(const ServiceReference& reference)
{
  ServiceReferencePrivate* curr_d = d;
  d = reference.d;
  d->ref.Ref();

  if (!curr_d->ref.Deref())
    delete curr_d;

  return *this;
}

}

std::ostream& operator<<(std::ostream& os, const mitk::ServiceReference& serviceRef)
{
  os << "Reference for service object registered from "
     << serviceRef.GetModule()->GetName() << " " << serviceRef.GetModule()->GetVersion()
     << " (";
  std::vector<std::string> keys;
  serviceRef.GetPropertyKeys(keys);
  int keySize = keys.size();
  for(int i = 0; i < keySize; ++i)
  {
    os << keys[i] << "=" << serviceRef.GetProperty(keys[i]);
    if (i < keySize-1) os << ",";
  }
  os << ")";

  return os;
}

#ifdef MITK_HAS_UNORDERED_MAP_H
namespace std {
#elif defined(__GNUC__)
namespace __gnu_cxx {
#else
namespace itk {
#endif

std::size_t hash<mitk::ServiceReference>::operator()(const mitk::ServiceReference& sr) const
{
#ifdef MITK_HAS_HASH_SIZE_T
  return hash<std::size_t>()(reinterpret_cast<std::size_t>(sr.d->registration));
#else
  std::size_t key = reinterpret_cast<std::size_t>(sr.d->registration);
  return std::size_t(key & (~0U));
#endif
}

}
