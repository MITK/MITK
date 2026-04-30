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
#include <mitkStorageThreadDispatcherBase.h>
#include <mitkNodeUidMapper.h>
#include <mitkNodeQueryParams.h>

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
   * \brief Thread-safe bridge between the REST API layer and MITK DataStorage.
   *
   * This class abstracts between the concrete REST/HTTP implementation
   * and MITK-specific handling of requests, translating between
   * DataStorage content and REST message content (as JSON).
   *
   * Provides:
   *   - Thread-safe CRUD operations on DataNodes
   *   - UID-based node identification via NodeUidMapper
   *   - JSON serialization of nodes and properties
   *
   * \section datastorage_bridge_internal Internal Properties
   * Properties with keys starting with "restapi." are internal metadata
   * used by the REST API (e.g., "restapi.uid" for node identification).
   * These properties are:
   *   - Filtered out from all property query responses (not leaked to clients)
   *   - Protected from modification via SetNodeProperty
   *   - Protected from deletion via DeleteNodeProperty and ReplaceNodeProperties
   *   - Clients cannot set, get, or delete these properties via the API
   *
   * \section datastorage_bridge_tracking Modification Tracking
   * When a node is modified through the bridge (create, update, property changes),
   * the bridge automatically adds two internal properties to the node:
   *   - "restapi.modified" (string): Timestamp of the last modification via REST API.
   *   - "restapi.lastmodification" (string): Description of the last modification operation.
   *
   * Possible lastmodification values:
   *   - "created" -- Node was created via CreateNode
   *   - "reparented" -- Node was moved to a different parent via UpdateNode
   *   - "property_set:\<key\>" -- A property was set via SetNodeProperty
   *   - "property_deleted:\<key\>" -- A property was deleted via DeleteNodeProperty
   *   - "properties_replaced" -- Properties were replaced via ReplaceNodeProperties
   *
   * All public methods are thread-safe.
   *
   * \sa NodeUidMapper, RestServer, IRestServerService
   */
  class MITKRESTAPI_EXPORT DataStorageBridge
  {
  public:
    using Json = nlohmann::json;

    /** \brief Prefix for internal REST API properties (filtered from responses, protected from modification). */
    static constexpr const char* INTERNAL_PROPERTY_PREFIX = "restapi.";

    /**
     * \brief Property key for tracking if a node was modified via the REST API.
     *
     * If this string property exists on a node, it was modified via the REST API
     * and its value is the timestamp of the last modification.
     */
    static constexpr const char* MODIFIED_PROPERTY_KEY = "restapi.modified";

    /** \brief Property key for tracking the last modification operation (string property). */
    static constexpr const char* LAST_MODIFICATION_PROPERTY_KEY = "restapi.lastmodification";

    DataStorageBridge();
    ~DataStorageBridge();

    // Non-copyable, non-movable
    DataStorageBridge(const DataStorageBridge&) = delete;
    DataStorageBridge& operator=(const DataStorageBridge&) = delete;
    DataStorageBridge(DataStorageBridge&&) = delete;
    DataStorageBridge& operator=(DataStorageBridge&&) = delete;

    /**
     * \brief Set the dispatcher for thread-safe DataStorage operations.
     *
     * If set, all public operations (reads and writes) are dispatched to the
     * storage-owning thread. This is necessary because even read operations
     * may set restapi.uid properties on first access, triggering Modified events
     * that cascade into Qt widget updates.
     *
     * If not set, operations execute directly on the calling thread
     * (suitable for tests/headless scenarios).
     *
     * \param[in] dispatcher The dispatcher, or \c nullptr to clear.
     */
    void SetDispatcher(StorageThreadDispatcherBase* dispatcher);

    /**
     * \brief Set the DataStorage to operate on.
     *
     * \param[in] dataStorage The DataStorage, or \c nullptr to disconnect.
     */
    void SetDataStorage(DataStorage* dataStorage);

    /**
     * \brief Get the current DataStorage.
     *
     * \return The DataStorage, or \c nullptr if not connected.
     */
    DataStorage::Pointer GetDataStorage() const;

    /**
     * \brief Status codes for mutation operations.
     *
     * Enables controllers to produce precise HTTP responses:
     * - Success           200/204
     * - NodeNotFound      404
     * - PropertyNotFound  404
     * - InvalidInput      400
     * - InternalError     500
     */
    enum class OperationStatus
    {
      Success,
      NodeNotFound,       ///< Target node does not exist           404
      PropertyNotFound,   ///< Property absent in target scope      404
      InvalidInput,       ///< Malformed value or unsupported scope 400
      InternalError       ///< No DataStorage or unexpected state   500
    };

    /**
     * \brief Check if a DataStorage is connected.
     *
     * \return true if a DataStorage is available.
     */
    bool HasDataStorage() const;

    /**
     * \brief Get the UID for a DataNode.
     *
     * Creates a UID if the node doesn't have one yet.
     *
     * \param[in] node The node to get the UID for. Must not be \c nullptr.
     * \return The UID string for this node.
     * \throw std::invalid_argument if \p node is \c nullptr.
     */
    std::string GetNodeUid (DataNode* node) const;

    // Node operations

    /**
     * \brief Result of a node query with pagination metadata.
     */
    struct NodeQueryResult
    {
      Json nodes;       ///< Array of node objects
      int totalCount;   ///< Total number of matching nodes (before pagination)
      int limit;        ///< Limit used in query
      int offset;       ///< Offset used in query
    };

    /**
     * \brief Get nodes with full query parameter support.
     *
     * \param params Query parameters including filters, pagination, etc.
     * \return Query result with nodes and metadata.
     */
    NodeQueryResult GetNodes(const NodeQueryParams& params) const;

    /**
     * \brief Get a single node by UID.
     *
     * \param uid The node UID.
     * \return JSON object for the node, or std::nullopt if not found.
     */
    std::optional<Json> GetNode(const std::string& uid) const;

    /**
     * \brief Result of a node creation operation.
     */
    struct CreateNodeResult
    {
      bool success;                               ///< Whether the node was created
      std::string uid;                            ///< UID of the created node (empty if failed)
      std::vector<std::string> failedProperties;  ///< Property keys that failed to deserialize
    };

    /**
     * \brief Create a new node.
     *
     * Creates a new DataNode with the specified name and properties.
     * Properties that fail to deserialize are logged and reported in the result,
     * but do not prevent node creation.
     *
     * \param nodeData JSON object with node data (name, properties).
     * \param parentUid Optional parent node UID. If provided, creates as child of parent.
     * \return CreateNodeResult with success status, UID, and any failed properties.
     */
    CreateNodeResult CreateNode(const Json& nodeData, const std::optional<std::string>& parentUid = std::nullopt);

    /**
     * \brief Update a node.
     *
     * Currently supports reparenting via "parent_uid" field.
     * Reparenting is done by removing the node from DataStorage and
     * re-adding it under the new parent (same approach as Data Manager UI).
     *
     * \param uid The node UID.
     * \param updates JSON object with fields to update (e.g., "parent_uid").
     * \return true if updated successfully.
     */
    bool UpdateNode(const std::string& uid, const Json& updates);

    /**
     * \brief Result of a node deletion operation.
     */
    struct DeleteResult
    {
      bool success;                         ///< Whether deletion succeeded
      std::string deletedUid;               ///< UID of the deleted node
      std::vector<std::string> deletedChildren;  ///< UIDs of deleted children (if recursive)
      int childrenCount;                    ///< Number of children (for error reporting)
    };

    /**
     * \brief Delete a node.
     *
     * \param uid The node UID.
     * \param recursive If true, also delete all children recursively.
     * \return DeleteResult with success status and deleted UIDs.
     */
    DeleteResult DeleteNode(const std::string& uid, bool recursive = false);

    // Data operations

    /**
     * \brief Result of a GetNodeData operation.
     */
    struct GetNodeDataResult
    {
      bool nodeFound;         ///< Whether the node with the given UID exists
      BaseData::Pointer data; ///< Clone of the data, or nullptr if node has no data
    };

    /**
     * \brief Get a clone of the node's data for thread-safe processing.
     *
     * Returns a clone of the BaseData attached to the node. The clone is
     * independent of the original data and can be safely processed (e.g.,
     * serialized) without holding locks and without affecting the original.
     *
     * This method is designed for scenarios where the caller needs to perform
     * potentially slow operations on the data (like serialization) without
     * blocking other DataStorage operations.
     *
     * \param uid The node UID.
     * \return GetNodeDataResult with nodeFound status and cloned data.
     *         - nodeFound=false: Node does not exist
     *         - nodeFound=true, data=nullptr: Node exists but has no data
     *         - nodeFound=true, data!=nullptr: Node exists and data is cloned
     */
    GetNodeDataResult GetNodeData(const std::string& uid) const;

    /**
     * \brief Look up a DataNode by UID.
     *
     * Returns a const strong reference to the node, keeping it alive for
     * the duration of the caller's use. Returns null if the UID is not registered.
     *
     * Intended for read-only access (e.g. inspecting geometry, properties).
     * Do not retain the returned pointer beyond the immediate call site without
     * understanding the threading implications.
     *
     * \param uid The node UID.
     * \return DataNode::ConstPointer to the node, or null if not found.
     */
    DataNode::ConstPointer FindDataNode(const std::string& uid) const;

    /**
     * \brief Set or replace the data on a node.
     *
     * Thread-safe assignment of BaseData to a node. The data is assigned
     * directly (not cloned), so the caller should not modify the data object
     * after calling this method.
     *
     * Can be used to:
     * - Set data on an empty node (data_type was null)
     * - Replace existing data with new data
     *
     * \param uid The node UID.
     * \param data The data to assign. Can be nullptr to clear the node's data.
     * \return OperationStatus::Success, ::NodeNotFound, or ::InternalError.
     */
    OperationStatus SetNodeData(const std::string& uid, BaseData* data);

    // Property operations

    /**
     * \brief Get properties of a node with scope filtering.
     *
     * \param uid The node UID.
     * \param params Query parameters for property filtering.
     * \return JSON object with properties, or std::nullopt if node not found.
     */
    std::optional<Json> GetNodeProperties(const std::string& uid, const PropertyQueryParams& params = {}) const;

    /**
     * \brief Get a single property of a node.
     *
     * Returns the property in standard JSON serialization format:
     * - Simple properties: {"property_name": value}
     * - Complex properties: {"property_name": {"value": ..., "type": "..."}}
     *
     * \param uid The node UID.
     * \param key The property key.
     * \param params Query parameters for scope and context.
     * \return JSON object with the property, or std::nullopt if not found.
     */
    std::optional<Json> GetNodeProperty(
      const std::string& uid,
      const std::string& key,
      const PropertyQueryParams& params = {}) const;

    /**
     * \brief Set a single property on a node.
     *
     * \param uid The node UID.
     * \param key The property key.
     * \param value The property value as JSON.
     * \param params Query parameters for scope and context.
     * \return OperationStatus::Success, ::NodeNotFound, ::InvalidInput, or ::InternalError.
     */
    OperationStatus SetNodeProperty(
      const std::string& uid,
      const std::string& key,
      const Json& value,
      const PropertyQueryParams& params);

    /**
     * \brief Delete a property from a node.
     *
     * \param uid The node UID.
     * \param key The property key.
     * \param params Query parameters for scope and context.
     * \return OperationStatus::Success, ::NodeNotFound, ::PropertyNotFound, ::InvalidInput, or ::InternalError.
     */
    OperationStatus DeleteNodeProperty(
      const std::string& uid,
      const std::string& key,
      const PropertyQueryParams& params);

    /**
     * \brief Result of a ReplaceNodeProperties operation.
     */
    struct ReplacePropertiesResult
    {
      OperationStatus status = OperationStatus::InternalError;
      Json result = {};  ///< {"replaced": [...], "removed": [...]} - valid only on Success
    };

    /**
     * \brief Replace all properties on a node (PUT semantics).
     *
     * \param uid The node UID.
     * \param properties JSON object with all properties (replaces existing).
     * \param params Query parameters for scope and context.
     * \return ReplacePropertiesResult with status and, on success, the replaced/removed lists.
     */
    ReplacePropertiesResult ReplaceNodeProperties(
      const std::string& uid,
      const Json& properties,
      const PropertyQueryParams& params);

    /**
     * \brief Get available property contexts for a node.
     *
     * Returns a JSON array containing null (for default context) and strings
     * for each named context available on the node.
     *
     * \param uid The node UID.
     * \return JSON array of contexts, or std::nullopt if node not found.
     */
    std::optional<Json> GetNodeAvailableContexts(const std::string& uid) const;

  private:
    /**
     * \brief Build a composite NodePredicate from query parameters.
     *
     * Uses MITK's NodePredicate system to create a reusable, composable filter.
     * Supports: hierarchy, path, data type, parent UID, and property filters.
     *
     * \param params Query parameters to convert to predicate.
     * \return Composite predicate that matches all filter criteria.
     */
    NodePredicateBase::Pointer BuildNodePredicate(const NodeQueryParams& params) const;

    Json NodeToJson(DataNode* node) const;
    std::string BuildNodePath(const DataNode* node) const;
    int GetChildrenCount(const DataNode* node) const;

    /**
     * \brief Execute a task via dispatcher (if available) or directly.
     *
     * Handles IsDispatchThread() check internally to avoid deadlocks.
     * Use for void-returning tasks.
     */
    void DispatchTask(std::function<void()> task) const;

    /**
     * \brief Execute a task via dispatcher and return a value.
     *
     * Handles IsDispatchThread() check internally to avoid deadlocks.
     * Use for tasks that return a value.
     */
    template <typename R>
    R DispatchTask(std::function<R()> task) const;

    WeakPointer<DataStorage> m_DataStorage;
    WeakPointer<StorageThreadDispatcherBase> m_Dispatcher;
    std::unique_ptr<NodeUidMapper> m_UidMapper;

    /** Remark: Important to ensure that this mutex is always acquired before interacting with m_UidMapper
     * (and its mutex).*/
    mutable std::mutex m_Mutex;
  };
}

#endif
