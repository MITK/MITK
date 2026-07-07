/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodeSelectionService_h
#define mitkNodeSelectionService_h

#include <mitkINodeSelectionService.h>

#include <mutex>
#include <unordered_map>

namespace mitk
{
  /**
   * \brief Implementation of the INodeSelectionService interface.
   *
   * Provides a thread-safe mechanism for broadcasting DataNode selections to
   * registered listeners within a given context string. Multiple listeners can
   * be registered for the same context. All operations are guarded by a mutex.
   *
   * \sa INodeSelectionService
   * \sa INodeSelectionListener
   */
  class NodeSelectionService : public INodeSelectionService
  {
  public:
    NodeSelectionService();
    ~NodeSelectionService() override;

    /**
     * \brief Register a listener for selection events in the given context.
     *
     * If the listener is already registered for the given context, it is not added again.
     *
     * \param[in] context The selection context identifier. Must not be empty.
     * \param[in] listener The listener to register. Must not be nullptr.
     * \return \c true if the arguments are valid, \c false otherwise.
     */
    bool AddListener(const std::string& context, INodeSelectionListener* listener) override;

    /**
     * \brief Remove a listener from a specific context.
     *
     * \param[in] context The selection context identifier. Must not be empty.
     * \param[in] listener The listener to remove. Must not be nullptr.
     * \return \c true if the listener was found and removed, \c false otherwise.
     */
    bool RemoveListener(const std::string& context, const INodeSelectionListener* listener) override;

    /**
     * \brief Remove a listener from all contexts.
     *
     * \param[in] listener The listener to remove. Must not be nullptr.
     * \return \c true if at least one registration was removed, \c false otherwise.
     */
    bool RemoveListener(const INodeSelectionListener* listener) override;

    /**
     * \brief Send a selection to all listeners registered for the given context.
     *
     * Calls INodeSelectionListener::OnSelectionReceived on each listener registered
     * for the given context.
     *
     * \param[in] context The selection context identifier. Must not be empty.
     * \param[in] selection The DataNode pointers representing the selection.
     * \return \c true if the context is non-empty, \c false otherwise.
     */
    bool SendSelection(const std::string& context, const std::vector<mitk::DataNode::Pointer>& selection) const override;

  private:
    std::unordered_multimap<std::string, INodeSelectionListener*> m_Listeners; ///< Listeners keyed by context.
    mutable std::mutex m_Mutex; ///< Guards all access to m_Listeners.
  };
}

#endif
