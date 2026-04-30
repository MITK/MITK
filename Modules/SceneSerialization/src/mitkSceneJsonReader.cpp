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

  constexpr const char *SCENE_TYPE = "org.mitk.scene";
  constexpr int SUPPORTED_VERSION = 1;
  constexpr const char *SUPPORTED_TRANSFER_MODE = "file-reference";

  // Root-object field names.
  constexpr const char *FIELD_TYPE = "type";
  constexpr const char *FIELD_VERSION = "version";
  constexpr const char *FIELD_METADATA = "metadata";
  constexpr const char *FIELD_NODES = "nodes";

  // Node-object field names.
  constexpr const char *FIELD_UID = "uid";
  constexpr const char *FIELD_PARENT_UID = "parent_uid";
  constexpr const char *FIELD_DATA_TYPE = "data_type";
  constexpr const char *FIELD_DATA_UID = "data_uid";
  constexpr const char *FIELD_TRANSFER = "transfer";
  constexpr const char *FIELD_DATA_PROPERTIES = "data_properties";
  constexpr const char *FIELD_PROPERTIES = "properties";
  constexpr const char *FIELD_CONTEXT_PROPERTIES = "context_properties";

  // Transfer-object field names.
  constexpr const char *FIELD_MODE = "mode";
  constexpr const char *FIELD_FILE_PATH = "file_path";
  constexpr const char *FIELD_SIZE_BYTES = "size_bytes";
  constexpr const char *FIELD_DIRECTORY_PATH = "directory_path";

  // Property-map meta keys and their accepted values.
  constexpr const char *META_LOADSTYLE = "_loadstyle";
  constexpr const char *META_FILE = "_file";
  constexpr const char *LOADSTYLE_MODIFY = "modify";
  constexpr const char *LOADSTYLE_REPLACE = "replace";

  // Property-name keys read directly by the reader (not just forwarded).
  constexpr const char *PROPERTY_LAYER = "layer";

  // Metadata sub-keys.
  constexpr const char *METADATA_DESCRIPTION = "description";

  const std::set<std::string> ROOT_KNOWN_KEYS = {FIELD_TYPE, FIELD_VERSION, FIELD_METADATA, FIELD_NODES};
  const std::set<std::string> METADATA_KNOWN_KEYS = {METADATA_DESCRIPTION};
  const std::set<std::string> NODE_KNOWN_KEYS = {FIELD_UID,
                                                FIELD_PARENT_UID,
                                                FIELD_DATA_TYPE,
                                                FIELD_DATA_UID,
                                                FIELD_TRANSFER,
                                                FIELD_DATA_PROPERTIES,
                                                FIELD_PROPERTIES,
                                                FIELD_CONTEXT_PROPERTIES};
  const std::set<std::string> TRANSFER_KNOWN_KEYS = {FIELD_MODE, FIELD_FILE_PATH, FIELD_SIZE_BYTES, FIELD_DIRECTORY_PATH};
  const std::set<std::string> KNOWN_PROPERTY_MAP_META_KEYS = {META_LOADSTYLE, META_FILE};

  struct SceneNodeEntry
  {
    std::string uid;                    // always filled (auto-generated if absent)
    std::string parentUid;              // empty if top-level
    bool hasExplicitParent = false;     // parent_uid was explicitly given (and non-null)
    /**
     * Non-owning pointer into the JSON document parsed in `LoadScene`;
     * valid for the duration of that call. Must not outlive the
     * `document` local variable in `LoadScene`.
     */
    const json *nodeJson = nullptr;
    mitk::DataNode::Pointer dataNode;   // created in pass 1
  };

  /**
   * \brief Resolve a scene-relative path against \p basePath.
   *
   * Absolute paths are returned as-is. Relative paths are joined to
   * \p basePath and lexically normalised, so `..` segments are allowed
   * and collapsed against the scene directory's own path. A warning is
   * logged whenever the resolved path falls outside \p basePath
   * (absolute paths or `..` traversal that escapes the scene
   * directory), so an operator loading a scene from an untrusted
   * source notices references that reach outside the scene's own tree.
   * The `.mitk` ZIP scene format is structurally confined to its
   * unpacked directory; this warning brings the JSON format closer to
   * that asymmetry without breaking legitimate cross-tree references.
   * Scene files are user documents — callers remain responsible for
   * not loading scenes from untrusted sources.
   */
  fs::path ResolvePath(const fs::path &basePath, const std::string &relativeOrAbsolute)
  {
    const fs::path p(relativeOrAbsolute);
    if (p.is_absolute())
    {
      MITK_WARN << "Scene references absolute path '" << p.string()
                << "' which is outside the scene directory '" << basePath.string()
                << "'. Loading anyway - only load scene files from trusted sources.";
      return p;
    }
    const fs::path resolved = (basePath / p).lexically_normal();
    const fs::path normalizedBase = basePath.lexically_normal();
    const fs::path relative = resolved.lexically_relative(normalizedBase);
    if (!relative.empty() && relative.begin() != relative.end() && *relative.begin() == fs::path(".."))
    {
      MITK_WARN << "Scene path '" << relativeOrAbsolute
                << "' resolves outside the scene directory '" << normalizedBase.string()
                << "' (resolved: '" << resolved.string()
                << "'). Loading anyway - only load scene files from trusted sources.";
    }
    return resolved;
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

  /**
   * \brief Structural (I/O-free) validation of a property-map JSON object.
   *
   * Checks the invariants that do not require reading any external file:
   *  - the node is a JSON object,
   *  - `_loadstyle`, if present, is the string `"modify"` or `"replace"`,
   *  - if `_file` is present, no non-meta keys appear beside it.
   *
   * Called from the Pass-1 pre-validation so that these structural errors
   * throw before Pass 2 starts mutating the DataStorage. I/O-bound errors
   * (missing `_file` target, invalid JSON in the external map, nested
   * `_file`) remain non-fatal per-node errors in Pass 2.
   */
  void ValidatePropertyMapShape(const json &mapJson, const std::string &context)
  {
    if (!mapJson.is_object())
    {
      mitkThrow() << "Property map in " << context << " must be a JSON object.";
    }

    const auto loadstyleIt = mapJson.find(META_LOADSTYLE);
    if (loadstyleIt != mapJson.end() && !loadstyleIt->is_null())
    {
      if (!loadstyleIt->is_string())
      {
        mitkThrow() << "Invalid '_loadstyle' in " << context << ": expected string ('modify' or 'replace').";
      }
      const std::string value = loadstyleIt->get<std::string>();
      if (value != LOADSTYLE_MODIFY && value != LOADSTYLE_REPLACE)
      {
        mitkThrow() << "Invalid '_loadstyle' value '" << value << "' in " << context
                    << ". Expected 'modify' or 'replace'.";
      }
    }

    const auto fileIt = mapJson.find(META_FILE);
    if (fileIt != mapJson.end() && !fileIt->is_null())
    {
      if (!fileIt->is_string())
      {
        mitkThrow() << "'_file' in property map (" << context << ") must be a string.";
      }
      for (auto it = mapJson.begin(); it != mapJson.end(); ++it)
      {
        if (!it.key().empty() && it.key().front() != '_')
        {
          mitkThrow() << "Property map in " << context
                      << " specifies '_file' but also contains inline property '" << it.key()
                      << "'. Mixing the two is not allowed.";
        }
      }
    }
  }

  LoadStyle ParseLoadStyle(const json &propertyMap, const std::string &context)
  {
    if (!propertyMap.is_object())
      return LoadStyle::Modify;
    const auto it = propertyMap.find(META_LOADSTYLE);
    if (it == propertyMap.end() || it->is_null())
      return LoadStyle::Modify;
    if (!it->is_string())
    {
      mitkThrow() << "Invalid '_loadstyle' in " << context << ": expected string ('modify' or 'replace').";
    }
    const std::string value = it->get<std::string>();
    if (value == LOADSTYLE_MODIFY)
      return LoadStyle::Modify;
    if (value == LOADSTYLE_REPLACE)
      return LoadStyle::Replace;
    mitkThrow() << "Invalid '_loadstyle' value '" << value << "' in " << context
                << ". Expected 'modify' or 'replace'.";
  }

  /**
   * \brief Resolve `_file` indirection in a property-map JSON object.
   *
   * Returns a self-contained JSON object that no longer references external
   * files. Inline `_`-meta keys take precedence over the external file's meta
   * keys. If `_file` is set, the external file must contain a property map
   * and the inline object must not contain any non-meta keys. Only one level
   * of indirection is allowed: an externally referenced map must not itself
   * contain `_file`. `_loadstyle` is not interpreted here; it is resolved by
   * the caller once the map has been assembled.
   */
  json ResolvePropertyMap(const json &mapJson, const fs::path &basePath, const std::string &context)
  {
    if (!mapJson.is_object())
    {
      mitkThrow() << "Property map in " << context << " must be a JSON object.";
    }

    const auto fileIt = mapJson.find(META_FILE);
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
      if (it.key() == META_FILE)
        continue;
      external[it.key()] = it.value();
    }

    // Prevent infinite recursion in case the external file itself sets _file.
    if (external.contains(META_FILE))
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
        if (KNOWN_PROPERTY_MAP_META_KEYS.find(key) == KNOWN_PROPERTY_MAP_META_KEYS.end())
        {
          MITK_WARN << "Unknown meta key '" << key << "' in property map (" << context
                    << "). Ignoring (forward compatibility).";
        }
        continue;
      }

      mitk::BaseProperty::Pointer prop;
      try
      {
        prop = mitk::ConvertPropertyFromSelfContainedJson(it.value());
      }
      catch (const mitk::Exception &e)
      {
        mitkThrow() << "Failed to deserialize property '" << key << "' in " << context << ": " << e.what();
      }
      catch (const std::exception &e)
      {
        // ConvertPropertyFromSelfContainedJson can also propagate non-mitk
        // exceptions (e.g. nlohmann::json::out_of_range when a tagged value
        // has the wrong shape). Wrap as mitk::Exception so the Pass-2 catch
        // around ApplyPropertyMap can downgrade this to a non-fatal per-node
        // error rather than aborting the load.
        mitkThrow() << "Failed to deserialize property '" << key << "' in " << context << ": " << e.what();
      }
      if (prop.IsNull())
      {
        mitkThrow() << "Property '" << key << "' in " << context
                    << ": unknown property type or value cannot be deserialized.";
      }
      targetList.SetProperty(key, prop);
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
    const auto propsIt = nodeJson.find(FIELD_PROPERTIES);
    if (propsIt == nodeJson.end() || !propsIt->is_object())
      return 0;
    auto layerIt = propsIt->find(PROPERTY_LAYER);
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
    catch (const std::exception &)
    {
      // ConvertPropertyFromSelfContainedJson can also propagate non-mitk
      // exceptions (e.g. nlohmann::json::out_of_range on a malformed tagged
      // value). Treat the same as a missing/unusable layer: fall back to 0.
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

  const auto typeIt = document.find(FIELD_TYPE);
  if (typeIt == document.end() || !typeIt->is_string() || typeIt->get<std::string>() != SCENE_TYPE)
  {
    mitkThrow() << "Scene file '" << sceneSourcePath << "' has missing or wrong 'type' field (expected '"
                << SCENE_TYPE << "').";
  }

  const auto versionIt = document.find(FIELD_VERSION);
  if (versionIt == document.end() || !versionIt->is_number_integer())
  {
    mitkThrow() << "Scene file '" << sceneSourcePath << "' has missing or non-integer 'version' field.";
  }
  const int version = versionIt->get<int>();
  if (version != SUPPORTED_VERSION)
  {
    mitkThrow() << "Scene file '" << sceneSourcePath << "' has unsupported version " << version
                << " (supported: " << SUPPORTED_VERSION << ").";
  }

  WarnUnknownKeys(document, ROOT_KNOWN_KEYS, "root");

  const auto metadataIt = document.find(FIELD_METADATA);
  if (metadataIt != document.end() && metadataIt->is_object())
  {
    WarnUnknownKeys(*metadataIt, METADATA_KNOWN_KEYS, "metadata");
  }

  const auto nodesIt = document.find(FIELD_NODES);
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

    WarnUnknownKeys(nodeJson, NODE_KNOWN_KEYS, "nodes[" + std::to_string(i) + "]");

    SceneNodeEntry entry;
    entry.nodeJson = &nodeJson;

    const auto uidIt = nodeJson.find(FIELD_UID);
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

    const auto parentIt = nodeJson.find(FIELD_PARENT_UID);
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
      const auto cursorIt = uidIndex.find(cur);
      if (cursorIt == uidIndex.end())
      {
        mitkThrow() << "Internal error: uid '" << cur << "' missing from uidIndex during cycle check.";
      }
      const SceneNodeEntry &cursorEntry = entries[cursorIt->second];
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

  // ---- 3b. Shape-only pre-validation of node-level property maps.
  //
  // Anything that would cause Pass 2 to throw after `storage->Add` is
  // caught here instead, so a structural error can never leave the storage
  // half-populated. Property *value* errors (unknown type tag, failed
  // conversion) remain non-fatal and are logged in Pass 2.
  for (const auto &entry : entries)
  {
    const json &nodeJson = *entry.nodeJson;

    const auto dataPropsIt = nodeJson.find(FIELD_DATA_PROPERTIES);
    if (dataPropsIt != nodeJson.end() && !dataPropsIt->is_null())
    {
      if (!dataPropsIt->is_object())
      {
        mitkThrow() << "Node '" << entry.uid << "': 'data_properties' must be a JSON object.";
      }
      ValidatePropertyMapShape(*dataPropsIt, "node '" + entry.uid + "'.data_properties");
    }

    const auto propsIt = nodeJson.find(FIELD_PROPERTIES);
    if (propsIt != nodeJson.end() && !propsIt->is_null())
    {
      if (!propsIt->is_object())
      {
        mitkThrow() << "Node '" << entry.uid << "': 'properties' must be a JSON object.";
      }
      ValidatePropertyMapShape(*propsIt, "node '" + entry.uid + "'.properties");
    }

    const auto ctxIt = nodeJson.find(FIELD_CONTEXT_PROPERTIES);
    if (ctxIt != nodeJson.end() && !ctxIt->is_null())
    {
      if (!ctxIt->is_object())
      {
        mitkThrow() << "Node '" << entry.uid << "': 'context_properties' must be a JSON object.";
      }
      for (auto cit = ctxIt->begin(); cit != ctxIt->end(); ++cit)
      {
        const std::string &contextName = cit.key();
        if (contextName.empty() || contextName == "null")
        {
          mitkThrow() << "Node '" << entry.uid << "': invalid 'context_properties' key '"
                      << contextName << "'. Use the top-level 'properties' field for the default context.";
        }
        if (!cit.value().is_object())
        {
          mitkThrow() << "Node '" << entry.uid << "': 'context_properties[\"" << contextName
                      << "\"]' must be a JSON object.";
        }
        ValidatePropertyMapShape(cit.value(),
                                 "node '" + entry.uid + "'.context_properties['" + contextName + "']");
      }
    }
  }

  bool nonFatalError = false;

  // Track data_uid values restored in this load so we can warn about
  // collisions between BaseData objects loaded from the scene file. Note
  // that BaseData uniqueness is not currently enforced across a
  // DataStorage as a whole (e.g. loading the same file twice produces two
  // BaseData with the same UID), so a collision here is logged only.
  std::set<std::string> seenDataUids;

  // ---- 4. Pass 1: create nodes, load data, apply data-level properties.
  //
  // Data-level properties are applied here because they live on the loaded
  // BaseData's own property list, which mappers do not touch.
  //
  // Node-level property maps (`properties`, `context_properties`) are
  // deliberately deferred to Pass 2 (after storage->Add). The rationale is
  // that when a RenderingManager is observing the storage, Add triggers
  // mapper instantiation whose SetDefaultProperties populates the node's
  // property lists synchronously; Pass 2's `modify` then merges on top of
  // those defaults and `replace` clears them on purpose. In headless
  // contexts (unit tests, CLI, REST server) no such defaults exist, so
  // `modify` and `replace` behave identically on an empty list. This is the
  // intended contract.

  for (auto &entry : entries)
  {
    const json &nodeJson = *entry.nodeJson;
    entry.dataNode = DataNode::New();

    // Validate the optional informative data_type field.
    const auto dataTypeIt = nodeJson.find(FIELD_DATA_TYPE);
    if (dataTypeIt != nodeJson.end() && !dataTypeIt->is_null() && !dataTypeIt->is_string())
    {
      mitkThrow() << "Node '" << entry.uid << "': 'data_type' must be a string or null.";
    }
    const bool hasDataType = dataTypeIt != nodeJson.end() && !dataTypeIt->is_null();

    const auto transferIt = nodeJson.find(FIELD_TRANSFER);
    const bool hasTransfer = transferIt != nodeJson.end() && !transferIt->is_null();
    const auto dataUidIt = nodeJson.find(FIELD_DATA_UID);
    const bool hasDataUid = dataUidIt != nodeJson.end() && !dataUidIt->is_null();
    const auto dataPropsIt = nodeJson.find(FIELD_DATA_PROPERTIES);
    const bool hasDataProps = dataPropsIt != nodeJson.end() && !dataPropsIt->is_null();

    if (!hasTransfer)
    {
      // Data-less node. data_type (if present) has been validated as a
      // string or null above but is not persisted — it is author-facing
      // documentation only. data_uid / data_properties are meaningless
      // here and warned about.
      if (hasDataUid)
      {
        MITK_WARN << "Node '" << entry.uid
                  << "': 'data_uid' is ignored because the node carries no 'transfer' block.";
        nonFatalError = true;
      }
      if (hasDataProps)
      {
        MITK_WARN << "Node '" << entry.uid
                  << "': 'data_properties' is ignored because the node carries no 'transfer' block.";
        nonFatalError = true;
      }
      continue;
    }

    // transfer is present -> load data.
    if (!transferIt->is_object())
    {
      mitkThrow() << "Node '" << entry.uid << "': 'transfer' must be a JSON object.";
    }
    const json &transferJson = *transferIt;
    WarnUnknownKeys(transferJson, TRANSFER_KNOWN_KEYS, "node '" + entry.uid + "'.transfer");

    const auto modeIt = transferJson.find(FIELD_MODE);
    if (modeIt != transferJson.end() && !modeIt->is_null())
    {
      if (!modeIt->is_string())
      {
        mitkThrow() << "Node '" << entry.uid << "': 'transfer.mode' must be a string.";
      }
      const std::string mode = modeIt->get<std::string>();
      if (mode != SUPPORTED_TRANSFER_MODE)
      {
        mitkThrow() << "Node '" << entry.uid << "': unsupported 'transfer.mode' value '" << mode
                    << "' (v1 supports only '" << SUPPORTED_TRANSFER_MODE << "').";
      }
    }

    const auto filePathIt = transferJson.find(FIELD_FILE_PATH);
    if (filePathIt == transferJson.end() || !filePathIt->is_string() || filePathIt->get<std::string>().empty())
    {
      mitkThrow() << "Node '" << entry.uid
                  << "': 'transfer.file_path' is required and must be a non-empty string.";
    }

    const fs::path dataPath = ResolvePath(basePath, filePathIt->get<std::string>());
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
    const std::string dataPropsCtx = "node '" + entry.uid + "'.data_properties";
    if (hasDataProps)
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
    catch (const mitk::Exception &e)
    {
      // Catch mitk::Exception before std::exception so that any future
      // type-aware handling (e.g. preserving GetTypeName/Description) has a
      // hook point. Rethrows wrap with node-level context; the message is
      // intentionally identical in shape to the std::exception arm below.
      mitkThrow() << "Node '" << entry.uid << "': failed to read data file '" << dataPath.string()
                  << "': " << e.what();
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

    // Informative-only: if the author declared data_type, compare against the
    // class the IO layer actually produced. A leading 'mitk::' on the declared
    // name is stripped so that the canonical REST form ('mitk::Image') matches
    // BaseData::GetNameOfClass() ('Image'). Mismatches are warnings, never
    // errors — data_type never drives loader dispatch.
    if (hasDataType)
    {
      const std::string declared = dataTypeIt->get<std::string>();
      const std::string observed = entry.dataNode->GetData()->GetNameOfClass();
      std::string declaredSuffix = declared;
      const std::string mitkNs("mitk::");
      if (declaredSuffix.compare(0, mitkNs.size(), mitkNs) == 0)
      {
        declaredSuffix = declaredSuffix.substr(mitkNs.size());
      }
      if (declaredSuffix != observed)
      {
        MITK_WARN << "Node '" << entry.uid << "': declared 'data_type' is '" << declared
                  << "' but the loaded object is of class '" << observed
                  << "'. 'data_type' is informative and did not influence loading.";
        nonFatalError = true;
      }
    }

    if (hasDataUid)
    {
      if (!dataUidIt->is_string())
      {
        mitkThrow() << "Node '" << entry.uid << "': 'data_uid' must be a string.";
      }
      const std::string dataUidValue = dataUidIt->get<std::string>();
      if (!seenDataUids.insert(dataUidValue).second)
      {
        MITK_WARN << "Node '" << entry.uid << "': 'data_uid' value '" << dataUidValue
                  << "' has already been assigned to another BaseData in this scene.";
      }
      UIDManipulator manip(entry.dataNode->GetData());
      manip.SetUID(dataUidValue);
    }

    // Transfer the pre-parsed data-level properties onto the loaded
    // BaseData's PropertyList. The same PropertyList was handed to
    // IOUtil::Load above as read-only hints; authoring it onto the loaded
    // object is what makes the properties persist. JSON is parsed once.
    //
    // Note: `replace` here clears the BaseData's PropertyList wholesale,
    // including keys populated by the file reader itself (e.g. DICOM tags).
    // That matches the author's declared intent of taking full ownership.
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

      // If a RenderingManager observes `storage`, mapper defaults are now in
      // place on the node; `replace` clears them and `modify` merges on top
      // of them. In headless contexts the property lists are still empty at
      // this point and the two styles coincide (see the Pass-1 note).
      const json &nodeJson = *entry.nodeJson;

      // Property-map application can still throw on semantic errors that
      // aren't checkable without I/O (bad `_file` target, nested `_file`,
      // invalid `_loadstyle`). We downgrade these to non-fatal per-node
      // errors so that storage is never left partially populated after a
      // node has already been added.
      const auto propsIt = nodeJson.find(FIELD_PROPERTIES);
      if (propsIt != nodeJson.end() && !propsIt->is_null())
      {
        const std::string ctx = "node '" + entry.uid + "'.properties";
        try
        {
          // GetPropertyList() returns the always-present default list constructed
          // by DataNode; null check intentionally omitted (the named-context call
          // below may legitimately be null and is checked there).
          PropertyList *defaultList = entry.dataNode->GetPropertyList();
          ApplyPropertyMap(*defaultList, *propsIt, basePath, ctx);
        }
        catch (const mitk::Exception &e)
        {
          MITK_ERROR << "Failed to apply " << ctx << ": " << e.what();
          nonFatalError = true;
        }
      }

      const auto ctxIt = nodeJson.find(FIELD_CONTEXT_PROPERTIES);
      if (ctxIt != nodeJson.end() && !ctxIt->is_null())
      {
        // Structural shape of `context_properties` was validated in step 3b
        // so Pass 2 never throws structurally after storage->Add.
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
          const std::string ctx = "node '" + entry.uid + "'.context_properties['" + contextName + "']";
          try
          {
            ApplyPropertyMap(*ctxList, cit.value(), basePath, ctx);
          }
          catch (const mitk::Exception &e)
          {
            MITK_ERROR << "Failed to apply " << ctx << ": " << e.what();
            nonFatalError = true;
          }
        }
      }
    }
  }

  return !nonFatalError;
}
