/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDataStorageBridge_h
#define mitkDataStorageBridge_h

#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>
#include <mitkBaseData.h>
#include <mitkNodePredicateBase.h>
#include "mitkNodeUidMapper.h"
#include "mitkNodeQueryParams.h"

#include <nlohmann/json.hpp>

#include <mutex>
#include <optional>
#include <string>
#include <memory>
#include <vector>

#include <MitkRESTAPIExports.h>

namespace mitk
{
  /**
   * @brief Thread-safe bridge to DataStorage for REST API operations.
   *
   * This class abstracts between the concrete REST/HTTP implementation
   * and MITK-specific handling of requests, translating between
   * DataStorage content and REST message content (as JSON).
   *
   * Provides:
   * - Thread-safe CRUD operations on DataNodes
   * - UID-based node identification
   * - JSON serialization of nodes and properties
   *
   * @section Internal Properties
   * Properties with keys starting with "restapi." are internal metadata
   * used by the REST API (e.g., "restapi.uid" for node identification).
   * These properties are:
   * - Filtered out from all property query responses (not leaked to clients)
   * - Protected from modification via SetNodeProperty
   * - Protected from deletion via DeleteNodeProperty and ReplaceNodeProperties
   * - Clients cannot set, get, or delete these properties via the API
   *
   * @section Modification Tracking
   * When a node is modified through the bridge (create, update, property changes),
   * the bridge automatically adds two internal properties to the node:
   * - "restapi.modified" (string): Set with a time stamp string of last modification.
   *   If this property exists, the node was modified via REST
   * - "restapi.lastmodification" (string): Describes the last modification operation
   *
   * These properties are only added when a modification occurs. If the properties
   * do not exist, the node has not been modified via the REST API (absence = not modified).
   * Read operations (GetNodes, GetNode, GetNodeProperties, etc.) do not set these properties.
   *
   * Possible lastmodification values:
   * - "created" - Node was created via CreateNode
   * - "reparented" - Node was moved to a different parent via UpdateNode
   * - "property_set:<key>" - A property was set via SetNodeProperty
   * - "property_deleted:<key>" - A property was deleted via DeleteNodeProperty
   * - "properties_replaced" - Properties were replaced via ReplaceNodeProperties
   *
   * These properties are useful for debugging and tracking REST API modifications
   * but are not exposed to clients through the API.
   *
   * All public methods are thread-safe.
   */
  class MITKRESTAPI_EXPORT DataStorageBridge
  {
  public:
    using Json = nlohmann::json;

    /// Prefix for internal REST API properties (filtered from responses, protected from modification)
    static constexpr const char* INTERNAL_PROPERTY_PREFIX = "restapi.";

    /// Property key for tracking if a node was modified via REST API (bool property)
    static constexpr const char* MODIFIED_PROPERTY_KEY = "restapi.modified";

    /// Property key for tracking the last modification operation (string property)
    static constexpr const char* LAST_MODIFICATION_PROPERTY_KEY = "restapi.lastmodification";

    DataStorageBridge();
    ~DataStorageBridge();

    // Non-copyable, non-movable
    DataStorageBridge(const DataStorageBridge&) = delete;
    DataStorageBridge& operator=(const DataStorageBridge&) = delete;
    DataStorageBridge(DataStorageBridge&&) = delete;
    DataStorageBridge& operator=(DataStorageBridge&&) = delete;

    /**
     * @brief Set the DataStorage to operate on.
     *
     * @param dataStorage The DataStorage, or nullptr to disconnect.
     */
    void SetDataStorage(DataStorage* dataStorage);

    /**
     * @brief Get the current DataStorage.
     *
     * @return The DataStorage, or nullptr if not connected.
     */
    DataStorage::Pointer GetDataStorage() const;

    /**
     * @brief Check if a DataStorage is connected.
     *
     * @return true if a DataStorage is available.
     */
    bool HasDataStorage() const;

