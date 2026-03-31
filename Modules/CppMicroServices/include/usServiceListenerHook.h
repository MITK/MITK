/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USSERVICELISTENERHOOK_H
#define USSERVICELISTENERHOOK_H

#include <usServiceInterface.h>
#include <usShrinkableVector.h>
#include <usSharedData.h>

#include <string>

namespace us {

class ModuleContext;
class ServiceListenerEntry;

/**
 * \ingroup MicroServices
 *
 * \brief Service Listener Hook Service.
 *
 * <p>
 * Modules registering this service will be called during service listener
 * addition and removal.
 *
 * \remarks Implementations of this interface are required to be thread-safe.
 */
struct MITKCPPMICROSERVICES_EXPORT ServiceListenerHook
{

  class ListenerInfoData;

  /**
   * \brief Information about a Service Listener. This class describes the module
   * which added the Service Listener and the filter with which it was added.
   *
   * \remarks This class is not intended to be implemented by clients.
   */
  struct MITKCPPMICROSERVICES_EXPORT ListenerInfo
  {
    /** \brief Default constructor. Creates an invalid ListenerInfo instance. */
    ListenerInfo();

    /** \brief Copy constructor. */
    ListenerInfo(const ListenerInfo& other);

    /** \brief Destructor. */
    ~ListenerInfo();

    /** \brief Copy assignment operator. */
    ListenerInfo& operator=(const ListenerInfo& other);

    /**
     * \brief Can be used to check if this ListenerInfo instance is valid,
     * or if it has been constructed using the default constructor.
     *
     * \return <code>true</code> if this listener object is valid,
     *         <code>false</code> otherwise.
     */
    bool IsNull() const;

    /**
     * \brief Return the context of the module which added the listener.
     *
     * \pre IsNull() returns \c false.
     *
     * \return The context of the module which added the listener.
     */
    ModuleContext* GetModuleContext() const;

    /**
     * \brief Return the filter string with which the listener was added.
     *
     * \pre IsNull() returns \c false.
     *
     * \return The filter string with which the listener was added. This may
     *         be empty if the listener was added without a filter.
     */
    std::string GetFilter() const;

    /**
     * \brief Return the state of the listener for this addition and removal life
     * cycle. Initially this method will return \c false indicating the
     * listener has been added but has not been removed. After the listener
     * has been removed, this method must always returns \c true.
     *
     * <p>
     * There is an extremely rare case in which removed notification to
     * {@link ServiceListenerHook}s can be made before added notification if two
     * threads are racing to add and remove the same service listener.
     * Because {@link ServiceListenerHook}s are called synchronously during service
     * listener addition and removal, the CppMicroServices library cannot guarantee
     * in-order delivery of added and removed notification for a given
     * service listener. This method can be used to detect this rare
     * occurrence.
     *
     * \return \c false if the listener has not been been removed,
     *         \c true otherwise.
     */
    bool IsRemoved() const;

    /**
     * \brief Compares this \c ListenerInfo to another \c ListenerInfo.
     * Two {@code ListenerInfo}s are equal if they refer to the same
     * listener for a given addition and removal life cycle. If the same
     * listener is added again, it will have a different
     * \c ListenerInfo which is not equal to this \c ListenerInfo.
     *
     * \param[in] other The object to compare against this \c ListenerInfo.
     * \return \c true if the other object is a \c ListenerInfo
     *         object and both objects refer to the same listener for a
     *         given addition and removal life cycle.
     */
    bool operator==(const ListenerInfo& other) const;

  private:

    friend class ServiceListenerEntry;

    friend struct std::hash<ServiceListenerHook::ListenerInfo>;

    ListenerInfo(ListenerInfoData* data);

    ExplicitlySharedDataPointer<ListenerInfoData> d;
  };

  /** \brief Destructor. */
  virtual ~ServiceListenerHook();

  /**
   * \brief Added listeners hook method. This method is called to provide the hook
   * implementation with information on newly added service listeners. This
   * method will be called as service listeners are added while this hook is
   * registered. Also, immediately after registration of this hook, this
   * method will be called to provide the current collection of service
   * listeners which had been added prior to the hook being registered.
   *
   * \param[in] listeners A collection of \c ListenerInfo objects for newly added
   *        service listeners which are now listening to service events.
   */
  virtual void Added(const std::vector<ListenerInfo>& listeners) = 0;

  /**
   * \brief Removed listeners hook method. This method is called to provide the hook
   * implementation with information on newly removed service listeners. This
   * method will be called as service listeners are removed while this hook is
   * registered.
   *
   * \param[in] listeners A collection of \c ListenerInfo objects for newly removed
   *        service listeners which are no longer listening to service events.
   */
  virtual void Removed(const std::vector<ListenerInfo>& listeners) = 0;

};

}

namespace std {
template<> struct hash<us::ServiceListenerHook::ListenerInfo> { std::size_t operator()(const us::ServiceListenerHook::ListenerInfo& arg) const {
  return std::hash<const us::ServiceListenerHook::ListenerInfoData*>()(arg.d.Data());
} };
}

#endif // USSERVICELISTENERHOOK_H
