/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDataStorageBridge.h"
#include <mitkStringProperty.h>
#include <mitkProperties.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateFunction.h>
#include <mitkLog.h>

#include <algorithm>
#include <sstream>


namespace
{
  /**
   * @brief Convert a JSON value to its string representation for filtering.
   *
   * This ensures filtering uses the same representation users see in the REST API:
   * - JSON boolean true -> "true" (not "1" as GetValueAsString() returns)
   * - JSON boolean false -> "false" (not "0" as GetValueAsString() returns)
   * - JSON number -> string representation
   * - JSON string -> the string value
   * - JSON null -> empty string
   * - JSON object/array -> dump() representation
   */
  std::string JsonValueToFilterString(const nlohmann::json& json)
  {
    if (json.is_boolean())
    {
      return json.get<bool>() ? "true" : "false";
    }
    else if (json.is_string())
    {
      return json.get<std::string>();
    }
    else if (json.is_number_integer())
    {
      return std::to_string(json.get<int>());
    }
    else if (json.is_number_float())
    {
      // Use stringstream for consistent float formatting
      std::ostringstream ss;
      ss << json.get<double>();
      return ss.str();
    }
    else if (json.is_null())
    {
      return "";
    }
    else
    {
      // For complex types (arrays, objects), use the JSON dump
      return json.dump();
    }
  }

  mitk::BaseProperty::ConstPointer GetConstProperty(const mitk::DataNode* node, const std::string& propertyKey,
    const std::optional<std::string>& context, const mitk::PropertyScope scope)
  {
    if (nullptr == node)
      mitkThrow() << "Wrong use of GetConstProperty. Node is nullptr.";

    std::string contextName = context.has_value() ? context.value() : "";

    mitk::BaseProperty::ConstPointer allResult = node->GetConstProperty(propertyKey, contextName, false);

    if (scope == mitk::PropertyScope::All)
    {
      return allResult;
    }

    // scope is not all, we have to check explicit because the node implementation has a automatic
    // fall trough...
    mitk::BaseProperty::ConstPointer dataResult;
    if (nullptr != node->GetData())
    {
      dataResult = node->GetData()->GetConstProperty(propertyKey, contextName, false);
    }

    if (scope == mitk::PropertyScope::Data)
    {
      return dataResult;
    }

    // scope is Node only. If dataResult and allResult are different pointer,
    // then there is a property defined at node level.
    if (allResult != dataResult)
    {
      return allResult;
    }

    return nullptr;
  }

  mitk::BaseProperty::Pointer GetProperty(mitk::DataNode* node, const std::string& propertyKey,
    const std::optional<std::string>& context, const mitk::PropertyScope scope)
  {
    if (nullptr == node)
      mitkThrow() << "Wrong use of GetProperty. Node is nullptr.";

    std::string contextName = context.has_value() ? context.value() : "";

    mitk::BaseProperty::Pointer allResult = node->GetNonConstProperty(propertyKey, contextName, false);

    if (scope == mitk::PropertyScope::All)
    {
      return allResult;
    }

    // scope is not all, we have to check explicit because the node implementation has a automatic
    // fall trough...
    mitk::BaseProperty::Pointer dataResult;
    if (nullptr != node->GetData())
    {
      dataResult = node->GetData()->GetNonConstProperty(propertyKey, contextName, false);
    }

    if (scope == mitk::PropertyScope::Data)
    {
      return dataResult;
    }

    // scope is Node only. If dataResult and allResult are different pointer,
    // then there is a property defined at node level.
    if (allResult != dataResult)
    {
      return allResult;
    }

    return nullptr;
  }
}


namespace mitk
{

  DataStorageBridge::DataStorageBridge()
    : m_UidMapper(std::make_unique<NodeUidMapper>())
  {
  }

  DataStorageBridge::~DataStorageBridge() = default;