    /**
     * @brief Get the UID for a DataNode.
     *
     * Creates a UID if the node doesn't have one yet.
     *
     * @param node The node to get the UID for. Must not be nullptr.
     * @return The UID for this node.
     * @throws std::invalid_argument if node is nullptr.
     */
    std::string GetNodeUid(const DataNode* node) const;

    // Node operations

    /**
     * @brief Result of a node query with pagination metadata.
     */
    struct NodeQueryResult
    {
      Json nodes;       ///< Array of node objects
      int totalCount;   ///< Total number of matching nodes (before pagination)
      int limit;        ///< Limit used in query
      int offset;       ///< Offset used in query
    };

    /**
     * @brief Get nodes with full query parameter support.
     *
     * @param params Query parameters including filters, pagination, etc.
     * @return Query result with nodes and metadata.
     */
    NodeQueryResult GetNodes(const NodeQueryParams& params) const;

    /**
     * @brief Get a single node by UID.
     *
     * @param uid The node UID.
     * @return JSON object for the node, or std::nullopt if not found.
     */
    std::optional<Json> GetNode(const std::string& uid) const;

    /**
     * @brief Result of a node creation operation.
     */
    struct CreateNodeResult
    {
      bool success;                               ///< Whether the node was created
      std::string uid;                            ///< UID of the created node (empty if failed)
      std::vector<std::string> failedProperties;  ///< Property keys that failed to deserialize
    };

    /**
     * @brief Create a new node.
     *
     * Creates a new DataNode with the specified name and properties.
     * Properties that fail to deserialize are logged and reported in the result,
     * but do not prevent node creation.
     *
     * @param nodeData JSON object with node data (name, properties).
     * @param parentUid Optional parent node UID. If provided, creates as child of parent.
     * @return CreateNodeResult with success status, UID, and any failed properties.
     */
    CreateNodeResult CreateNode(const Json& nodeData, const std::optional<std::string>& parentUid = std::nullopt);

    /**
     * @brief Update a node.
     *
     * Currently supports reparenting via "parent_uid" field.
     * Reparenting is done by removing the node from DataStorage and
     * re-adding it under the new parent (same approach as Data Manager UI).
     *
     * @param uid The node UID.
     * @param updates JSON object with fields to update (e.g., "parent_uid").
     * @return true if updated successfully.
     */
    bool UpdateNode(const std::string& uid, const Json& updates);

    /**
     * @brief Result of a node deletion operation.
     */
    struct DeleteResult
    {
      bool success;                         ///< Whether deletion succeeded
      std::string deletedUid;               ///< UID of the deleted node
      std::vector<std::string> deletedChildren;  ///< UIDs of deleted children (if recursive)
      int childrenCount;                    ///< Number of children (for error reporting)
    };

    /**
     * @brief Delete a node.
     *
     * @param uid The node UID.
     * @param recursive If true, also delete all children recursively.
     * @return DeleteResult with success status and deleted UIDs.
     */
    DeleteResult DeleteNode(const std::string& uid, bool recursive = false);

    // Data operations

    /**
     * @brief Result of a GetNodeData operation.
     */
    struct GetNodeDataResult
    {
      bool nodeFound;         ///< Whether the node with the given UID exists
      BaseData::Pointer data; ///< Clone of the data, or nullptr if node has no data
    };

    /**
     * @brief Get a clone of the node's data for thread-safe processing.
     *
     * Returns a clone of the BaseData attached to the node. The clone is
     * independent of the original data and can be safely processed (e.g.,
     * serialized) without holding locks and without affecting the original.
     *
     * This method is designed for scenarios where the caller needs to perform
     * potentially slow operations on the data (like serialization) without
     * blocking other DataStorage operations.
     *
     * @param uid The node UID.
     * @return GetNodeDataResult with nodeFound status and cloned data.
     *         - nodeFound=false: Node does not exist
     *         - nodeFound=true, data=nullptr: Node exists but has no data
     *         - nodeFound=true, data!=nullptr: Node exists and data is cloned
     */
    GetNodeDataResult GetNodeData(const std::string& uid) const;

