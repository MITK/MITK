/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USSERVICETRACKERPRIVATE_H
#define USSERVICETRACKERPRIVATE_H

#include <usServiceReference.h>
#include <usLDAPFilter.h>


namespace us {

/**
 * \ingroup MicroServices
 */
template<class S, class TTT>
class ServiceTrackerPrivate : MultiThreaded<>
{

public:

  /** \brief The tracked object type. */
  typedef typename TTT::TrackedType T;

  /** \brief Constructs the private implementation tracking a specific service reference. */
  ServiceTrackerPrivate(ServiceTracker<S,TTT>* st,
                        ModuleContext* context,
                        const ServiceReference<S>& reference,
                        ServiceTrackerCustomizer<S,T>* customizer);

  /** \brief Constructs the private implementation tracking by class name. */
  ServiceTrackerPrivate(ServiceTracker<S,TTT>* st,
                        ModuleContext* context, const std::string& clazz,
                        ServiceTrackerCustomizer<S,T>* customizer);

  /** \brief Constructs the private implementation tracking by LDAP filter. */
  ServiceTrackerPrivate(ServiceTracker<S,TTT>* st,
                        ModuleContext* context, const LDAPFilter& filter,
                        ServiceTrackerCustomizer<S,T>* customizer);

  /** \brief Destructor. */
  ~ServiceTrackerPrivate();

  /**
   * \brief Returns the list of initial <code>ServiceReference</code>s that will be
   * tracked by this <code>ServiceTracker</code>.
   *
   * \param[in] className The class name with which the service was registered, or
   *        <code>null</code> for all services.
   * \param[in] filterString The filter criteria or <code>null</code> for all
   *        services.
   * \return The list of initial <code>ServiceReference</code>s.
   * \throws std::invalid_argument If the specified filterString has an
   *         invalid syntax.
   */
  std::vector<ServiceReference<S> > GetInitialReferences(const std::string& className,
                                                         const std::string& filterString);

  /** \brief Retrieves service references without holding the lock. */
  void GetServiceReferences_unlocked(std::vector<ServiceReference<S> >& refs, TrackedService<S,TTT>* t) const;

  /* set this to true to compile in debug messages */

  /**
   * \brief The Module Context used by this <code>ServiceTracker</code>.
   */
  ModuleContext* const context;

  /**
   * \brief The filter used by this <code>ServiceTracker</code> which specifies the
   * search criteria for the services to track.
   */
  LDAPFilter filter;

  /**
   * \brief The <code>ServiceTrackerCustomizer</code> for this tracker.
   */
  ServiceTrackerCustomizer<S,T>* customizer;

  /**
   * \brief Filter string for use when adding the ServiceListener. If this field is
   * set, then certain optimizations can be taken since we don't have a user
   * supplied filter.
   */
  std::string listenerFilter;

  /**
   * \brief Class name to be tracked. If this field is set, then we are tracking by
   * class name.
   */
  std::string trackClass;

  /**
   * \brief Reference to be tracked. If this field is set, then we are tracking a
   * single ServiceReference.
   */
  ServiceReference<S> trackReference;

  /**
   * \brief Tracked services: <code>ServiceReference</code> -> customized Object and
   * <code>ServiceListenerEntry</code> object
   */
  TrackedService<S,TTT>* trackedService;

  /**
   * \brief Accessor method for the current TrackedService object. This method is only
   * intended to be used by the unsynchronized methods which do not modify the
   * trackedService field.
   *
   * \return The current Tracked object.
   */
  TrackedService<S,TTT>* Tracked() const;

  /**
   * \brief Called by the TrackedService object whenever the set of tracked services is
   * modified. Clears the cache.
   */
  /*
   * This method must not be synchronized since it is called by TrackedService while
   * TrackedService is synchronized. We don't want synchronization interactions
   * between the listener thread and the user thread.
   */
  void Modified();

  /**
   * \brief Cached ServiceReference for getServiceReference.
   */
  mutable ServiceReference<S> cachedReference;

  /**
   * \brief Cached service object for GetService.
   */
  mutable T cachedService;


private:

  inline ServiceTracker<S,TTT>* q_func()
  {
    return static_cast<ServiceTracker<S,TTT> *>(q_ptr);
  }

  inline const ServiceTracker<S,TTT>* q_func() const
  {
    return static_cast<const ServiceTracker<S,TTT> *>(q_ptr);
  }

  friend class ServiceTracker<S,TTT>;

  ServiceTracker<S,TTT> * const q_ptr;

};

}

#include <usServiceTrackerPrivate.tpp>

#endif // USSERVICETRACKERPRIVATE_H