  void DataStorageBridge::SetDataStorage(DataStorage* dataStorage)
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_DataStorage = dataStorage;
    m_UidMapper->SetDataStorage(dataStorage);
  }

  DataStorage::Pointer DataStorageBridge::GetDataStorage() const
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_DataStorage;
  }

  bool DataStorageBridge::HasDataStorage() const
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_DataStorage.IsNotNull();
  }

  std::string DataStorageBridge::GetNodeUid(const DataNode* node) const
  {
    return m_UidMapper->GetOrCreateUid(node);
  }

  bool DataStorageBridge::IsInternalProperty(const std::string& key)
  {
    // Check if key starts with INTERNAL_PROPERTY_PREFIX ("restapi.")
    return key.rfind(INTERNAL_PROPERTY_PREFIX, 0) == 0;
  }

  void DataStorageBridge::MarkNodeAsModified(DataNode* node, const std::string& operation)
  {
    if (node == nullptr)
    {
      return;
    }

    node->SetBoolProperty(MODIFIED_PROPERTY_KEY, true);
    node->SetStringProperty(LAST_MODIFICATION_PROPERTY_KEY, operation.c_str());
  }

  bool DataStorageBridge::MatchesPropertyFilter(
    const std::optional<std::string>& propertyValue,
    const std::string& filterValue) const
  {
    // If property doesn't exist, it doesn't match
    if (!propertyValue.has_value())
    {
      return false;
    }

    const std::string& value = propertyValue.value();

    // Check for wildcard patterns per API spec:
    // - filter.name=CT* (prefix match)
    // - filter.name=*Scan (suffix match)

    if (filterValue.length() >= 2)
    {
      // Prefix match: CT*
      if (filterValue.back() == '*' && filterValue.front() != '*')
      {
        std::string prefix = filterValue.substr(0, filterValue.length() - 1);
        return value.compare(0, prefix.length(), prefix) == 0;
      }

      // Suffix match: *Scan
      if (filterValue.front() == '*' && filterValue.back() != '*')
      {
        std::string suffix = filterValue.substr(1);
        if (value.length() >= suffix.length())
        {
          return value.compare(
            value.length() - suffix.length(),
            suffix.length(),
            suffix) == 0;
        }
        return false;
      }
    }

    // Exact match
    return value == filterValue;
  }

  NodePredicateBase::Pointer DataStorageBridge::BuildNodePredicate(const NodeQueryParams& params) const
  {
    std::vector<NodePredicateBase::Pointer> predicates;

    // Hierarchy filter (toplevel = root nodes only)
    if (params.hierarchy == Hierarchy::Toplevel)
    {
      auto isRootPredicate = NodePredicateFunction::New(
        [this](const DataNode* node) -> bool {
          auto sources = m_DataStorage->GetSources(node);
          return sources->Size() == 0;
        });
      predicates.push_back(isRootPredicate.GetPointer());
    }

    // Path filter
    if (params.path.has_value())
    {
      const std::string targetPath = params.path.value();
      auto pathPredicate = NodePredicateFunction::New(
        [this, targetPath](const DataNode* node) -> bool {
          return this->BuildNodePath(node) == targetPath;
        });
      predicates.push_back(pathPredicate.GetPointer());
    }

    // Data type filter
    if (params.dataType.has_value())
    {
      std::string dataType = params.dataType.value();
      // Remove "mitk::" prefix if present for NodePredicateDataType
      if (dataType.rfind("mitk::", 0) == 0)
      {
        dataType = dataType.substr(6);
      }
      auto dataTypePredicate = NodePredicateDataType::New(dataType.c_str());
      predicates.push_back(dataTypePredicate.GetPointer());
    }

    // Parent UID filter
    if (params.parentUid.has_value())
    {
      const std::string targetParentUid = params.parentUid.value();
      auto parentPredicate = NodePredicateFunction::New(
        [this, targetParentUid](const DataNode* node) -> bool {
          auto sources = m_DataStorage->GetSources(node);

          if (targetParentUid == "null")
          {
            // Filter for root nodes (no parent)
            return sources->Size() == 0;
          }
          else
          {
            // Check if parent matches the target UID
            for (auto srcIt = sources->Begin(); srcIt != sources->End(); ++srcIt)
            {
              auto parentUid = m_UidMapper->GetUid(srcIt->Value().GetPointer());
              if (parentUid.has_value() && parentUid.value() == targetParentUid)
              {
                return true;
              }
            }
            return false;
          }
        });
      predicates.push_back(parentPredicate.GetPointer());
    }

    // Property filters
    for (const auto& propFilter : params.propertyFilters)
    {
      auto propertyPredicate = NodePredicateFunction::New(
        [this, params, propFilter](const DataNode* node) -> bool {
          std::optional<std::string> propValue;

          // Handle special fields that aren't stored as properties
          if (propFilter.key == "name")
          {
            propValue = node->GetName();
          }
          else
          {
            auto prop = GetConstProperty(node, propFilter.key, params.context, params.propertyScope);
            if (prop != nullptr)
            {
              // Use JSON representation for filtering to match what users see in API responses.
              // This ensures BoolProperty filters with "true"/"false" (not "1"/"0"),
              // making the filter behavior consistent with the REST API output.
              auto jsonValue = ConvertPropertyToSelfContainedJson(prop);
              propValue = JsonValueToFilterString(jsonValue);
            }
          }

          bool matches = this->MatchesPropertyFilter(propValue, propFilter.value);

          // Apply negation if needed (for != operator)
          return propFilter.negated ? !matches : matches;
        });
      predicates.push_back(propertyPredicate.GetPointer());
    }

    // Combine all predicates with AND
    if (predicates.empty())
    {
      // No filters - return a predicate that matches everything
      return NodePredicateFunction::New([](const DataNode*) { return true; });
    }
    else if (predicates.size() == 1)
    {
      return predicates[0];
    }

    // Build a chain of AND predicates
    auto combined = NodePredicateAnd::New();
    for (auto predicate : predicates)
    {
      combined->AddPredicate(predicate);
    }
    return combined.GetPointer();
  }

  DataStorageBridge::NodeQueryResult DataStorageBridge::GetNodes(const NodeQueryParams& params) const
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    NodeQueryResult result;
    result.nodes = Json::array();
    result.totalCount = 0;
    result.limit = params.limit;
    result.offset = params.offset;

    if (m_DataStorage.IsNull())
    {
      return result;
    }

    // Build predicate from query parameters
    auto predicate = this->BuildNodePredicate(params);

    // Get filtered nodes using the predicate
    auto filteredNodes = m_DataStorage->GetSubset(predicate);

    // Convert to vector for sorting and pagination
    std::vector<DataNode*> matchingNodes;
    matchingNodes.reserve(filteredNodes->Size());
    for (auto it = filteredNodes->Begin(); it != filteredNodes->End(); ++it)
    {
      matchingNodes.push_back(it->Value().GetPointer());
    }

    // Apply sorting if specified
    // Sorting is done by JSON result fields: uid, name, path, parent_uid, data_type, children_count, timestamp
    if (params.sort.has_value())
    {
      const auto& sortSpec = params.sort.value();
      std::sort(matchingNodes.begin(), matchingNodes.end(),
        [this, &sortSpec](DataNode* a, DataNode* b) {
          const std::string& field = sortSpec.field;

          // Handle numeric fields (children_count, timestamp)
          if (field == "children_count")
          {
            int countA = this->GetChildrenCount(a);
            int countB = this->GetChildrenCount(b);
            return sortSpec.ascending ? (countA < countB) : (countA > countB);
          }
          else if (field == "timestamp")
          {
            unsigned long timeA = a->GetMTime();
            unsigned long timeB = b->GetMTime();
            return sortSpec.ascending ? (timeA < timeB) : (timeA > timeB);
          }

          // Handle string fields
          std::string valA, valB;

          if (field == "uid")
          {
            valA = m_UidMapper->GetOrCreateUid(a);
            valB = m_UidMapper->GetOrCreateUid(b);
          }
          else if (field == "name")
          {
            valA = a->GetName();
            valB = b->GetName();
          }
          else if (field == "path")
          {
            valA = this->BuildNodePath(a);
            valB = this->BuildNodePath(b);
          }
          else if (field == "parent_uid")
          {
            auto sourcesA = m_DataStorage->GetSources(a);
            auto sourcesB = m_DataStorage->GetSources(b);
            valA = (sourcesA->Size() > 0) ? m_UidMapper->GetOrCreateUid(sourcesA->ElementAt(0)) : "";
            valB = (sourcesB->Size() > 0) ? m_UidMapper->GetOrCreateUid(sourcesB->ElementAt(0)) : "";
          }
          else if (field == "data_type")
          {
            auto dataA = a->GetData();
            auto dataB = b->GetData();
            valA = dataA ? (std::string("mitk::") + dataA->GetNameOfClass()) : "";
            valB = dataB ? (std::string("mitk::") + dataB->GetNameOfClass()) : "";
          }
          else
          {
            // Unknown field - no sorting
            return false;
          }

          return sortSpec.ascending ? (valA < valB) : (valA > valB);
        });
    }

    // Store total count before pagination
    result.totalCount = static_cast<int>(matchingNodes.size());

    // Apply pagination
    int startIdx = std::min(params.offset, static_cast<int>(matchingNodes.size()));
    int endIdx = std::min(params.offset + params.limit, static_cast<int>(matchingNodes.size()));

    // Build result with optional field selection
    for (int i = startIdx; i < endIdx; ++i)
    {
      Json nodeJson = this->NodeToJson(matchingNodes[i]);

      if (!params.fields.empty())
      {
        Json filtered = Json::object();
        for (const auto& field : params.fields)
        {
          if (nodeJson.contains(field))
          {
            filtered[field] = nodeJson[field];
          }
        }
        result.nodes.push_back(filtered);
      }
      else
      {
        result.nodes.push_back(nodeJson);
      }
    }

    return result;
  }

  std::optional<DataStorageBridge::Json> DataStorageBridge::GetNode(const std::string& uid) const
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_DataStorage.IsNull())
    {
      return std::nullopt;
    }

    auto node = m_UidMapper->FindNodeByUid(uid);
    if (node == nullptr)
    {
      return std::nullopt;
    }

    return this->NodeToJson(node);
  }

  DataStorageBridge::CreateNodeResult DataStorageBridge::CreateNode(const Json& nodeData, const std::optional<std::string>& parentUid)
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    CreateNodeResult result;
    result.success = false;

    if (m_DataStorage.IsNull())
    {
      return result;
    }

    // Find parent node if specified
    DataNode* parentNode = nullptr;
    if (parentUid.has_value())
    {
      parentNode = m_UidMapper->FindNodeByUid(parentUid.value());
      if (parentNode == nullptr)
      {
        return result;  // Parent not found
      }
    }

    auto node = DataNode::New();

    // Set name if provided
    if (nodeData.contains("name") && nodeData["name"].is_string())
    {
      node->SetName(nodeData["name"].get<std::string>());
    }

    // Apply properties if provided
    if (nodeData.contains("properties") && nodeData["properties"].is_object())
    {
      for (auto& [key, value] : nodeData["properties"].items())
      {
        try
        {
          auto prop = ConvertPropertyFromSelfContainedJson(value);
          if (prop.IsNotNull())
          {
            node->SetProperty(key, prop);
          }
          else
          {
            MITK_WARN << "REST API: Failed to deserialize property '" << key << "' - null result from deserialization";
            result.failedProperties.push_back(key);
          }
        }
        catch (const std::exception& e)
        {
          MITK_WARN << "REST API: Failed to deserialize property '" << key << "': " << e.what();
          result.failedProperties.push_back(key);
        }
      }
    }

    // Add to DataStorage (with or without parent)
    if (parentNode != nullptr)
    {
      m_DataStorage->Add(node, parentNode);
    }
    else
    {
      m_DataStorage->Add(node);
    }

    // Get/create UID for the new node
    result.uid = m_UidMapper->GetOrCreateUid(node);
    result.success = true;

    // Mark node as modified via REST API
    MarkNodeAsModified(node, "created");

    return result;
  }

  bool DataStorageBridge::UpdateNode(const std::string& uid, const Json& updates)
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_DataStorage.IsNull())
    {
      return false;
    }

    mitk::DataNode::Pointer node = m_UidMapper->FindNodeByUid(uid);
    if (node == nullptr)
    {
      return false;
    }

    // Handle parent_uid for reparenting
    if (!updates.contains("parent_uid"))
    {
      return false;
    }

    DataNode* newParent = nullptr;
    if (!updates["parent_uid"].is_null())
    {
      std::string newParentUid = updates["parent_uid"].get<std::string>();
      newParent = m_UidMapper->FindNodeByUid(newParentUid);
    }

    if (newParent == node)
    { // Prevent circular references: new parent cannot be the node itself or a descendant
      return false;
    }

    // Check if newParent is a descendant of node
    auto descendants = m_DataStorage->GetDerivations(node, nullptr, true);
    for (auto it = descendants->Begin(); it != descendants->End(); ++it)
    {
      if (it->Value().GetPointer() == newParent)
      {
        return false;  // Would create circular reference
      }
    }

    // Reparent: remove and re-add under new parent
    // This is the same approach used by QmitkDataStorageTreeModel
    //
    // Important: Preserve the UID across reparenting operation.
    // The NodeUidMapper clears mappings when a node is removed from DataStorage,
    // so we save the UID and restore the mapping after re-adding.
    const std::string preservedUid = uid;

    m_DataStorage->Remove(node);
    if (newParent != nullptr)
    {
      m_DataStorage->Add(node, newParent);
    }
    else
    {
      m_DataStorage->Add(node);  // Move to root level
    }

    m_UidMapper->RestoreUid(node, preservedUid);

    MarkNodeAsModified(node, "reparented");

    return true;
  }

  DataStorageBridge::DeleteResult DataStorageBridge::DeleteNode(const std::string& uid, bool recursive)
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    DeleteResult result;
    result.success = false;
    result.deletedUid = uid;
    result.childrenCount = 0;

    if (m_DataStorage.IsNull())
    {
      return result;
    }

    auto node = m_UidMapper->FindNodeByUid(uid);
    if (node == nullptr)
    {
      return result;
    }

    // Check for children
    auto derivatives = m_DataStorage->GetDerivations(node);
    result.childrenCount = static_cast<int>(derivatives->Size());

    if (result.childrenCount > 0 && !recursive)
    {
      // Has children but recursive not requested
      return result;
    }

    // If recursive, collect and delete all descendants first
    if (recursive && result.childrenCount > 0)
    {
      // Collect all descendants (depth-first)
      std::vector<DataNode*> toDelete;
      std::function<void(DataNode*)> collectDescendants = [&](DataNode* n) {
        auto children = m_DataStorage->GetDerivations(n);
        for (auto it = children->Begin(); it != children->End(); ++it)
        {
          collectDescendants(it->Value().GetPointer());
        }
        toDelete.push_back(n);
      };

      // Collect children (not the node itself yet)
      for (auto it = derivatives->Begin(); it != derivatives->End(); ++it)
      {
        collectDescendants(it->Value().GetPointer());
      }

      // Delete all descendants (children first, then grandchildren, etc.)
      for (auto descendant : toDelete)
      {
        // Use GetOrCreateUid to ensure all deleted children have UIDs for reporting
        result.deletedChildren.push_back(m_UidMapper->GetOrCreateUid(descendant));
        m_DataStorage->Remove(descendant);
      }
    }

    // Delete the node itself
    m_DataStorage->Remove(node);
    result.success = true;

    return result;
  }

  DataStorageBridge::GetNodeDataResult DataStorageBridge::GetNodeData(const std::string& uid) const
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    GetNodeDataResult result;
    result.nodeFound = false;
    result.data = nullptr;

    if (m_DataStorage.IsNull())
    {
      return result;
    }

    auto node = m_UidMapper->FindNodeByUid(uid);
    if (node == nullptr)
    {
      return result;  // nodeFound = false
    }

    result.nodeFound = true;

    auto data = node->GetData();
    if (data == nullptr)
    {
      return result;  // nodeFound = true, data = nullptr
    }

    // Clone the data for thread-safe processing outside the lock
    // This allows the caller to serialize/process the data without
    // blocking other DataStorage operations
    result.data = dynamic_cast<BaseData*>(data->Clone().GetPointer());
    return result;
  }

  bool DataStorageBridge::SetNodeData(const std::string& uid, BaseData* data)
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_DataStorage.IsNull())
    {
      return false;
    }

    auto node = m_UidMapper->FindNodeByUid(uid);
    if (node == nullptr)
    {
      return false;
    }

    node->SetData(data);

    // Mark node as modified via REST API
    MarkNodeAsModified(node, "data_set");

    return true;
  }

  std::optional<DataStorageBridge::Json> DataStorageBridge::GetNodeProperties(
    const std::string& uid,
    const PropertyQueryParams& params) const
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_DataStorage.IsNull())
    {
      return std::nullopt;
    }

    auto node = m_UidMapper->FindNodeByUid(uid);
    if (node == nullptr)
    {
      return std::nullopt;
    }

    std::string contextName = params.context.has_value() ? params.context.value() : "";

    // Get properties based on scope
    // Note: Data properties require BaseData access which may not always be available
    PropertyList* propertyList = nullptr;

    if (params.scope == PropertyScope::Node || params.scope == PropertyScope::All)
    {
      propertyList = node->GetPropertyList(contextName);
    }

    if (params.scope == PropertyScope::Data || (propertyList == nullptr && params.scope == PropertyScope::All))
    {
      auto data = node->GetData();
      if (data != nullptr)
      {
        propertyList = data->GetPropertyList();
      }
    }

    if (propertyList == nullptr)
    {
      return Json::object();
    }

    // Check if we should return content or just names
    if (!params.includeContent)
    {
      // Return just property names as array
      Json names = Json::array();
      auto propMap = propertyList->GetMap();
      for (auto it = propMap->begin(); it != propMap->end(); ++it)
      {
        // Skip internal properties (restapi.*)
        if (IsInternalProperty(it->first))
        {
          continue;
        }

        // Filter by names if specified
        if (params.names.empty() ||
            std::find(params.names.begin(), params.names.end(), it->first) != params.names.end())
        {
          names.push_back(it->first);
        }
      }
      return names;
    }

    Json result = ConvertPropertyListToSelfContainedJson(propertyList);

    // Remove internal properties (restapi.*)
    for (auto it = result.begin(); it != result.end(); )
    {
      if (IsInternalProperty(it.key()))
      {
        it = result.erase(it);
      }
      else
      {
        ++it;
      }
    }

    // Filter by names if specified
    if (!params.names.empty())
    {
      Json filtered = Json::object();
      for (const auto& name : params.names)
      {
        if (result.contains(name))
        {
          filtered[name] = result[name];
        }
      }
      return filtered;
    }

    return result;
  }

  std::optional<DataStorageBridge::Json> DataStorageBridge::GetNodeProperty(
    const std::string& uid,
    const std::string& key,
    const PropertyQueryParams& params) const
  {
    // Reject access to internal properties
    if (IsInternalProperty(key))
    {
      return std::nullopt;
    }

    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_DataStorage.IsNull())
    {
      return std::nullopt;
    }

    auto node = m_UidMapper->FindNodeByUid(uid);
    if (node == nullptr)
    {
      return std::nullopt;
    }

    auto prop = GetConstProperty(node, key, params.context, params.scope);

    if (prop == nullptr)
    {
      return std::nullopt;
    }

    Json result = Json::object();
    result[key] = ConvertPropertyToSelfContainedJson(prop);

    return result;
  }

  bool DataStorageBridge::SetNodeProperty(
    const std::string& uid,
    const std::string& key,
    const Json& value,
    const PropertyQueryParams& params)
  {
    // Reject modification of internal properties
    if (IsInternalProperty(key))
    {
      return false;
    }

    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_DataStorage.IsNull())
    {
      return false;
    }

    auto node = m_UidMapper->FindNodeByUid(uid);
    if (node == nullptr)
    {
      return false;
    }

    try
    {
      // Handle the case the also simple value forms are wrapped in {"value": ...}
      Json propValue = value;
      if (value.is_object() && value.contains("value") && !value.contains("type"))
      {
         propValue = value["value"];
      }
      auto prop = ConvertPropertyFromSelfContainedJson(propValue);
      if (prop.IsNotNull())
      {
        std::string contextName = params.context.has_value() ? params.context.value() : "";

        if (params.scope == PropertyScope::Data)
        {
          auto data = node->GetData();
          if (data != nullptr)
          {
            data->SetProperty(key, prop, contextName);
            // Mark node as modified via REST API
            MarkNodeAsModified(node, "property_set:" + key);
            return true;
          }
          return false;
        }
        else
        {
          // Default to node scope (or "all" which defaults to node)
          node->SetProperty(key, prop, contextName);
          // Mark node as modified via REST API
          MarkNodeAsModified(node, "property_set:" + key);
          return true;
        }
      }
    }
    catch (const std::exception&)
    {
      return false;
    }

    return false;
  }

  bool DataStorageBridge::DeleteNodeProperty(
    const std::string& uid,
    const std::string& key,
    const PropertyQueryParams& params)
  {
    // Reject deletion of internal properties
    if (IsInternalProperty(key))
    {
      return false;
    }

    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_DataStorage.IsNull())
    {
      return false;
    }

    auto node = m_UidMapper->FindNodeByUid(uid);
    if (node == nullptr)
    {
      return false;
    }

    std::string contextName = params.context.has_value() ? params.context.value() : "";
    PropertyList* propertyList = nullptr;

    if (params.scope == PropertyScope::Data)
    {
      auto data = node->GetData();
      if (data != nullptr)
      {
        propertyList = data->GetPropertyList();
      }
    }
    else
    {
      // Default to node scope (or "all" which defaults to node)
      propertyList = node->GetPropertyList(contextName);
    }

    if (propertyList == nullptr)
    {
      return false;
    }

    // Check if property exists
    if (propertyList->GetProperty(key) == nullptr)
    {
      return false;
    }

    propertyList->DeleteProperty(key);

    // Mark node as modified via REST API
    MarkNodeAsModified(node, "property_deleted:" + key);

    return true;
  }

  std::optional<DataStorageBridge::Json> DataStorageBridge::ReplaceNodeProperties(
    const std::string& uid,
    const Json& properties,
    const PropertyQueryParams& params)
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_DataStorage.IsNull())
    {
      return std::nullopt;
    }

    auto node = m_UidMapper->FindNodeByUid(uid);
    if (node == nullptr)
    {
      return std::nullopt;
    }

    if (!properties.is_object())
    {
      return std::nullopt;
    }

    std::string contextName = params.context.has_value() ? params.context.value() : "";
    PropertyList* propertyList = nullptr;

    if (params.scope == PropertyScope::Data)
    {
      auto data = node->GetData();
      if (data != nullptr)
      {
        propertyList = data->GetPropertyList();
      }
    }
    else
    {
      propertyList = node->GetPropertyList(contextName);
    }

    if (propertyList == nullptr)
    {
      return std::nullopt;
    }

    // Collect existing property names
    std::vector<std::string> existingNames;
    auto propMap = propertyList->GetMap();
    for (auto it = propMap->begin(); it != propMap->end(); ++it)
    {
      existingNames.push_back(it->first);
    }

    // Track what was replaced vs removed
    std::vector<std::string> replaced;
    std::vector<std::string> removed;

    // Set new properties (skip internal properties - clients cannot set them)
    for (auto& [key, value] : properties.items())
    {
      // Skip internal properties
      if (IsInternalProperty(key))
      {
        continue;
      }

      try
      {
        auto prop = ConvertPropertyFromSelfContainedJson(value);
        if (prop.IsNotNull())
        {
          propertyList->SetProperty(key, prop);
          replaced.push_back(key);
        }
      }
      catch (const std::exception&)
      {
        // Skip invalid properties
      }
    }

    // Remove properties that weren't in the new set
    // (but protect system properties like "name" and internal "restapi.*" properties)
    for (const auto& existingName : existingNames)
    {
      if (!properties.contains(existingName))
      {
        // Don't remove "name" property (it's protected)
        if (existingName == "name" && params.scope != PropertyScope::Data)
        {
          continue;
        }
        // Don't remove internal properties (restapi.*)
        if (IsInternalProperty(existingName))
        {
          continue;
        }
        propertyList->DeleteProperty(existingName);
        removed.push_back(existingName);
      }
    }

    // Mark node as modified via REST API
    MarkNodeAsModified(node, "properties_replaced");

    Json result;
    result["replaced"] = replaced;
    result["removed"] = removed;

    return result;
  }

  std::optional<DataStorageBridge::Json> DataStorageBridge::GetNodeAvailableContexts(const std::string& uid) const
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_DataStorage.IsNull())
    {
      return std::nullopt;
    }

    auto node = m_UidMapper->FindNodeByUid(uid);
    if (node == nullptr)
    {
      return std::nullopt;
    }

    Json contexts = Json::array();

    // Add null for default context (always available)
    contexts.push_back(nullptr);

    // Add named contexts from the node
    auto contextNames = node->GetPropertyContextNames();
    for (const auto& contextName : contextNames)
    {
      contexts.push_back(contextName);
    }

    return contexts;
  }

  DataStorageBridge::Json DataStorageBridge::NodeToJson(const DataNode* node) const
  {
    // Note: caller must hold m_Mutex

    Json result;

    // Get or create UID (const_cast needed because GetOrCreateUid may modify node property)
    result["uid"] = m_UidMapper->GetOrCreateUid(node);
    result["name"] = node->GetName();
    result["path"] = this->BuildNodePath(node);

    // Get parent UID if exists
    auto sources = m_DataStorage->GetSources(node);
    if (sources->Size() > 0)
    {
      auto parent = sources->ElementAt(0).GetPointer();
      result["parent_uid"] = m_UidMapper->GetOrCreateUid(parent);
    }
    else
    {
      result["parent_uid"] = nullptr;
    }

    // Get data type
    auto data = node->GetData();
    if (data != nullptr)
    {
      result["data_type"] = std::string("mitk::") + data->GetNameOfClass();
    }
    else
    {
      result["data_type"] = nullptr;
    }

    // Children count
    result["children_count"] = this->GetChildrenCount(node);

    // Timestamp: ITK modified time (ever-increasing integer, session-only, higher is newer)
    result["timestamp"] = static_cast<unsigned long>(node->GetMTime());

    return result;
  }

  std::string DataStorageBridge::BuildNodePath(const DataNode* node) const
  {
    // Note: caller must hold m_Mutex

    std::string path = "/" + node->GetName();

    auto sources = m_DataStorage->GetSources(node);
    while (sources->Size() > 0)
    {
      auto parent = sources->ElementAt(0).GetPointer();
      path = "/" + parent->GetName() + path;
      sources = m_DataStorage->GetSources(parent);
    }

    return path;
  }

  int DataStorageBridge::GetChildrenCount(const DataNode* node) const
  {
    // Note: caller must hold m_Mutex

    auto derivatives = m_DataStorage->GetDerivations(node);
    return static_cast<int>(derivatives->Size());
  }

}
