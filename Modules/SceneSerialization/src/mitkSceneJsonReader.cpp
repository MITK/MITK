/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSceneJsonReader.h>

#include "mitkSceneReaderHelpers.h"

#include <mitkBaseProperty.h>
#include <mitkDataNode.h>
#include <mitkException.h>
#include <mitkFileSystem.h>
#include <mitkIOUtil.h>
#include <mitkLog.h>
#include <mitkProperties.h>
#include <mitkPropertyList.h>
#include <mitkUIDGenerator.h>
#include <mitkUIDManipulator.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace
{
  using json = nlohmann::json;

  constexpr const char *kSceneType = "org.mitk.scene";
  constexpr int kSupportedVersion = 1;

  const std::set<std::string> kRootKnownKeys = {"type", "version", "metadata", "nodes"};
  const std::set<std::string> kMetadataKnownKeys = {"description"};
  const std::set<std::string> kNodeKnownKeys = {"uid", "parent_uid", "data", "properties", "context_properties"};
  const std::set<std::string> kDataKnownKeys = {"data_type", "_file", "uid", "properties"};
  const std::set<std::string> kKnownPropertyMapMetaKeys = {"_loadstyle", "_file"};

  struct SceneNodeEntry
  {
    std::string uid;                    // always filled (auto-generated if absent)
    std::string parentUid;              // empty if top-level
    bool hasExplicitParent = false;     // parent_uid was explicitly given (and non-null)
    const json *nodeJson = nullptr;     // source JSON object
    mitk::DataNode::Pointer dataNode;   // created in pass 1
  };

  fs::path ResolvePath(const fs::path &basePath, const std::string &relativeOrAbsolute)
  {
    fs::path p(relativeOrAbsolute);
    if (p.is_absolute())
    {
      return p;
    }
    return basePath / p;
  }

  void WarnUnknownKeys(const json &obj, const std::set<std::string> &known, const std::string &context)
  {
    if (!obj.is_object())
      return;
    for (auto it = obj.begin(); it != obj.end(); ++it)
    {
      if (known.find(it.key()) == known.end())
      {
        MITK_WARN << "Unknown key '" << it.key() << "' in " << context << " - ignoring.";
      }
    }
  }

  enum class LoadStyle
  {
    Modify,
    Replace
  };

  LoadStyle ParseLoadStyle(const json &propertyMap, const std::string &context)
  {
    if (!propertyMap.is_object())
      return LoadStyle::Modify;
    auto it = propertyMap.find("_loadstyle");
    if (it == propertyMap.end() || it->is_null())
      return LoadStyle::Modify;
    if (!it->is_string())
    {
      mitkThrow() << "Invalid '_loadstyle' in " << context << ": expected string ('modify' or 'replace').";
    }
    const std::string value = it->get<std::string>();
    if (value == "modify")
      return LoadStyle::Modify;
    if (value == "replace")
      return LoadStyle::Replace;
    mitkThrow() << "Invalid '_loadstyle' value '" << value << "' in " << context
                << ". Expected 'modify' or 'replace'.";
  }

  /**
   * \brief Resolve a property-map JSON object, optionally following `_file`.
   *
   * Returns a self-contained JSON object that no longer references external
   * files. Inline `_`-meta keys take precedence over the external file's meta
   * keys. If `_file` is set, the external file must contain a property map
   * and the inline object must not contain any non-meta keys.
   */
  json ResolvePropertyMap(const json &mapJson, const fs::path &basePath, const std::string &context)
  {
    if (!mapJson.is_object())
    {
      mitkThrow() << "Property map in " << context << " must be a JSON object.";
    }

    auto fileIt = mapJson.find("_file");
    if (fileIt == mapJson.end() || fileIt->is_null())
      return mapJson;

    if (!fileIt->is_string())
    {
      mitkThrow() << "'_file' in property map (" << context << ") must be a string.";
    }

    // inline must only contain meta keys when _file is present
    for (auto it = mapJson.begin(); it != mapJson.end(); ++it)
    {
      if (!it.key().empty() && it.key().front() != '_')
      {
        mitkThrow() << "Property map in " << context
                    << " specifies '_file' but also contains inline property '" << it.key()
                    << "'. Mixing the two is not allowed.";
      }
    }

    const fs::path path = ResolvePath(basePath, fileIt->get<std::string>());
    std::ifstream in(path.string());
    if (!in.good())
    {
      mitkThrow() << "Property map '_file' in " << context << " refers to missing/unreadable file '"
                  << path.string() << "'.";
    }

    json external;
    try
    {
      in >> external;
    }
    catch (const json::exception &e)
    {
      mitkThrow() << "Property map '_file' in " << context << " ('" << path.string()
                  << "') is not valid JSON: " << e.what();
    }

    if (!external.is_object())
    {
      mitkThrow() << "Property map file '" << path.string() << "' (referenced from " << context
                  << ") must contain a JSON object.";
    }

    // Inline meta keys override external ones.
    for (auto it = mapJson.begin(); it != mapJson.end(); ++it)
    {
      if (it.key() == "_file")
        continue;
      external[it.key()] = it.value();
    }

    // Prevent infinite recursion in case the external file itself sets _file.
    if (external.contains("_file"))
    {
      mitkThrow() << "Nested '_file' references are not supported (in " << path.string() << ").";
    }

    return external;
  }

  /**
   * \brief Write resolved property-map keys into \p targetList.
   *
   * The JSON must already be resolved (no `_file` indirection). Honors no
   * `_loadstyle` — the caller decides how the list was prepared. Unknown
   * `_`-meta keys produce a warning and are skipped.
   */
  void ApplyResolvedPropertyMap(mitk::PropertyList &targetList,
                                const json &resolved,
                                const std::string &context)
  {
    for (auto it = resolved.begin(); it != resolved.end(); ++it)
    {
      const std::string &key = it.key();
      if (key.empty())
        continue;
      if (key.front() == '_')
      {
        if (kKnownPropertyMapMetaKeys.find(key) == kKnownPropertyMapMetaKeys.end())
        {
          MITK_WARN << "Unknown meta key '" << key << "' in property map (" << context
                    << "). Ignoring (forward compatibility).";
        }
        continue;
      }

      try
      {
        mitk::BaseProperty::Pointer prop = mitk::ConvertPropertyFromSelfContainedJson(it.value());
        if (prop.IsNotNull())
        {
          targetList.SetProperty(key, prop);
        }
      }
      catch (const mitk::Exception &e)
      {
        MITK_ERROR << "Failed to deserialize property '" << key << "' in " << context << ": " << e.what();
      }
    }
  }

  /**
   * \brief Apply a property map to \p targetList.
   *
   * Honors `_loadstyle` (Replace clears the list first; Modify merges on top
   * of whatever is already in the list), resolves `_file` references, and
   * skips / warns about other `_`-meta keys.
   */
  void ApplyPropertyMap(mitk::PropertyList &targetList,
                        const json &mapJson,
                        const fs::path &basePath,
                        const std::string &context)
  {
    const json resolved = ResolvePropertyMap(mapJson, basePath, context);
    const LoadStyle style = ParseLoadStyle(resolved, context);

    if (style == LoadStyle::Replace)
    {
      targetList.Clear();
    }

    ApplyResolvedPropertyMap(targetList, resolved, context);
  }

  /**
   * \brief Extract the integer `layer` property value from a node JSON object
   *        for pre-Add ordering.
   *
   * Accepts both the simple primitive form (`"layer": 2`) and the tagged
   * object form (`"layer": {"type": "IntProperty", "value": 2}`) via the
   * standard property converter. Returns 0 if no usable layer value is
   * present or if the property is of a non-integer type.
   */
  int ExtractLayer(const json &nodeJson)
  {
    auto propsIt = nodeJson.find("properties");
    if (propsIt == nodeJson.end() || !propsIt->is_object())
      return 0;
    auto layerIt = propsIt->find("layer");
    if (layerIt == propsIt->end() || layerIt->is_null())
      return 0;
    try
    {
      mitk::BaseProperty::Pointer prop = mitk::ConvertPropertyFromSelfContainedJson(*layerIt);
      if (auto *intProp = dynamic_cast<mitk::IntProperty *>(prop.GetPointer()))
        return intProp->GetValue();
    }
    catch (const mitk::Exception &)
    {
      // Ignore — sort falls back to layer 0 for this node.
    }
    return 0;
  }
}

