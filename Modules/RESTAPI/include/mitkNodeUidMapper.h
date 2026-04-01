/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodeUidMapper_h
#define mitkNodeUidMapper_h

#include <mitkDataStorage.h>
#include <mitkDataNode.h>

#include <unordered_map>
#include <mutex>
#include <string>
#include <optional>
#include <atomic>

#include <MitkRESTAPIExports.h>

namespace mitk
{
  /**
   * \brief Maintains transient UID-to-DataNode mappings for the REST API.
   *
   * This class provides a bidirectional mapping between string UIDs and DataNode
   * pointers. UIDs are:
   *   - Generated on first access (format: "node_<counter>")
   *   - Stored as a node property under the key "restapi.uid" for debugging visibility
   *   - Cleared from the cache when the node is removed from the DataStorage
   *   - Never reused during a session lifetime
   *   - NOT persisted when a scene is saved (transient property)
   *
   * All public methods are thread-safe.
   *
   * \sa DataStorageBridge
   */
  class MITKRESTAPI_EXPORT NodeUidMapper
  {
  public:
    /** \brief Property key for storing the UID on nodes (for debugging visibility). */
    static constexpr const char* UID_PROPERTY_KEY = "restapi.uid";

    /** \brief Default constructor. */
    NodeUidMapper();

    /** \brief Destructor. Disconnects from the DataStorage if connected. */
    ~NodeUidMapper();

    NodeUidMapper(const NodeUidMapper&) = delete;
    NodeUidMapper& operator=(const NodeUidMapper&) = delete;
    NodeUidMapper(NodeUidMapper&&) = delete;
    NodeUidMapper& operator=(NodeUidMapper&&) = delete;

    /**
     * \brief Connect to a DataStorage and start tracking nodes.
     *
     * Subscribes to AddNodeEvent and RemoveNodeEvent. Clears all existing
     * mappings and removes any stale "restapi.uid" properties from all nodes
     * in the DataStorage (fresh start).
     *
     * \param[in] dataStorage The DataStorage to track, or \c nullptr to disconnect.
     */
    void SetDataStorage(DataStorage* dataStorage);

    /**
     * \brief Disconnect from the current DataStorage.
     *
     * Equivalent to calling SetDataStorage(nullptr).
     */
    void ClearDataStorage();

    /**
     * \brief Get or create a UID for the given node.
     *
     * If the node already has a cached UID, it is returned. Otherwise a new
     * unique UID is generated, stored in the cache, and set as a property
     * on the node.
     *
     * \param[in] node The node to get or create a UID for. Must not be \c nullptr.
     * \return The UID string for this node.
     * \throw std::invalid_argument if \p node is \c nullptr.
     */
    std::string GetOrCreateUid(DataNode* node);

    /**
     * \brief Find a node by its UID.
     *
     * Returns a strong SmartPointer reference to guarantee the node stays alive
     * after the internal lock is released.
     *
     * \param[in] uid The UID to look up.
     * \return A SmartPointer to the node, or a null pointer if not found or if
     *         the node has already been destroyed.
     */
    DataNode::Pointer FindNodeByUid(const std::string& uid) const;

    /**
     * \brief Get the UID for a node without creating one.
     *
     * \param[in] node The node to look up.
     * \return The UID if it exists, or \c std::nullopt if the node has no UID
     *         or \p node is \c nullptr.
     */
    std::optional<std::string> GetUid(const DataNode* node) const;

    /**
     * \brief Check whether a UID exists in the mapping.
     *
     * \param[in] uid The UID string to check.
     * \return \c true if the UID is registered in the mapping.
     */
    bool HasUid(const std::string& uid) const;

    /**
     * \brief Restore a previously assigned UID mapping for a node.
     *
     * Used to preserve UIDs across operations that temporarily remove and re-add
     * nodes to the DataStorage (e.g., reparenting). The node's previous UID is
     * re-registered in the cache and stored as a property.
     *
     * \param[in] node The node to restore the UID for. Must not be \c nullptr.
     * \param[in] uid The UID to restore.
     * \throw std::invalid_argument if \p node is \c nullptr.
     */
    void RestoreUid(const DataNode* node, const std::string& uid);

  private:
    void OnNodeAdded(const DataNode* node);
    void OnNodeRemoved(const DataNode* node);
    std::string GenerateUid();
    void ClearRESTUIDProperty();

    mitk::WeakPointer<DataStorage> m_DataStorage;

    // Thread-safe UID mappings
    mutable std::mutex m_Mutex;
    std::unordered_map<std::string, mitk::WeakPointer<DataNode>> m_UidToNode;
    std::unordered_map<const DataNode*, std::string> m_NodeToUid;

    // Counter for unique ID generation (never reused in session)
    std::atomic<uint64_t> m_UidCounter{0};

    // Track if we're connected
    bool m_Connected = false;
  };
}

#endif
