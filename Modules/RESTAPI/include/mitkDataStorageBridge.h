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
   * - "restapi.modified" (bool): Set to true when the node is modified
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

    /**
     * @brief Check if a property value matches a filter pattern.
     *
     * Supports:
     * - Exact match: "CT_Scan"
     * - Prefix wildcard: "CT*"
     * - Suffix wildcard: "*Scan"
     *
     * @param propertyValue The actual property value, or std::nullopt if property doesn't exist.
     * @param filterValue The filter pattern.
     * @return true if the value matches the pattern.
     */
    bool MatchesPropertyFilter(
      const std::optional<std::string>& propertyValue,
      const std::string& filterValue) const;

    /**
     * @brief Check if a property key is internal (starts with INTERNAL_PROPERTY_PREFIX).
     *
     * Internal properties are REST API metadata and should not be exposed to clients.
     *
     * @param key The property key to check.
     * @return true if the property is internal.
     */
    static bool IsInternalProperty(const std::string& key);

    /**
     * @brief Mark a node as modified via REST API.
     *
     * Sets the internal properties "restapi.modified" to true and
     * "restapi.lastmodification" to the specified operation description.
     *
     * @param node The node to mark as modified. Must not be nullptr.
     * @param operation Description of the modification operation (e.g., "created", "property_set:name").
     */
    static void MarkNodeAsModified(DataNode* node, const std::string& operation);

    DataStorage::Pointer m_DataStorage;
    std::unique_ptr<NodeUidMapper> m_UidMapper;

    mutable std::mutex m_Mutex;
  };
}

#endif