mitk::SceneJsonReader::SceneJsonReader() = default;
mitk::SceneJsonReader::~SceneJsonReader() = default;

bool mitk::SceneJsonReader::LoadScene(const std::string &sceneSourcePath, DataStorage *storage, bool clearStorageFirst)
{
  if (storage == nullptr)
  {
    mitkThrow() << "SceneJsonReader::LoadScene called with null DataStorage.";
  }

  // ---- 1. read and parse -------------------------------------------------

  std::ifstream in(sceneSourcePath);
  if (!in.good())
  {
    mitkThrow() << "Could not open scene file '" << sceneSourcePath << "' for reading.";
  }

  json document;
  try
  {
    in >> document;
  }
  catch (const json::exception &e)
  {
    mitkThrow() << "Scene file '" << sceneSourcePath << "' is not valid JSON: " << e.what();
  }

  // ---- 2. validate root --------------------------------------------------

  if (!document.is_object())
  {
    mitkThrow() << "Scene file '" << sceneSourcePath << "' root must be a JSON object.";
  }

  auto typeIt = document.find("type");
  if (typeIt == document.end() || !typeIt->is_string() || typeIt->get<std::string>() != kSceneType)
  {
    mitkThrow() << "Scene file '" << sceneSourcePath << "' has missing or wrong 'type' field (expected '"
                << kSceneType << "').";
  }

  auto versionIt = document.find("version");
  if (versionIt == document.end() || !versionIt->is_number_integer())
  {
    mitkThrow() << "Scene file '" << sceneSourcePath << "' has missing or non-integer 'version' field.";
  }
  const int version = versionIt->get<int>();
  if (version != kSupportedVersion)
  {
    mitkThrow() << "Scene file '" << sceneSourcePath << "' has unsupported version " << version
                << " (supported: " << kSupportedVersion << ").";
  }

  WarnUnknownKeys(document, kRootKnownKeys, "root");

  auto metadataIt = document.find("metadata");
  if (metadataIt != document.end() && metadataIt->is_object())
  {
    WarnUnknownKeys(*metadataIt, kMetadataKnownKeys, "metadata");
  }

  auto nodesIt = document.find("nodes");
  if (nodesIt == document.end() || !nodesIt->is_array())
  {
    mitkThrow() << "Scene file '" << sceneSourcePath << "' is missing required 'nodes' array.";
  }

  // The scene descriptor is now well-formed enough to commit to the load;
  // clearing the DataStorage before this point would silently destroy the
  // caller's session on a missing or malformed scene file.
  if (clearStorageFirst)
  {
    try
    {
      storage->Remove(storage->GetAll());
    }
    catch (...)
    {
      MITK_ERROR << "DataStorage cannot be cleared properly.";
    }
  }

  const fs::path basePath = fs::path(sceneSourcePath).parent_path();

  // ---- 3. enumerate nodes; validate uids & graph ------------------------

  std::vector<SceneNodeEntry> entries;
  entries.reserve(nodesIt->size());

  UIDGenerator autoUidGen("scene_autoUID_");

  for (std::size_t i = 0; i < nodesIt->size(); ++i)
  {
    const json &nodeJson = (*nodesIt)[i];
    if (!nodeJson.is_object())
    {
      mitkThrow() << "nodes[" << i << "] must be a JSON object.";
    }

    WarnUnknownKeys(nodeJson, kNodeKnownKeys, "nodes[" + std::to_string(i) + "]");

    SceneNodeEntry entry;
    entry.nodeJson = &nodeJson;

    auto uidIt = nodeJson.find("uid");
    if (uidIt != nodeJson.end() && !uidIt->is_null())
    {
      if (!uidIt->is_string())
      {
        mitkThrow() << "nodes[" << i << "].uid must be a string or null.";
      }
      entry.uid = uidIt->get<std::string>();
      if (entry.uid.empty())
      {
        mitkThrow() << "nodes[" << i << "].uid must not be an empty string.";
      }
    }
    else
    {
      entry.uid = autoUidGen.GetUID();
    }

    auto parentIt = nodeJson.find("parent_uid");
    if (parentIt != nodeJson.end() && !parentIt->is_null())
    {
      if (!parentIt->is_string())
      {
        mitkThrow() << "nodes[" << i << "].parent_uid must be a string or null.";
      }
      entry.parentUid = parentIt->get<std::string>();
      if (entry.parentUid.empty())
      {
        mitkThrow() << "nodes[" << i
                    << "].parent_uid must not be an empty string (use null for a top-level node).";
      }
      entry.hasExplicitParent = true;
    }

    entries.push_back(std::move(entry));
  }

  // duplicate uid check
  std::map<std::string, std::size_t> uidIndex;
  for (std::size_t i = 0; i < entries.size(); ++i)
  {
    auto [it, inserted] = uidIndex.emplace(entries[i].uid, i);
    if (!inserted)
    {
      mitkThrow() << "Duplicate node uid '" << entries[i].uid << "' (nodes[" << it->second
                  << "] and nodes[" << i << "]).";
    }
  }

  // dangling parent_uid
  for (std::size_t i = 0; i < entries.size(); ++i)
  {
    if (entries[i].hasExplicitParent && uidIndex.find(entries[i].parentUid) == uidIndex.end())
    {
      mitkThrow() << "nodes[" << i << "] (uid '" << entries[i].uid
                  << "') has unknown parent_uid '" << entries[i].parentUid << "'.";
    }
  }

  // circular parent chain check
  for (std::size_t i = 0; i < entries.size(); ++i)
  {
    std::set<std::string> visited;
    std::string cur = entries[i].uid;
    while (true)
    {
      visited.insert(cur);
      const SceneNodeEntry &cursorEntry = entries[uidIndex.find(cur)->second];
      if (!cursorEntry.hasExplicitParent)
        break;
      const std::string &next = cursorEntry.parentUid;
      if (visited.count(next) > 0)
      {
        mitkThrow() << "Circular parent_uid chain detected involving node '" << entries[i].uid
                    << "'. Cycle revisits '" << next << "'.";
      }
      cur = next;
    }
  }

  bool nonFatalError = false;

  // ---- 4. Pass 1: create nodes, load data, apply data-level properties,
  //                 apply "replace" style property maps (before storage Add) --

  for (auto &entry : entries)
  {
    const json &nodeJson = *entry.nodeJson;
    entry.dataNode = DataNode::New();

    auto dataIt = nodeJson.find("data");
    if (dataIt != nodeJson.end() && !dataIt->is_null())
    {
      if (!dataIt->is_object())
      {
        mitkThrow() << "Node '" << entry.uid << "': 'data' must be a JSON object.";
      }
      const json &dataJson = *dataIt;
      WarnUnknownKeys(dataJson, kDataKnownKeys, "node '" + entry.uid + "'.data");

      // data_type is optional in v1 and currently informative only. If present,
      // it must be a string; stricter validation (matching the loaded BaseData
      // class against it) is reserved for a future version.
      auto dataTypeIt = dataJson.find("data_type");
      if (dataTypeIt != dataJson.end() && !dataTypeIt->is_null() && !dataTypeIt->is_string())
      {
        mitkThrow() << "Node '" << entry.uid << "': 'data.data_type' must be a string if present.";
      }

      auto fileIt = dataJson.find("_file");
      if (fileIt == dataJson.end() || !fileIt->is_string() || fileIt->get<std::string>().empty())
      {
        mitkThrow() << "Node '" << entry.uid
                    << "': 'data._file' is required (inline data sources are not yet supported).";
      }

      const fs::path dataPath = ResolvePath(basePath, fileIt->get<std::string>());
      if (!fs::exists(dataPath))
      {
        mitkThrow() << "Node '" << entry.uid << "': data file '" << dataPath.string() << "' does not exist.";
      }

      // Pre-parse data-level properties so the IO reader can see them as
      // read-only meta data (analogous to SceneReaderV1). The same parsed
      // PropertyList is later transferred onto the loaded BaseData's own
      // property list according to _loadstyle, so the JSON is parsed once.
      PropertyList::Pointer preloadedDataProps;
      LoadStyle dataStyle = LoadStyle::Modify;
      auto dataPropsIt = dataJson.find("properties");
      const std::string dataPropsCtx = "node '" + entry.uid + "'.data.properties";
      if (dataPropsIt != dataJson.end() && !dataPropsIt->is_null())
      {
        const json resolvedDataMap = ResolvePropertyMap(*dataPropsIt, basePath, dataPropsCtx);
        dataStyle = ParseLoadStyle(resolvedDataMap, dataPropsCtx);
        preloadedDataProps = PropertyList::New();
        ApplyResolvedPropertyMap(*preloadedDataProps, resolvedDataMap, dataPropsCtx);
      }

      try
      {
        auto baseData = IOUtil::Load(dataPath.string(), preloadedDataProps.GetPointer());
        entry.dataNode->SetData(baseData);
      }
      catch (const std::exception &e)
      {
        mitkThrow() << "Node '" << entry.uid << "': failed to read data file '" << dataPath.string()
                    << "': " << e.what();
      }

      if (entry.dataNode->GetData() == nullptr)
      {
        mitkThrow() << "Node '" << entry.uid << "': data file '" << dataPath.string()
                    << "' could not be loaded.";
      }

      auto dataUidIt = dataJson.find("uid");
      if (dataUidIt != dataJson.end() && !dataUidIt->is_null())
      {
        if (!dataUidIt->is_string())
        {
          mitkThrow() << "Node '" << entry.uid << "': 'data.uid' must be a string.";
        }
        UIDManipulator manip(entry.dataNode->GetData());
        manip.SetUID(dataUidIt->get<std::string>());
      }

      // Transfer the pre-parsed data-level properties onto the loaded
      // BaseData's PropertyList. The same PropertyList was handed to
      // IOUtil::Load above as read-only hints; authoring it onto the loaded
      // object is what makes the properties persist. JSON is parsed once.
      if (preloadedDataProps.IsNotNull())
      {
        BaseData *loaded = entry.dataNode->GetData();
        if (loaded->GetPropertyList() == nullptr)
        {
          loaded->SetPropertyList(PropertyList::New());
        }
        PropertyList *existing = loaded->GetPropertyList();
        if (dataStyle == LoadStyle::Replace)
        {
          existing->Clear();
        }
        existing->ConcatenatePropertyList(preloadedDataProps, true);
      }

      mitk::SceneReaderHelpers::ApplyProportionalTimeGeometryProperties(entry.dataNode->GetData());
    }
  }

  // ---- 5. Pass 2: topological add + apply property maps ------------------

  // Process in waves such that parents are added before children. Within
  // each wave, ready nodes are added in ascending `layer` order so that
  // DataStorage observer ordering (and derived UI assumptions like stacking)
  // is deterministic across loads.
  std::vector<bool> added(entries.size(), false);
  std::size_t addedCount = 0;
  while (addedCount < entries.size())
  {
    std::vector<std::size_t> ready;
    ready.reserve(entries.size() - addedCount);
    for (std::size_t i = 0; i < entries.size(); ++i)
    {
      if (added[i])
        continue;
      const SceneNodeEntry &entry = entries[i];
      if (entry.hasExplicitParent && !added[uidIndex.find(entry.parentUid)->second])
        continue;
      ready.push_back(i);
    }

    if (ready.empty())
    {
      // Should not happen given the cycle check above, but guard anyway.
      mitkThrow() << "Internal error: could not make progress adding nodes; likely a bug in graph validation.";
    }

    std::stable_sort(ready.begin(), ready.end(),
                     [&entries](std::size_t a, std::size_t b)
                     {
                       return ExtractLayer(*entries[a].nodeJson) < ExtractLayer(*entries[b].nodeJson);
                     });

    for (std::size_t i : ready)
    {
      const SceneNodeEntry &entry = entries[i];

      DataStorage::SetOfObjects::Pointer parents = DataStorage::SetOfObjects::New();
      if (entry.hasExplicitParent)
      {
        parents->push_back(entries[uidIndex.find(entry.parentUid)->second].dataNode);
      }

      storage->Add(entry.dataNode, parents);
      added[i] = true;
      ++addedCount;

      // Mapper-assigned defaults are now in place on the node; 'replace'
      // clears them and 'modify' merges on top of them.
      const json &nodeJson = *entry.nodeJson;

      auto propsIt = nodeJson.find("properties");
      if (propsIt != nodeJson.end() && !propsIt->is_null())
      {
        PropertyList *defaultList = entry.dataNode->GetPropertyList();
        ApplyPropertyMap(*defaultList, *propsIt, basePath, "node '" + entry.uid + "'.properties");
      }

      auto ctxIt = nodeJson.find("context_properties");
      if (ctxIt != nodeJson.end() && !ctxIt->is_null())
      {
        if (!ctxIt->is_object())
        {
          mitkThrow() << "Node '" << entry.uid << "': 'context_properties' must be a JSON object.";
        }
        for (auto cit = ctxIt->begin(); cit != ctxIt->end(); ++cit)
        {
          const std::string contextName = cit.key();
          PropertyList *ctxList = entry.dataNode->GetPropertyList(contextName);
          if (ctxList == nullptr)
          {
            MITK_WARN << "Could not obtain property list for context '" << contextName << "' on node '"
                      << entry.uid << "'. Skipping.";
            nonFatalError = true;
            continue;
          }
          ApplyPropertyMap(*ctxList, cit.value(), basePath,
                           "node '" + entry.uid + "'.context_properties['" + contextName + "']");
        }
      }
    }
  }

  return !nonFatalError;
}
