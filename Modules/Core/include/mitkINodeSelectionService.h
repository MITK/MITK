/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkINodeSelectionService_h
#define mitkINodeSelectionService_h

#include <mitkDataNode.h>
#include <mitkINodeSelectionListener.h>
#include <mitkServiceInterface.h>

namespace mitk
{
  /**
   * \brief A simple messaging service for sending and receiving data node
   *        selections for certain contexts.
   *
   * \sa CoreServices::GetPreferencesService()
   * \sa INodeSelectionListener
   *
   * \ingroup MicroServices_Interfaces
   */
  class MITKCORE_EXPORT INodeSelectionService
  {
  public:
    virtual ~INodeSelectionService();

    /**
     * \brief Add a listener for selections in a certain selection context.
     *
     * A selection context is a non-empty, arbitrary and unique string that both
     * sender and receiver agreed upon. A listener must not be \c nullptr and
     * can be added only once.
     *
     * \param context A non-empty, arbitrary and unique string that both
     *        sender and receiver agreed upon.
     * \param listener A valid pointer to a class implementing the listener
     *        interface.
     *
     * \return \c true if the listener was successfully added or already has
     *         been added before, or \c false otherwise.
     */
    virtual bool AddListener(const std::string& context, INodeSelectionListener* listener) = 0;

    /**
     * \brief Remove a listener from selections of a certain selection context.
     *
     * \param context A non-empty, arbitrary and unique string that both
     *        sender and receiver agreed upon.
     * \param listener A pointer to an already added listener.
     *
     * \return \c true if the listener was actually found for the given context,
     *         or \c false otherwise.
     *
     * \sa AddListener()
     */
    virtual bool RemoveListener(const std::string& context, const INodeSelectionListener* listener) = 0;

    /**
     * \brief Send a selection to all listeners of a certain selection context.
     *
     * \param context A non-empty, arbitrary and unique string that both
     *        sender and receiver agreed upon.
     * \param selection A selection of potentially multiple data nodes to send
     *        to any listeners for the given selection context.
     *
     * \sa AddListener()
     */
    virtual bool SendSelection(const std::string& context, const std::vector<mitk::DataNode::Pointer>& selection) const = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::INodeSelectionService, "org.mitk.INodeSelectionService")

#endif
