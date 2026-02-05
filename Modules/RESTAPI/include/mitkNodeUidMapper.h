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
   * @brief Maintains transient UID <-> DataNode mappings.
   *
   * UIDs are:
   * - Generated on first access
   * - Stored as node property for debugging visibility
   * - Cleared from cache when node removed from DataStorage
   * - Never reused during session lifetime
   * - NOT persisted when scene is saved (transient property)
   */
  class MITKRESTAPI_EXPORT NodeUidMapper
  {
  public:
    /// Property key for storing UID on nodes (for debugging)
    static constexpr const char* UID_PROPERTY_KEY = "restapi.uid";

    NodeUidMapper();
    ~NodeUidMapper();

    // Non-copyable, non-movable
    NodeUidMapper(const NodeUidMapper&) = delete;
    NodeUidMapper& operator=(const NodeUidMapper&) = delete;
    NodeUidMapper(NodeUidMapper&&) = delete;
    NodeUidMapper& operator=(NodeUidMapper&&) = delete;

    /**
     * @brief Connect to a DataStorage and start tracking nodes.
     *
     * Subscribes to AddNodeEvent and RemoveNodeEvent.
     * Clears any existing restapi.uid properties (fresh start).
     *
     * @param dataStorage The DataStorage to track, or nullptr to disconnect.
     */
    void SetDataStorage(DataStorage* dataStorage);

    /**
     * @brief Disconnect from current DataStorage.
     */
    void ClearDataStorage();

    /**
     * @brief Get or create a UID for the given node.
     *
     * Thread-safe. Creates new UID if none exists.
     *
     * @param node The node to get/create UID for. Must not be nullptr.
     * @return The UID for this node.
     * @throws std::invalid_argument if node is nullptr.
     */
    std::string GetOrCreateUid(const DataNode* node);

    /**
     * @brief Find a node by its UID.
     *
     * Thread-safe. Returns nullptr if not found.
     *
     * @param uid The UID to look up.
     * @return The node with this UID, or nullptr if not found.
     */
    DataNode* FindNodeByUid(const std::string& uid) const;

    /**
     * @brief Get UID for a node if it exists (doesn't create).
     *
     * @param node The node to look up.
     * @return The UID if it exists, or std::nullopt if not.
     */
    std::optional<std::string> GetUid(const DataNode* node) const;

    /**
     * @brief Check if a UID exists.
     *
     * @param uid The UID to check.
     * @return true if the UID exists in the mapping.
     */
    bool HasUid(const std::string& uid) const;

    /**
     * @brief Restore a UID mapping for a node.
     *
     * Used to preserve UIDs across operations that temporarily remove and re-add
     * nodes to the DataStorage (e.g., reparenting). The node's previous UID is
     * re-registered in the cache and stored as a property.
     *
     * @param node The node to restore the UID for. Must not be nullptr.
     * @param uid The UID to restore.
     * @throws std::invalid_argument if node is nullptr.
     */
    void RestoreUid(const DataNode* node, const std::string& uid);

  private:
    void OnNodeAdded(const DataNode* node);
    void OnNodeRemoved(const DataNode* node);
    std::string GenerateUid();
    void ClearRESTUIDProperty();

    DataStorage* m_DataStorage = nullptr;

    // Thread-safe UID mappings
    mutable std::mutex m_Mutex;
    std::unordered_map<std::string, DataNode*> m_UidToNode;
    std::unordered_map<const DataNode*, std::string> m_NodeToUid;

    // Counter for unique ID generation (never reused in session)
    std::atomic<uint64_t> m_UidCounter{0};

    // Track if we're connected
    bool m_Connected = false;
  };
}

#endif
