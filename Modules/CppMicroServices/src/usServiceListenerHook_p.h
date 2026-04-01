/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USSERVICELISTENERHOOK_P_H
#define USSERVICELISTENERHOOK_P_H

#include <usServiceListenerHook.h>
#include "usServiceListenerEntry_p.h"
#include <usSharedData.h>

namespace us {

/**
 * \brief Private data class for ServiceListenerHook::ListenerInfo.
 *
 * Stores the module context, listener callback, associated data, LDAP filter
 * string, and removal state for a service listener.
 *
 * \sa ServiceListenerHook::ListenerInfo ServiceListenerEntry
 */
class ServiceListenerHook::ListenerInfoData : public SharedData
{
public:
  /** \brief Construct listener info data.
   *  \param[in] mc The module context that registered the listener.
   *  \param[in] l The service listener callback.
   *  \param[in] data Additional data to distinguish listener objects.
   *  \param[in] filter The LDAP filter string for this listener.
   */
  ListenerInfoData(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& l,
                   void* data, const std::string& filter);

  /** \brief Destructor. */
  virtual ~ListenerInfoData();

  /** \brief The module context that registered the listener. */
  ModuleContext* const mc;

  /** \brief The service listener callback. */
  ServiceListenerEntry::ServiceListener listener;

  /** \brief Additional data used to distinguish listener objects. */
  void* data;

  /** \brief The LDAP filter string for this listener. */
  std::string filter;

  /** \brief Whether this listener has been removed. */
  bool bRemoved;
};

}

#endif // USSERVICELISTENERHOOK_P_H
