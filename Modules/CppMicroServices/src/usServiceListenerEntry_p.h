/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USSERVICELISTENERENTRY_H
#define USSERVICELISTENERENTRY_H

#include <usUtils_p.h>
#include <usServiceListenerHook.h>
#include <usListenerFunctors_p.h>

#include "usLDAPExpr_p.h"

namespace us {

class Module;
class ServiceListenerEntryData;

/**
 * \brief Data structure for saving service listener info.
 *
 * Contains the optional service listener filter, in addition to the info
 * in ListenerEntry.
 *
 * \sa ServiceListenerHook::ListenerInfo
 */
class ServiceListenerEntry : public ServiceListenerHook::ListenerInfo
{

public:

  typedef US_SERVICE_LISTENER_FUNCTOR ServiceListener;

  /** \brief Copy constructor.
   *  \param[in] other The entry to copy from.
   */
  ServiceListenerEntry(const ServiceListenerEntry& other);

  /** \brief Construct from a ListenerInfo object.
   *  \param[in] info The listener info to construct from.
   */
  ServiceListenerEntry(const ServiceListenerHook::ListenerInfo& info);

  /** \brief Destructor. */
  ~ServiceListenerEntry();

  /** \brief Copy assignment operator.
   *  \param[in] other The entry to assign from.
   *  \return Reference to this object.
   */
  ServiceListenerEntry& operator=(const ServiceListenerEntry& other);

  /** \brief Mark this entry as removed or not.
   *  \param[in] removed Whether the entry should be marked as removed.
   */
  void SetRemoved(bool removed) const;

  /** \brief Construct a new service listener entry.
   *  \param[in] mc The module context that owns this listener.
   *  \param[in] l The service listener callback.
   *  \param[in] data Additional data to distinguish listener objects.
   *  \param[in] filter An LDAP filter string for this listener.
   */
  ServiceListenerEntry(ModuleContext* mc, const ServiceListener& l, void* data, const std::string& filter = "");

  /** \brief Get the LDAP expression for this listener's filter.
   *  \return The parsed LDAP expression.
   */
  const LDAPExpr& GetLDAPExpr() const;

  /** \brief Get the local cache for LDAP expression evaluation.
   *  \return Reference to the local cache.
   */
  LDAPExpr::LocalCache& GetLocalCache() const;

  /** \brief Invoke the service listener delegate with the given event.
   *  \param[in] event The service event to deliver.
   */
  void CallDelegate(const ServiceEvent& event) const;

  /** \brief Equality comparison operator.
   *  \param[in] other The entry to compare with.
   *  \return \c true if both entries refer to the same listener.
   */
  bool operator==(const ServiceListenerEntry& other) const;

  /** \brief Compute a hash value for this entry.
   *  \return The hash value.
   */
  std::size_t Hash() const;

};

}

namespace std {
template<> struct hash<us::ServiceListenerEntry> { std::size_t operator()(const us::ServiceListenerEntry& arg) const {
  return arg.Hash();
} };
}

#endif // USSERVICELISTENERENTRY_H