    /**
     * @brief Set or replace the data on a node.
     *
     * Thread-safe assignment of BaseData to a node. The data is assigned
     * directly (not cloned), so the caller should not modify the data object
     * after calling this method.
     *
     * Can be used to:
     * - Set data on an empty node (data_type was null)
     * - Replace existing data with new data
     *
     * @param uid The node UID.
     * @param data The data to assign. Can be nullptr to clear the node's data.
     * @return true if successful, false if node not found.
     */
    bool SetNodeData(const std::string& uid, BaseData* data);

    // Property operations

    /**
     * @brief Get properties of a node with scope filtering.
     *
     * @param uid The node UID.
     * @param params Query parameters for property filtering.
     * @return JSON object with properties, or std::nullopt if node not found.
     */
    std::optional<Json> GetNodeProperties(const std::string& uid, const PropertyQueryParams& params = {}) const;

    /**
     * @brief Get a single property of a node.
     *
     * Returns the property in standard JSON serialization format:
     * - Simple properties: {"property_name": value}
     * - Complex properties: {"property_name": {"value": ..., "type": "..."}}
     *
     * @param uid The node UID.
     * @param key The property key.
     * @param params Query parameters for scope and context.
     * @return JSON object with the property, or std::nullopt if not found.
     */
    std::optional<Json> GetNodeProperty(
      const std::string& uid,
      const std::string& key,
      const PropertyQueryParams& params = {}) const;

    /**
     * @brief Set a single property on a node.
     *
     * @param uid The node UID.
     * @param key The property key.
     * @param value The property value as JSON.
     * @param params Query parameters for scope and context.
     * @return true if set successfully.
     */
    bool SetNodeProperty(
      const std::string& uid,
      const std::string& key,
      const Json& value,
      const PropertyQueryParams& params = {});

    /**
     * @brief Delete a property from a node.
     *
     * @param uid The node UID.
     * @param key The property key.
     * @param params Query parameters for scope and context.
     * @return true if deleted successfully.
     */
    bool DeleteNodeProperty(
      const std::string& uid,
      const std::string& key,
      const PropertyQueryParams& params = {});

    /**
     * @brief Replace all properties on a node (PUT semantics).
     *
     * @param uid The node UID.
     * @param properties JSON object with all properties (replaces existing).
     * @param params Query parameters for scope and context.
     * @return JSON with replaced/removed property names, or nullopt if node not found.
     */
    std::optional<Json> ReplaceNodeProperties(
      const std::string& uid,
      const Json& properties,
      const PropertyQueryParams& params = {});

    /**
     * @brief Get available property contexts for a node.
     *
     * Returns a JSON array containing null (for default context) and strings
     * for each named context available on the node.
     *
     * @param uid The node UID.
     * @return JSON array of contexts, or std::nullopt if node not found.
     */
    std::optional<Json> GetNodeAvailableContexts(const std::string& uid) const;

  private:
    /**
     * @brief Build a composite NodePredicate from query parameters.
     *
     * Uses MITK's NodePredicate system to create a reusable, composable filter.
     * Supports: hierarchy, path, data type, parent UID, and property filters.
     *
     * @param params Query parameters to convert to predicate.
     * @return Composite predicate that matches all filter criteria.
     */
    NodePredicateBase::Pointer BuildNodePredicate(const NodeQueryParams& params) const;

    Json NodeToJson(const DataNode* node) const;
    std::string BuildNodePath(const DataNode* node) const;
    int GetChildrenCount(const DataNode* node) const;

    WeakPointer<DataStorage> m_DataStorage;
    std::unique_ptr<NodeUidMapper> m_UidMapper;

    mutable std::mutex m_Mutex;
  };
}

#endif
