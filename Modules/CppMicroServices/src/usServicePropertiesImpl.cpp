/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "usServicePropertiesImpl_p.h"

#include <limits>
#include <stdexcept>
#ifdef US_PLATFORM_WINDOWS
#include <string.h>
#define ci_compare strnicmp
#else
#include <strings.h>
#define ci_compare strncasecmp
#endif

namespace us {

Any ServicePropertiesImpl::emptyAny;

ServicePropertiesImpl::ServicePropertiesImpl(const ServiceProperties& p)
{
  if (p.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
  {
    throw std::runtime_error("ServiceProperties object contains too many keys");
  }

  keys.reserve(p.size());
  values.reserve(p.size());

  for (ServiceProperties::const_iterator iter = p.begin();
       iter != p.end(); ++iter)
  {
    if (Find(iter->first) > -1)
    {
      std::string msg = "ServiceProperties object contains case variants of the key: ";
      msg += iter->first;
      throw std::runtime_error(msg.c_str());
    }
    keys.push_back(iter->first);
    values.push_back(iter->second);
  }
}

const Any& ServicePropertiesImpl::Value(const std::string& key) const
{
  int i = Find(key);
  if (i < 0) return emptyAny;
  return values[i];
}

const Any& ServicePropertiesImpl::Value(int index) const
{
  if (index < 0 || static_cast<std::size_t>(index) >= values.size())
    return emptyAny;
  return values[static_cast<std::size_t>(index)];
}

int ServicePropertiesImpl::Find(const std::string& key) const
{
  for (std::size_t i = 0; i < keys.size(); ++i)
  {
    if (key.size() == keys[i].size() && ci_compare(key.c_str(), keys[i].c_str(), key.size()) == 0)
    {
      return static_cast<int>(i);
    }
  }
  return -1;
}

int ServicePropertiesImpl::FindCaseSensitive(const std::string& key) const
{
  for (std::size_t i = 0; i < keys.size(); ++i)
  {
    if (key == keys[i])
    {
      return static_cast<int>(i);
    }
  }
  return -1;
}

const std::vector<std::string>& ServicePropertiesImpl::Keys() const
{
  return keys;
}

}
