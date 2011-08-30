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


#ifndef MITKSERVICELISTENERENTRY_H
#define MITKSERVICELISTENERENTRY_H

#include "mitkLDAPExpr_p.h"

#include "mitkModuleContext.h"

namespace mitk {

class Module;
class ServiceListenerEntryData;

/**
 * Data structure for saving service listener info. Contains
 * the optional service listener filter, in addition to the info
 * in ListenerEntry.
 */
class ServiceListenerEntry
{

public:

  ServiceListenerEntry(const ServiceListenerEntry& other);
  ~ServiceListenerEntry();
  ServiceListenerEntry& operator=(const ServiceListenerEntry& other);

  bool operator==(const ServiceListenerEntry& other) const;

  bool operator<(const ServiceListenerEntry& other) const;

  void SetRemoved(bool removed) const;
  bool IsRemoved() const;

  ServiceListenerEntry(Module* mc, const ServiceListenerDelegate& l, const std::string& filter = "");

  Module* GetModule() const;

  std::string GetFilter() const;

  LDAPExpr GetLDAPExpr() const;

  LDAPExpr::LocalCache& GetLocalCache() const;

  void CallDelegate(const ServiceEvent& event) const;

private:

#ifdef MITK_HAS_UNORDERED_MAP_H
  friend class std::hash<ServiceListenerEntry>;
#elif defined(__GNUC__)
  friend class __gnu_cxx::hash<ServiceListenerEntry>;
#else
  friend struct itk::hash<ServiceListenerEntry>;
#endif

  ExplicitlySharedDataPointer<ServiceListenerEntryData> d;
};

}

#ifdef MITK_HAS_UNORDERED_MAP_H
namespace std {
#elif defined(__GNUC__)
namespace __gnu_cxx {
#else
namespace itk {
#endif

template<>
class hash<mitk::ServiceListenerEntry> : public std::unary_function<mitk::ServiceListenerEntry, size_t>
{
public:
  size_t operator()(const mitk::ServiceListenerEntry& sle) const
  {
#ifdef MITK_HAS_HASH_SIZE_T
    return hash<std::size_t>()(reinterpret_cast<std::size_t>(sle.d.ConstData()));
#else
    unsigned long long key = reinterpret_cast<unsigned long long>(sle.d.ConstData());
    if (sizeof(unsigned long long) > sizeof(std::size_t))
    {
      return std::size_t(((key >> (8 * sizeof(std::size_t) - 1)) ^ key) & (~0U));
    }
    else
    {
      return std::size_t(key & (~0U));
    }
#endif
  }
};

}

#endif // MITKSERVICELISTENERENTRY_H
