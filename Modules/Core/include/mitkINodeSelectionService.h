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
   * \brief A service for requesting node selections from listeners.
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
     * sender and receiver agreed upon. A listener must not be \c nullptr.
     */
    virtual bool AddListener(const std::string& context, INodeSelectionListener* listener) = 0;

    /**
     * \brief Remove a listener from selections of a certain selection context.
     *
     * \sa AddListener()
     */
    virtual bool RemoveListener(const std::string& context, const INodeSelectionListener* listener) = 0;

    /**
     * \brief Send a selection to all listeners of a certain selection context.
     *
     * \sa AddListener()
     */
    virtual bool SendSelection(const std::string& context, const std::vector<mitk::DataNode::Pointer>& selection) const = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::INodeSelectionService, "org.mitk.INodeSelectionService")

#endif
