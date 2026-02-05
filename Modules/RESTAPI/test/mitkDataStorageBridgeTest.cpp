/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

#include "mitkDataStorageBridge.h"
#include "mitkNodeQueryParams.h"
#include "mitkNodeUidMapper.h"
#include <mitkStandaloneDataStorage.h>
#include <mitkStringProperty.h>
#include <mitkProperties.h>
#include <mitkColorProperty.h>
#include <mitkImage.h>
#include <mitkImageGenerator.h>
#include <mitkSurface.h>

#include <set>

/**
 * @brief Test fixture for DataStorageBridge with a complex default node hierarchy.
 *
 * Default setup creates the following hierarchy:
 *
 *   Root1 (no parent, no children)
 *   Root2 (no parent, has children)
 *     +-- Child1 (has parent, no children)
 *     +-- Child2 (has parent, has children)
 *           +-- Grandchild1 (has parent and grandparent)
 *           +-- Grandchild2 (has parent and grandparent)
 *   Root3 (no parent, has one child)
 *     +-- Child3 (has parent, no children)
 *
 * This setup allows testing:
 * - Root nodes (no parent): Root1, Root2, Root3
 * - Nodes with parent but no children: Child1, Child3
 * - Nodes with parent and children: Child2
 * - Leaf nodes at different depths: Child1, Grandchild1, Grandchild2, Child3
 * - Multi-child scenarios: Root2 has 2 children, Child2 has 2 children
 * - Single-child scenarios: Root3 has 1 child
 */
class mitkDataStorageBridgeTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDataStorageBridgeTestSuite);
  MITK_TEST(HasDataStorageReportsCorrectly);
  MITK_TEST(GetNodesReturnsAllNodes);
  MITK_TEST(GetNodesWithPagination);
  MITK_TEST(GetNodeByUid);
  MITK_TEST(GetRootNodeHasNoParent);
  MITK_TEST(GetChildNodeHasParent);
  MITK_TEST(GetNodeWithChildrenReportsChildrenCount);
  MITK_TEST(GetRootNodesOnly);
  MITK_TEST(GetChildrenOfParent);
  MITK_TEST(GetChildrenOfNodeWithNoChildren);
  MITK_TEST(CreateNodeAtRoot);
  MITK_TEST(CreateNodeWithParent);
  MITK_TEST(CreateNodeWithProperties);
  MITK_TEST(CreateNodeWithFailedProperties);
  MITK_TEST(UpdateNodeReparentToAnotherParent);
  MITK_TEST(UpdateNodeReparentToRoot);
  MITK_TEST(UpdateNodeReparentFromRootToParent);
  MITK_TEST(DeleteLeafNode);
  MITK_TEST(DeleteNodeWithChildrenNonRecursive);
  MITK_TEST(DeleteNodeWithChildrenRecursive);
  MITK_TEST(DeleteRootNodeWithSubtree);
  // Data operations
  MITK_TEST(GetNodeDataReturnsClone);
  MITK_TEST(GetNodeDataWithNoData);
  MITK_TEST(GetNodeDataWithNonExistentNode);
  MITK_TEST(SetNodeDataToExistingNode);
  MITK_TEST(SetNodeDataToEmptyNode);
  MITK_TEST(SetNodeDataClearsData);
  MITK_TEST(SetNodeDataWithNonExistentNode);
  MITK_TEST(SetNodeDataSetsModificationTracking);
  MITK_TEST(GetNodeProperties);
  MITK_TEST(GetNodePropertiesWithScope);
  MITK_TEST(GetNodePropertiesWithFiltering);
  MITK_TEST(GetNodePropertiesNamesOnly);
  MITK_TEST(GetNodePropertyWithScopeAndValue);
  MITK_TEST(SetNodeProperty);
  MITK_TEST(DeleteNodeProperty);
  MITK_TEST(ReplaceNodeProperties);
  MITK_TEST(ComplexPropertySerializationRoundTrip);
  MITK_TEST(NameFilterWithExactMatch);
  MITK_TEST(NameFilterWithWildcard);
  MITK_TEST(BoolPropertyFilterUsesJsonRepresentation);
  MITK_TEST(NegatedPropertyFilter);
  MITK_TEST(TimestampIsInteger);
  MITK_TEST(GetNodeAvailableContexts);
  MITK_TEST(SortByTimestamp);
  MITK_TEST(SortByName);
  // Internal property tests
  MITK_TEST(InternalPropertiesNotLeakedInGetProperties);
  MITK_TEST(InternalPropertyCannotBeAccessed);
  MITK_TEST(InternalPropertyCannotBeSet);
  MITK_TEST(InternalPropertyCannotBeDeleted);
  MITK_TEST(ReplacePropertiesPreservesInternalProperties);
  // Modification tracking tests
  MITK_TEST(CreateNodeSetsModificationTracking);
  MITK_TEST(UpdateNodeSetsModificationTracking);
  MITK_TEST(SetNodePropertySetsModificationTracking);
  MITK_TEST(DeleteNodePropertySetsModificationTracking);
  MITK_TEST(ReplaceNodePropertiesSetsModificationTracking);
  MITK_TEST(ReadOperationsDoNotSetModificationTracking);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  std::unique_ptr<mitk::DataStorageBridge> m_Bridge;

  // Node references for the default hierarchy
  mitk::DataNode::Pointer m_Root1;
  mitk::DataNode::Pointer m_Root2;
  mitk::DataNode::Pointer m_Root3;
  mitk::DataNode::Pointer m_Child1;
  mitk::DataNode::Pointer m_Child2;
  mitk::DataNode::Pointer m_Child3;
  mitk::DataNode::Pointer m_Grandchild1;
  mitk::DataNode::Pointer m_Grandchild2;

  // UIDs for quick access
  std::string m_Root1Uid;
  std::string m_Root2Uid;
  std::string m_Root3Uid;
  std::string m_Child1Uid;
  std::string m_Child2Uid;
  std::string m_Child3Uid;
  std::string m_Grandchild1Uid;
  std::string m_Grandchild2Uid;

  /**
   * @brief Helper to find a node's UID by querying with a name filter.
   *
   * This avoids needing direct UID access methods on the bridge.
   */
  std::string FindUidByName(const std::string& name)
  {
    mitk::NodeQueryParams params;
    params.propertyFilters.push_back({"name", name, false});
    auto result = m_Bridge->GetNodes(params);
    if (result.nodes.empty())
    {
      throw std::runtime_error("Node not found: " + name);
    }
    return result.nodes[0]["uid"].get<std::string>();
  }

  void SetupDefaultHierarchy()
  {
    // Root1 - standalone root node
    m_Root1 = mitk::DataNode::New();
    m_Root1->SetName("Root1");
    m_Root1->SetBoolProperty("visible", true);
    m_DataStorage->Add(m_Root1);

    // Root2 - root with children
    m_Root2 = mitk::DataNode::New();
    m_Root2->SetName("Root2");
    m_Root2->SetBoolProperty("visible", true);
    m_DataStorage->Add(m_Root2);

    // Child1 - child of Root2, no children
    m_Child1 = mitk::DataNode::New();
    m_Child1->SetName("Child1");
    m_Child1->SetBoolProperty("visible", true);
    m_DataStorage->Add(m_Child1, m_Root2.GetPointer());

    // Child2 - child of Root2, has children
    m_Child2 = mitk::DataNode::New();
    m_Child2->SetName("Child2");
    m_Child2->SetBoolProperty("visible", false);
    m_DataStorage->Add(m_Child2, m_Root2.GetPointer());

    // Grandchild1 - child of Child2
    m_Grandchild1 = mitk::DataNode::New();
    m_Grandchild1->SetName("Grandchild1");
    m_Grandchild1->SetBoolProperty("visible", true);
    m_DataStorage->Add(m_Grandchild1, m_Child2.GetPointer());

    // Grandchild2 - child of Child2
    m_Grandchild2 = mitk::DataNode::New();
    m_Grandchild2->SetName("Grandchild2");
    m_Grandchild2->SetBoolProperty("visible", true);
    m_DataStorage->Add(m_Grandchild2, m_Child2.GetPointer());

    // Root3 - root with one child
    m_Root3 = mitk::DataNode::New();
    m_Root3->SetName("Root3");
    m_Root3->SetBoolProperty("visible", true);
    m_DataStorage->Add(m_Root3);

    // Child3 - only child of Root3
    m_Child3 = mitk::DataNode::New();
    m_Child3->SetName("Child3");
    m_Child3->SetBoolProperty("visible", true);
    m_DataStorage->Add(m_Child3, m_Root3.GetPointer());

    // Query UIDs by name (uses the bridge's public API)
    m_Root1Uid = FindUidByName("Root1");
    m_Root2Uid = FindUidByName("Root2");
    m_Root3Uid = FindUidByName("Root3");
    m_Child1Uid = FindUidByName("Child1");
    m_Child2Uid = FindUidByName("Child2");
    m_Child3Uid = FindUidByName("Child3");
    m_Grandchild1Uid = FindUidByName("Grandchild1");
    m_Grandchild2Uid = FindUidByName("Grandchild2");
  }

  void ClearStorage()
  {
    auto all = m_DataStorage->GetAll();
    for (auto it = all->Begin(); it != all->End(); ++it)
    {
      m_DataStorage->Remove(it->Value());
    }
  }

public:
  void setUp() override
  {
    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Bridge = std::make_unique<mitk::DataStorageBridge>();
    m_Bridge->SetDataStorage(m_DataStorage);

    SetupDefaultHierarchy();
  }

  void tearDown() override
  {
    m_Bridge->SetDataStorage(nullptr);
    m_Bridge.reset();
    m_DataStorage = nullptr;
  }

  void HasDataStorageReportsCorrectly()
  {
    CPPUNIT_ASSERT(m_Bridge->HasDataStorage());

    m_Bridge->SetDataStorage(nullptr);
    CPPUNIT_ASSERT(!m_Bridge->HasDataStorage());
  }

  void GetNodesReturnsAllNodes()
  {
    mitk::NodeQueryParams params;
    auto result = m_Bridge->GetNodes(params);

    // Should return all 8 nodes
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), result.nodes.size());
    CPPUNIT_ASSERT_EQUAL(8, result.totalCount);

    // Verify all 8 nodes are unique by collecting UIDs
    std::set<std::string> uniqueUids;
    for (const auto& nodeJson : result.nodes)
    {
      uniqueUids.insert(nodeJson["uid"].get<std::string>());
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), uniqueUids.size());

    // Verify all expected nodes are present
    CPPUNIT_ASSERT(uniqueUids.count(m_Root1Uid) == 1);
    CPPUNIT_ASSERT(uniqueUids.count(m_Root2Uid) == 1);
    CPPUNIT_ASSERT(uniqueUids.count(m_Root3Uid) == 1);
    CPPUNIT_ASSERT(uniqueUids.count(m_Child1Uid) == 1);
    CPPUNIT_ASSERT(uniqueUids.count(m_Child2Uid) == 1);
    CPPUNIT_ASSERT(uniqueUids.count(m_Child3Uid) == 1);
    CPPUNIT_ASSERT(uniqueUids.count(m_Grandchild1Uid) == 1);
    CPPUNIT_ASSERT(uniqueUids.count(m_Grandchild2Uid) == 1);
  }

  void GetNodesWithPagination()
  {
    // Collect all UIDs from paginated results to verify:
    // 1. All pages return unique nodes
    // 2. All 8 nodes are covered across all pages
    std::set<std::string> allUids;

    // Get first 3 nodes
    mitk::NodeQueryParams params;
    params.limit = 3;
    params.offset = 0;

    auto result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), result.nodes.size());
    CPPUNIT_ASSERT_EQUAL(8, result.totalCount);
    CPPUNIT_ASSERT_EQUAL(3, result.limit);
    CPPUNIT_ASSERT_EQUAL(0, result.offset);

    for (const auto& node : result.nodes)
    {
      allUids.insert(node["uid"].get<std::string>());
    }

    // Get next 3 nodes
    params.offset = 3;
    result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), result.nodes.size());
    CPPUNIT_ASSERT_EQUAL(8, result.totalCount);

    for (const auto& node : result.nodes)
    {
      allUids.insert(node["uid"].get<std::string>());
    }

    // Get last 2 nodes
    params.offset = 6;
    result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.nodes.size());
    CPPUNIT_ASSERT_EQUAL(8, result.totalCount);

    for (const auto& node : result.nodes)
    {
      allUids.insert(node["uid"].get<std::string>());
    }

    // Verify all 8 unique nodes were returned across all pages
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), allUids.size());

    // Verify all expected nodes are present
    CPPUNIT_ASSERT(allUids.count(m_Root1Uid) == 1);
    CPPUNIT_ASSERT(allUids.count(m_Root2Uid) == 1);
    CPPUNIT_ASSERT(allUids.count(m_Root3Uid) == 1);
    CPPUNIT_ASSERT(allUids.count(m_Child1Uid) == 1);
    CPPUNIT_ASSERT(allUids.count(m_Child2Uid) == 1);
    CPPUNIT_ASSERT(allUids.count(m_Child3Uid) == 1);
    CPPUNIT_ASSERT(allUids.count(m_Grandchild1Uid) == 1);
    CPPUNIT_ASSERT(allUids.count(m_Grandchild2Uid) == 1);
  }

  void GetNodeByUid()
  {
    // Get a specific node by UID
    auto nodeResult = m_Bridge->GetNode(m_Child2Uid);
    CPPUNIT_ASSERT(nodeResult.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("Child2"), nodeResult.value()["name"].get<std::string>());
  }

  void GetRootNodeHasNoParent()
  {
    // Root1 should have no parent
    auto nodeResult = m_Bridge->GetNode(m_Root1Uid);
    CPPUNIT_ASSERT(nodeResult.has_value());
    CPPUNIT_ASSERT(nodeResult.value()["parent_uid"].is_null());

    // Verify it's actually Root1 by checking name and UID
    CPPUNIT_ASSERT_EQUAL(std::string("Root1"), nodeResult.value()["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(m_Root1Uid, nodeResult.value()["uid"].get<std::string>());

    // Double-check in DataStorage that Root1 has no parent
    auto sources = m_DataStorage->GetSources(m_Root1);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), sources->Size());
  }

  void GetChildNodeHasParent()
  {
    // Child1 should have Root2 as parent
    auto nodeResult = m_Bridge->GetNode(m_Child1Uid);
    CPPUNIT_ASSERT(nodeResult.has_value());
    CPPUNIT_ASSERT_EQUAL(m_Root2Uid, nodeResult.value()["parent_uid"].get<std::string>());

    // Verify it's actually Child1
    CPPUNIT_ASSERT_EQUAL(std::string("Child1"), nodeResult.value()["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(m_Child1Uid, nodeResult.value()["uid"].get<std::string>());

    // Double-check in DataStorage
    auto sources = m_DataStorage->GetSources(m_Child1);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), sources->Size());
    CPPUNIT_ASSERT(sources->front() == m_Root2.GetPointer());

    // Grandchild1 should have Child2 as parent
    nodeResult = m_Bridge->GetNode(m_Grandchild1Uid);
    CPPUNIT_ASSERT(nodeResult.has_value());
    CPPUNIT_ASSERT_EQUAL(m_Child2Uid, nodeResult.value()["parent_uid"].get<std::string>());

    // Verify it's actually Grandchild1
    CPPUNIT_ASSERT_EQUAL(std::string("Grandchild1"), nodeResult.value()["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(m_Grandchild1Uid, nodeResult.value()["uid"].get<std::string>());

    // Double-check in DataStorage
    sources = m_DataStorage->GetSources(m_Grandchild1);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), sources->Size());
    CPPUNIT_ASSERT(sources->front() == m_Child2.GetPointer());
  }

  void GetNodeWithChildrenReportsChildrenCount()
  {
    // Root2 should have 2 children
    auto nodeResult = m_Bridge->GetNode(m_Root2Uid);
    CPPUNIT_ASSERT(nodeResult.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("Root2"), nodeResult.value()["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(2, nodeResult.value()["children_count"].get<int>());

    // Verify in DataStorage
    auto children = m_DataStorage->GetDerivations(m_Root2);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), children->Size());

    // Child2 should have 2 children
    nodeResult = m_Bridge->GetNode(m_Child2Uid);
    CPPUNIT_ASSERT(nodeResult.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("Child2"), nodeResult.value()["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(2, nodeResult.value()["children_count"].get<int>());

    children = m_DataStorage->GetDerivations(m_Child2);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), children->Size());

    // Root3 should have 1 child
    nodeResult = m_Bridge->GetNode(m_Root3Uid);
    CPPUNIT_ASSERT(nodeResult.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("Root3"), nodeResult.value()["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(1, nodeResult.value()["children_count"].get<int>());

    children = m_DataStorage->GetDerivations(m_Root3);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), children->Size());

    // Root1 should have 0 children
    nodeResult = m_Bridge->GetNode(m_Root1Uid);
    CPPUNIT_ASSERT(nodeResult.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("Root1"), nodeResult.value()["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(0, nodeResult.value()["children_count"].get<int>());

    children = m_DataStorage->GetDerivations(m_Root1);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), children->Size());

    // Grandchild1 (leaf) should have 0 children
    nodeResult = m_Bridge->GetNode(m_Grandchild1Uid);
    CPPUNIT_ASSERT(nodeResult.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("Grandchild1"), nodeResult.value()["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(0, nodeResult.value()["children_count"].get<int>());

    children = m_DataStorage->GetDerivations(m_Grandchild1);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), children->Size());
  }

  void GetRootNodesOnly()
  {
    // Use Hierarchy::Toplevel to get only root nodes
    mitk::NodeQueryParams params;
    params.hierarchy = mitk::Hierarchy::Toplevel;

    auto result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), result.nodes.size());
    CPPUNIT_ASSERT_EQUAL(3, result.totalCount);

    // Collect returned UIDs to verify exactly Root1, Root2, Root3 are returned
    std::set<std::string> returnedUids;
    for (const auto& nodeJson : result.nodes)
    {
      CPPUNIT_ASSERT(nodeJson["parent_uid"].is_null());
      returnedUids.insert(nodeJson["uid"].get<std::string>());
    }

    // Verify exactly the expected root nodes are returned
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), returnedUids.size());
    CPPUNIT_ASSERT(returnedUids.count(m_Root1Uid) == 1);
    CPPUNIT_ASSERT(returnedUids.count(m_Root2Uid) == 1);
    CPPUNIT_ASSERT(returnedUids.count(m_Root3Uid) == 1);

    // Verify Child nodes are NOT in the results
    CPPUNIT_ASSERT(returnedUids.count(m_Child1Uid) == 0);
    CPPUNIT_ASSERT(returnedUids.count(m_Child2Uid) == 0);
    CPPUNIT_ASSERT(returnedUids.count(m_Child3Uid) == 0);
    CPPUNIT_ASSERT(returnedUids.count(m_Grandchild1Uid) == 0);
    CPPUNIT_ASSERT(returnedUids.count(m_Grandchild2Uid) == 0);

    // Verify using node pointers in DataStorage: Root1, Root2, Root3 should have no sources
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), m_DataStorage->GetSources(m_Root1)->Size());
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), m_DataStorage->GetSources(m_Root2)->Size());
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), m_DataStorage->GetSources(m_Root3)->Size());
  }

  void GetChildrenOfParent()
  {
    // Get children of Root2
    mitk::NodeQueryParams params;
    params.parentUid = m_Root2Uid;

    auto result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.nodes.size());
    CPPUNIT_ASSERT_EQUAL(2, result.totalCount);

    // Get children of Child2
    params.parentUid = m_Child2Uid;
    result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.nodes.size());

    // Get children of Root3 (single child)
    params.parentUid = m_Root3Uid;
    result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), result.nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("Child3"), result.nodes[0]["name"].get<std::string>());
  }

  void GetChildrenOfNodeWithNoChildren()
  {
    // Root1 has no children
    mitk::NodeQueryParams params;
    params.parentUid = m_Root1Uid;

    auto result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT(result.nodes.empty());
    CPPUNIT_ASSERT_EQUAL(0, result.totalCount);

    // Grandchild1 (leaf) has no children
    params.parentUid = m_Grandchild1Uid;
    result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT(result.nodes.empty());
    CPPUNIT_ASSERT_EQUAL(0, result.totalCount);
  }

  void CreateNodeAtRoot()
  {
    nlohmann::json nodeData;
    nodeData["name"] = "NewRootNode";

    auto result = m_Bridge->CreateNode(nodeData);
    CPPUNIT_ASSERT(result.success);
    CPPUNIT_ASSERT(!result.uid.empty());

    // Verify node is at root level (no parent)
    auto node = m_Bridge->GetNode(result.uid);
    CPPUNIT_ASSERT(node.has_value());
    CPPUNIT_ASSERT(node.value()["parent_uid"].is_null());
  }

  void CreateNodeWithParent()
  {
    nlohmann::json nodeData;
    nodeData["name"] = "NewChildOfRoot1";

    auto result = m_Bridge->CreateNode(nodeData, m_Root1Uid);
    CPPUNIT_ASSERT(result.success);

    // Verify node has correct parent
    auto node = m_Bridge->GetNode(result.uid);
    CPPUNIT_ASSERT(node.has_value());
    CPPUNIT_ASSERT_EQUAL(m_Root1Uid, node.value()["parent_uid"].get<std::string>());

    // Verify Root1 now has a child
    auto root1 = m_Bridge->GetNode(m_Root1Uid);
    CPPUNIT_ASSERT(root1.has_value());
    CPPUNIT_ASSERT_EQUAL(1, root1.value()["children_count"].get<int>());
  }

  void CreateNodeWithProperties()
  {
    nlohmann::json nodeData;
    nodeData["name"] = "NodeWithProps";
    nodeData["properties"]["visible"] = true;
    nodeData["properties"]["opacity"] = 0.5;

    auto result = m_Bridge->CreateNode(nodeData);
    CPPUNIT_ASSERT(result.success);
    CPPUNIT_ASSERT(result.failedProperties.empty());

    // Verify properties via bridge API
    auto props = m_Bridge->GetNodeProperties(result.uid);
    CPPUNIT_ASSERT(props.has_value());
    CPPUNIT_ASSERT(props.value().contains("visible"));
    CPPUNIT_ASSERT(props.value().contains("opacity"));

    // Verify the actual property values match what was set
    CPPUNIT_ASSERT_EQUAL(true, props.value()["visible"].get<bool>());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, props.value()["opacity"].get<double>(), 0.001);

    // Also verify directly on the node in DataStorage
    mitk::DataNode* createdNode = nullptr;
    auto allNodes = m_DataStorage->GetAll();
    for (auto it = allNodes->Begin(); it != allNodes->End(); ++it)
    {
      if (it->Value()->GetName() == "NodeWithProps")
      {
        createdNode = it->Value();
        break;
      }
    }
    CPPUNIT_ASSERT(createdNode != nullptr);

    bool visibleValue = false;
    CPPUNIT_ASSERT(createdNode->GetBoolProperty("visible", visibleValue));
    CPPUNIT_ASSERT_EQUAL(true, visibleValue);

    float opacityValue = 0.0f;
    CPPUNIT_ASSERT(createdNode->GetFloatProperty("opacity", opacityValue));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5f, opacityValue, 0.001f);
  }

  void CreateNodeWithFailedProperties()
  {
    nlohmann::json nodeData;
    nodeData["name"] = "NodeWithBadProps";
    nodeData["properties"]["good_prop"] = true;
    // Invalid property value that can't be deserialized
    nodeData["properties"]["bad_prop"] = nlohmann::json::object({{"invalid_format", "test"}});

    auto result = m_Bridge->CreateNode(nodeData);

    // Node should still be created even with failed properties
    CPPUNIT_ASSERT(result.success);
    CPPUNIT_ASSERT(!result.uid.empty());

    // Should report the failed property
    CPPUNIT_ASSERT(!result.failedProperties.empty());
    CPPUNIT_ASSERT(std::find(result.failedProperties.begin(),
                              result.failedProperties.end(),
                              "bad_prop") != result.failedProperties.end());

    // Good property should have been set
    auto props = m_Bridge->GetNodeProperties(result.uid);
    CPPUNIT_ASSERT(props.has_value());
    CPPUNIT_ASSERT(props.value().contains("good_prop"));
  }

  void UpdateNodeReparentToAnotherParent()
  {
    // Move Child1 from Root2 to Root3
    nlohmann::json updates;
    updates["parent_uid"] = m_Root3Uid;

    bool success = m_Bridge->UpdateNode(m_Child1Uid, updates);
    CPPUNIT_ASSERT(success);

    // Verify Child1 is now under Root3 (check DataStorage directly)
    auto child1Sources = m_DataStorage->GetSources(m_Child1);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), child1Sources->Size());
    CPPUNIT_ASSERT(child1Sources->front() == m_Root3.GetPointer());

    // Verify Root2 now has only 1 child (Child2)
    auto root2Children = m_DataStorage->GetDerivations(m_Root2);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), root2Children->Size());

    // Verify Root3 now has 2 children (Child3 and Child1)
    auto root3Children = m_DataStorage->GetDerivations(m_Root3);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), root3Children->Size());
  }

  void UpdateNodeReparentToRoot()
  {
    // Move Child1 to root level (no parent)
    nlohmann::json updates;
    updates["parent_uid"] = nullptr;

    bool success = m_Bridge->UpdateNode(m_Child1Uid, updates);
    CPPUNIT_ASSERT(success);

    // Verify Child1 is now at root level (no sources)
    auto child1Sources = m_DataStorage->GetSources(m_Child1);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), child1Sources->Size());

    // Verify Root2 now has only 1 child (Child2)
    auto root2Children = m_DataStorage->GetDerivations(m_Root2);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), root2Children->Size());
  }

  void UpdateNodeReparentFromRootToParent()
  {
    // Move Root1 (currently a root) to be a child of Root2
    nlohmann::json updates;
    updates["parent_uid"] = m_Root2Uid;

    bool success = m_Bridge->UpdateNode(m_Root1Uid, updates);
    CPPUNIT_ASSERT(success);

    // Verify Root1 is now under Root2
    auto root1Sources = m_DataStorage->GetSources(m_Root1);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), root1Sources->Size());
    CPPUNIT_ASSERT(root1Sources->front() == m_Root2.GetPointer());

    // Verify Root2 now has 3 children (Child1, Child2, Root1)
    auto root2Children = m_DataStorage->GetDerivations(m_Root2);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), root2Children->Size());
  }

  void DeleteLeafNode()
  {
    // Remember the node count before deletion
    auto allBefore = m_DataStorage->GetAll();
    unsigned int countBefore = allBefore->Size();
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(8), countBefore);

    // Delete Grandchild1 (a leaf node)
    auto deleteResult = m_Bridge->DeleteNode(m_Grandchild1Uid);
    CPPUNIT_ASSERT(deleteResult.success);
    CPPUNIT_ASSERT_EQUAL(m_Grandchild1Uid, deleteResult.deletedUid);
    CPPUNIT_ASSERT(deleteResult.deletedChildren.empty());

    // Verify node is gone - check directly in DataStorage
    auto allAfter = m_DataStorage->GetAll();
    CPPUNIT_ASSERT_EQUAL(countBefore - 1, allAfter->Size());

    // Verify Grandchild1 is not in DataStorage
    bool foundGrandchild1 = false;
    for (auto it = allAfter->Begin(); it != allAfter->End(); ++it)
    {
      if (it->Value() == m_Grandchild1.GetPointer())
      {
        foundGrandchild1 = true;
        break;
      }
    }
    CPPUNIT_ASSERT(!foundGrandchild1);

    // Verify other nodes are still present in DataStorage
    bool foundRoot1 = false, foundRoot2 = false, foundRoot3 = false;
    bool foundChild1 = false, foundChild2 = false, foundChild3 = false;
    bool foundGrandchild2 = false;
    for (auto it = allAfter->Begin(); it != allAfter->End(); ++it)
    {
      if (it->Value() == m_Root1.GetPointer()) foundRoot1 = true;
      if (it->Value() == m_Root2.GetPointer()) foundRoot2 = true;
      if (it->Value() == m_Root3.GetPointer()) foundRoot3 = true;
      if (it->Value() == m_Child1.GetPointer()) foundChild1 = true;
      if (it->Value() == m_Child2.GetPointer()) foundChild2 = true;
      if (it->Value() == m_Child3.GetPointer()) foundChild3 = true;
      if (it->Value() == m_Grandchild2.GetPointer()) foundGrandchild2 = true;
    }
    CPPUNIT_ASSERT(foundRoot1 && foundRoot2 && foundRoot3);
    CPPUNIT_ASSERT(foundChild1 && foundChild2 && foundChild3);
    CPPUNIT_ASSERT(foundGrandchild2);

    // Verify Child2's children count via DataStorage
    auto child2Children = m_DataStorage->GetDerivations(m_Child2);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), child2Children->Size());
  }

  void DeleteNodeWithChildrenNonRecursive()
  {
    // Remember the node count before deletion attempt
    auto allBefore = m_DataStorage->GetAll();
    unsigned int countBefore = allBefore->Size();

    // Try to delete Child2 (has 2 children) without recursive - should fail
    auto deleteResult = m_Bridge->DeleteNode(m_Child2Uid, false);
    CPPUNIT_ASSERT(!deleteResult.success);
    CPPUNIT_ASSERT_EQUAL(2, deleteResult.childrenCount);

    // Verify NO nodes were deleted - DataStorage should be unchanged
    auto allAfter = m_DataStorage->GetAll();
    CPPUNIT_ASSERT_EQUAL(countBefore, allAfter->Size());

    // Verify Child2 is still in DataStorage
    bool foundChild2 = false;
    for (auto it = allAfter->Begin(); it != allAfter->End(); ++it)
    {
      if (it->Value() == m_Child2.GetPointer())
      {
        foundChild2 = true;
        break;
      }
    }
    CPPUNIT_ASSERT(foundChild2);

    // Verify its children are also still present
    auto children = m_DataStorage->GetDerivations(m_Child2);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), children->Size());
  }

  void DeleteNodeWithChildrenRecursive()
  {
    // Remember initial state
    auto allBefore = m_DataStorage->GetAll();
    unsigned int countBefore = allBefore->Size();
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(8), countBefore);

    // Delete Child2 with recursive flag - should delete Child2 and its 2 grandchildren
    auto deleteResult = m_Bridge->DeleteNode(m_Child2Uid, true);
    CPPUNIT_ASSERT(deleteResult.success);
    CPPUNIT_ASSERT_EQUAL(m_Child2Uid, deleteResult.deletedUid);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), deleteResult.deletedChildren.size());

    // Verify exactly 3 nodes were deleted (Child2, Grandchild1, Grandchild2)
    auto allAfter = m_DataStorage->GetAll();
    CPPUNIT_ASSERT_EQUAL(countBefore - 3, allAfter->Size());

    // Verify the deleted nodes are NOT in DataStorage
    bool foundChild2 = false, foundGrandchild1 = false, foundGrandchild2 = false;
    // Verify the remaining nodes ARE in DataStorage
    bool foundRoot1 = false, foundRoot2 = false, foundRoot3 = false;
    bool foundChild1 = false, foundChild3 = false;

    for (auto it = allAfter->Begin(); it != allAfter->End(); ++it)
    {
      if (it->Value() == m_Child2.GetPointer()) foundChild2 = true;
      if (it->Value() == m_Grandchild1.GetPointer()) foundGrandchild1 = true;
      if (it->Value() == m_Grandchild2.GetPointer()) foundGrandchild2 = true;
      if (it->Value() == m_Root1.GetPointer()) foundRoot1 = true;
      if (it->Value() == m_Root2.GetPointer()) foundRoot2 = true;
      if (it->Value() == m_Root3.GetPointer()) foundRoot3 = true;
      if (it->Value() == m_Child1.GetPointer()) foundChild1 = true;
      if (it->Value() == m_Child3.GetPointer()) foundChild3 = true;
    }

    // Deleted nodes should NOT be found
    CPPUNIT_ASSERT(!foundChild2);
    CPPUNIT_ASSERT(!foundGrandchild1);
    CPPUNIT_ASSERT(!foundGrandchild2);

    // Remaining nodes SHOULD be found
    CPPUNIT_ASSERT(foundRoot1 && foundRoot2 && foundRoot3);
    CPPUNIT_ASSERT(foundChild1 && foundChild3);

    // Verify Root2 now has only 1 child (Child1) in DataStorage
    auto root2Children = m_DataStorage->GetDerivations(m_Root2);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), root2Children->Size());
    CPPUNIT_ASSERT(root2Children->front() == m_Child1.GetPointer());
  }

  void DeleteRootNodeWithSubtree()
  {
    // Remember initial state
    auto allBefore = m_DataStorage->GetAll();
    unsigned int countBefore = allBefore->Size();
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(8), countBefore);

    // Delete Root2 with recursive - should delete Root2, Child1, Child2, Grandchild1, Grandchild2
    auto deleteResult = m_Bridge->DeleteNode(m_Root2Uid, true);
    CPPUNIT_ASSERT(deleteResult.success);
    CPPUNIT_ASSERT_EQUAL(m_Root2Uid, deleteResult.deletedUid);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), deleteResult.deletedChildren.size());

    // Verify exactly 5 nodes were deleted
    auto allAfter = m_DataStorage->GetAll();
    CPPUNIT_ASSERT_EQUAL(countBefore - 5, allAfter->Size());
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), allAfter->Size());

    // Verify the deleted nodes are NOT in DataStorage
    bool foundRoot2 = false, foundChild1 = false, foundChild2 = false;
    bool foundGrandchild1 = false, foundGrandchild2 = false;
    // Verify the remaining nodes ARE in DataStorage
    bool foundRoot1 = false, foundRoot3 = false, foundChild3 = false;

    for (auto it = allAfter->Begin(); it != allAfter->End(); ++it)
    {
      if (it->Value() == m_Root2.GetPointer()) foundRoot2 = true;
      if (it->Value() == m_Child1.GetPointer()) foundChild1 = true;
      if (it->Value() == m_Child2.GetPointer()) foundChild2 = true;
      if (it->Value() == m_Grandchild1.GetPointer()) foundGrandchild1 = true;
      if (it->Value() == m_Grandchild2.GetPointer()) foundGrandchild2 = true;
      if (it->Value() == m_Root1.GetPointer()) foundRoot1 = true;
      if (it->Value() == m_Root3.GetPointer()) foundRoot3 = true;
      if (it->Value() == m_Child3.GetPointer()) foundChild3 = true;
    }

    // Deleted nodes should NOT be found
    CPPUNIT_ASSERT(!foundRoot2);
    CPPUNIT_ASSERT(!foundChild1);
    CPPUNIT_ASSERT(!foundChild2);
    CPPUNIT_ASSERT(!foundGrandchild1);
    CPPUNIT_ASSERT(!foundGrandchild2);

    // Remaining nodes SHOULD be found
    CPPUNIT_ASSERT(foundRoot1);
    CPPUNIT_ASSERT(foundRoot3);
    CPPUNIT_ASSERT(foundChild3);

    // Verify hierarchy of remaining nodes
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), m_DataStorage->GetSources(m_Root1)->Size());
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), m_DataStorage->GetSources(m_Root3)->Size());
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), m_DataStorage->GetSources(m_Child3)->Size());
    CPPUNIT_ASSERT(m_DataStorage->GetSources(m_Child3)->front() == m_Root3.GetPointer());
  }

  // ===== Data operation tests =====

  void GetNodeDataReturnsClone()
  {
    // Create a node with image data
    auto nodeWithData = mitk::DataNode::New();
    nodeWithData->SetName("NodeWithImageData");

    auto originalImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(10, 10, 10);
    nodeWithData->SetData(originalImage);
    m_DataStorage->Add(nodeWithData);

    auto nodeUid = FindUidByName("NodeWithImageData");

    // Get a clone of the data
    auto result = m_Bridge->GetNodeData(nodeUid);

    // Verify node was found and data was returned
    CPPUNIT_ASSERT(result.nodeFound);
    CPPUNIT_ASSERT(result.data.IsNotNull());

    // Verify it's an Image
    auto* imageClone = dynamic_cast<mitk::Image*>(result.data.GetPointer());
    CPPUNIT_ASSERT(imageClone != nullptr);

    // Verify the clone is a DIFFERENT object (not the same pointer)
    CPPUNIT_ASSERT(imageClone != originalImage.GetPointer());

    // Verify clone has the same dimensions
    CPPUNIT_ASSERT_EQUAL(originalImage->GetDimension(), imageClone->GetDimension());
    for (unsigned int i = 0; i < originalImage->GetDimension(); ++i)
    {
      CPPUNIT_ASSERT_EQUAL(originalImage->GetDimension(i), imageClone->GetDimension(i));
    }
  }

  void GetNodeDataWithNoData()
  {
    // Root1 has no data attached
    auto result = m_Bridge->GetNodeData(m_Root1Uid);

    // Node should be found, but data should be nullptr
    CPPUNIT_ASSERT(result.nodeFound);
    CPPUNIT_ASSERT(result.data.IsNull());
  }

  void GetNodeDataWithNonExistentNode()
  {
    // Non-existent node
    auto result = m_Bridge->GetNodeData("non-existent-uid");

    // Node should NOT be found
    CPPUNIT_ASSERT(!result.nodeFound);
    CPPUNIT_ASSERT(result.data.IsNull());
  }

  void SetNodeDataToExistingNode()
  {
    // Create a node with image data
    auto nodeWithData = mitk::DataNode::New();
    nodeWithData->SetName("NodeForDataReplacement");

    auto originalImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(5, 5, 5);
    nodeWithData->SetData(originalImage);
    m_DataStorage->Add(nodeWithData);

    auto nodeUid = FindUidByName("NodeForDataReplacement");

    // Create new image with different dimensions
    auto newImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(8, 8, 8);

    // Replace the data
    bool success = m_Bridge->SetNodeData(nodeUid, newImage);
    CPPUNIT_ASSERT(success);

    // Verify the data was replaced
    auto* currentData = nodeWithData->GetData();
    CPPUNIT_ASSERT(currentData != nullptr);
    CPPUNIT_ASSERT(currentData == newImage.GetPointer());

    // Verify dimensions changed
    auto* currentImage = dynamic_cast<mitk::Image*>(currentData);
    CPPUNIT_ASSERT(currentImage != nullptr);
    CPPUNIT_ASSERT_EQUAL(8u, currentImage->GetDimension(0));
  }

  void SetNodeDataToEmptyNode()
  {
    // Root1 has no data - set data on it
    auto newImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(7, 7, 7);

    bool success = m_Bridge->SetNodeData(m_Root1Uid, newImage);
    CPPUNIT_ASSERT(success);

    // Verify the data was set
    auto* currentData = m_Root1->GetData();
    CPPUNIT_ASSERT(currentData != nullptr);
    CPPUNIT_ASSERT(currentData == newImage.GetPointer());

    // Verify via GetNode that data_type is now set
    auto nodeJson = m_Bridge->GetNode(m_Root1Uid);
    CPPUNIT_ASSERT(nodeJson.has_value());
    CPPUNIT_ASSERT(!nodeJson.value()["data_type"].is_null());
    CPPUNIT_ASSERT_EQUAL(std::string("mitk::Image"), nodeJson.value()["data_type"].get<std::string>());
  }

  void SetNodeDataClearsData()
  {
    // Create a node with data
    auto nodeWithData = mitk::DataNode::New();
    nodeWithData->SetName("NodeForDataClearing");
    nodeWithData->SetData(mitk::ImageGenerator::GenerateRandomImage<unsigned char>(5, 5, 5));
    m_DataStorage->Add(nodeWithData);

    auto nodeUid = FindUidByName("NodeForDataClearing");

    // Verify data exists
    CPPUNIT_ASSERT(nodeWithData->GetData() != nullptr);

    // Set data to nullptr to clear it
    bool success = m_Bridge->SetNodeData(nodeUid, nullptr);
    CPPUNIT_ASSERT(success);

    // Verify data is now nullptr
    CPPUNIT_ASSERT(nodeWithData->GetData() == nullptr);

    // Verify via GetNode that data_type is now null
    auto nodeJson = m_Bridge->GetNode(nodeUid);
    CPPUNIT_ASSERT(nodeJson.has_value());
    CPPUNIT_ASSERT(nodeJson.value()["data_type"].is_null());
  }

  void SetNodeDataWithNonExistentNode()
  {
    auto newImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(5, 5, 5);

    bool success = m_Bridge->SetNodeData("non-existent-uid", newImage);
    CPPUNIT_ASSERT(!success);
  }

  void SetNodeDataSetsModificationTracking()
  {
    // Create a fresh node without modification tracking
    auto freshNode = mitk::DataNode::New();
    freshNode->SetName("FreshNodeForDataMod");
    m_DataStorage->Add(freshNode);

    auto nodeUid = FindUidByName("FreshNodeForDataMod");

    // Verify no modification tracking yet
    bool modified = false;
    CPPUNIT_ASSERT(!freshNode->GetBoolProperty(mitk::DataStorageBridge::MODIFIED_PROPERTY_KEY, modified));

    // Set data
    auto newImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(5, 5, 5);
    bool success = m_Bridge->SetNodeData(nodeUid, newImage);
    CPPUNIT_ASSERT(success);

    // Verify modification tracking is now set
    CPPUNIT_ASSERT(freshNode->GetBoolProperty(mitk::DataStorageBridge::MODIFIED_PROPERTY_KEY, modified));
    CPPUNIT_ASSERT(modified);

    std::string lastMod;
    CPPUNIT_ASSERT(freshNode->GetStringProperty(mitk::DataStorageBridge::LAST_MODIFICATION_PROPERTY_KEY, lastMod));
    CPPUNIT_ASSERT_EQUAL(std::string("data_set"), lastMod);
  }

  void GetNodeProperties()
  {
    // Basic test: get all properties
    auto props = m_Bridge->GetNodeProperties(m_Root1Uid);
    CPPUNIT_ASSERT(props.has_value());
    CPPUNIT_ASSERT(props.value().contains("visible"));
    CPPUNIT_ASSERT(props.value().contains("name"));

    // Verify property values match what's on the node
    CPPUNIT_ASSERT_EQUAL(true, props.value()["visible"].get<bool>());
    CPPUNIT_ASSERT_EQUAL(std::string("Root1"), props.value()["name"].get<std::string>());

    // Verify all properties on the node are returned (except internal restapi.* properties)
    auto* nodeProps = m_Root1->GetPropertyList();
    auto* propMap = nodeProps->GetMap();
    for (auto it = propMap->begin(); it != propMap->end(); ++it)
    {
      // Skip internal properties
      if (it->first.rfind("restapi.", 0) == 0)
        continue;

      CPPUNIT_ASSERT_MESSAGE("Property '" + it->first + "' should be in response",
        props.value().contains(it->first));
    }
  }

  void GetNodePropertiesWithScope()
  {
    // Create a node with data that has its own properties
    auto nodeWithData = mitk::DataNode::New();
    nodeWithData->SetName("NodeWithData");

    // Add properties at node level
    nodeWithData->SetBoolProperty("node_only_prop", true);

    // Create an image and set a data-level property
    auto image = mitk::Image::New();
    unsigned int dims[3] = {10, 10, 10};
    image->Initialize(mitk::MakeScalarPixelType<short>(), 3, dims);
    image->SetProperty("data_only_prop", mitk::BoolProperty::New(true));

    nodeWithData->SetData(image);
    m_DataStorage->Add(nodeWithData);

    auto nodeUid = FindUidByName("NodeWithData");

    // Test PropertyScope::Node - should only return node-level properties
    mitk::PropertyQueryParams params;
    params.scope = mitk::PropertyScope::Node;
    auto nodeProps = m_Bridge->GetNodeProperties(nodeUid, params);
    CPPUNIT_ASSERT(nodeProps.has_value());
    CPPUNIT_ASSERT(nodeProps.value().contains("node_only_prop"));
    // Note: data_only_prop should NOT be in node scope results

    // Test PropertyScope::Data - should only return data-level properties
    params.scope = mitk::PropertyScope::Data;
    auto dataProps = m_Bridge->GetNodeProperties(nodeUid, params);
    CPPUNIT_ASSERT(dataProps.has_value());
    CPPUNIT_ASSERT(dataProps.value().contains("data_only_prop"));

    // Test PropertyScope::All - should return both
    params.scope = mitk::PropertyScope::All;
    auto allProps = m_Bridge->GetNodeProperties(nodeUid, params);
    CPPUNIT_ASSERT(allProps.has_value());
    // At least the node properties should be accessible
    CPPUNIT_ASSERT(allProps.value().contains("name"));
  }

  void GetNodePropertiesWithFiltering()
  {
    // Add some specific properties to test filtering
    m_Root1->SetStringProperty("filter_test_1", "value1");
    m_Root1->SetStringProperty("filter_test_2", "value2");
    m_Root1->SetIntProperty("filter_int", 42);

    // Test filtering by specific names
    mitk::PropertyQueryParams params;
    params.names = {"filter_test_1", "filter_int"};
    auto filtered = m_Bridge->GetNodeProperties(m_Root1Uid, params);
    CPPUNIT_ASSERT(filtered.has_value());

    // Should only contain the requested properties
    CPPUNIT_ASSERT(filtered.value().contains("filter_test_1"));
    CPPUNIT_ASSERT(filtered.value().contains("filter_int"));
    CPPUNIT_ASSERT(!filtered.value().contains("filter_test_2"));
    CPPUNIT_ASSERT(!filtered.value().contains("visible")); // Not in the filter list

    // Verify values
    CPPUNIT_ASSERT_EQUAL(std::string("value1"), filtered.value()["filter_test_1"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(42, filtered.value()["filter_int"].get<int>());
  }

  void GetNodePropertiesNamesOnly()
  {
    // Test includeContent=false - should return just property names as array
    mitk::PropertyQueryParams params;
    params.includeContent = false;
    auto names = m_Bridge->GetNodeProperties(m_Root1Uid, params);
    CPPUNIT_ASSERT(names.has_value());
    CPPUNIT_ASSERT(names.value().is_array());

    // Should contain property names as strings
    bool hasVisible = false;
    bool hasName = false;
    for (const auto& name : names.value())
    {
      std::string nameStr = name.get<std::string>();
      if (nameStr == "visible") hasVisible = true;
      if (nameStr == "name") hasName = true;

      // No internal properties should be listed
      CPPUNIT_ASSERT(nameStr.rfind("restapi.", 0) != 0);
    }
    CPPUNIT_ASSERT(hasVisible);
    CPPUNIT_ASSERT(hasName);
  }

  void GetNodePropertyWithScopeAndValue()
  {
    // Test basic GetNodeProperty
    auto prop = m_Bridge->GetNodeProperty(m_Root1Uid, "visible");
    CPPUNIT_ASSERT(prop.has_value());
    CPPUNIT_ASSERT(prop.value().contains("visible"));
    CPPUNIT_ASSERT_EQUAL(true, prop.value()["visible"].get<bool>());

    // Test GetNodeProperty for string property
    prop = m_Bridge->GetNodeProperty(m_Root1Uid, "name");
    CPPUNIT_ASSERT(prop.has_value());
    CPPUNIT_ASSERT(prop.value().contains("name"));
    CPPUNIT_ASSERT_EQUAL(std::string("Root1"), prop.value()["name"].get<std::string>());

    // Test non-existent property
    prop = m_Bridge->GetNodeProperty(m_Root1Uid, "nonexistent_property");
    CPPUNIT_ASSERT(!prop.has_value());

    // Test with different property types to verify JSON representation
    m_Root1->SetIntProperty("test_int", 42);
    m_Root1->SetFloatProperty("test_float", 3.14f);
    m_Root1->SetStringProperty("test_string", "hello");

    prop = m_Bridge->GetNodeProperty(m_Root1Uid, "test_int");
    CPPUNIT_ASSERT(prop.has_value());
    CPPUNIT_ASSERT_EQUAL(42, prop.value()["test_int"].get<int>());

    prop = m_Bridge->GetNodeProperty(m_Root1Uid, "test_float");
    CPPUNIT_ASSERT(prop.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.14f, prop.value()["test_float"].get<float>(), 0.01f);

    prop = m_Bridge->GetNodeProperty(m_Root1Uid, "test_string");
    CPPUNIT_ASSERT(prop.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("hello"), prop.value()["test_string"].get<std::string>());

    // Test PropertyScope with a node that has data
    auto nodeWithData = mitk::DataNode::New();
    nodeWithData->SetName("NodeForPropTest");
    nodeWithData->SetBoolProperty("node_level", true);

    auto image = mitk::Image::New();
    unsigned int dims[3] = {5, 5, 5};
    image->Initialize(mitk::MakeScalarPixelType<short>(), 3, dims);
    image->SetProperty("data_level", mitk::BoolProperty::New(true));
    nodeWithData->SetData(image);
    m_DataStorage->Add(nodeWithData);

    auto nodeUid = FindUidByName("NodeForPropTest");

    // Node scope should find node_level
    mitk::PropertyQueryParams params;
    params.scope = mitk::PropertyScope::Node;
    prop = m_Bridge->GetNodeProperty(nodeUid, "node_level", params);
    CPPUNIT_ASSERT(prop.has_value());
    CPPUNIT_ASSERT_EQUAL(true, prop.value()["node_level"].get<bool>());

    // Data scope should find data_level
    params.scope = mitk::PropertyScope::Data;
    prop = m_Bridge->GetNodeProperty(nodeUid, "data_level", params);
    CPPUNIT_ASSERT(prop.has_value());
    CPPUNIT_ASSERT_EQUAL(true, prop.value()["data_level"].get<bool>());
  }

  void SetNodeProperty()
  {
    nlohmann::json value;
    value["value"] = 0.5;

    bool set = m_Bridge->SetNodeProperty(m_Root1Uid, "opacity", value);
    CPPUNIT_ASSERT(set);

    auto prop = m_Bridge->GetNodeProperty(m_Root1Uid, "opacity");
    CPPUNIT_ASSERT(prop.has_value());
  }

  void DeleteNodeProperty()
  {
    // First set a custom property
    nlohmann::json value;
    value["value"] = "test_value";
    m_Bridge->SetNodeProperty(m_Root1Uid, "customProp", value);

    // Verify property exists
    auto prop = m_Bridge->GetNodeProperty(m_Root1Uid, "customProp");
    CPPUNIT_ASSERT(prop.has_value());

    // Delete it
    bool deleted = m_Bridge->DeleteNodeProperty(m_Root1Uid, "customProp");
    CPPUNIT_ASSERT(deleted);

    // Verify it's gone
    prop = m_Bridge->GetNodeProperty(m_Root1Uid, "customProp");
    CPPUNIT_ASSERT(!prop.has_value());
  }

  void ReplaceNodeProperties()
  {
    // Add some properties to Root1
    m_Root1->SetFloatProperty("oldProp1", 1.0f);
    m_Root1->SetBoolProperty("oldProp2", true);

    // Replace all properties with new set
    nlohmann::json newProps;
    newProps["newProp1"] = true;
    newProps["newProp2"] = 42;

    auto result = m_Bridge->ReplaceNodeProperties(m_Root1Uid, newProps);
    CPPUNIT_ASSERT(result.has_value());

    // Verify old properties are gone (except protected ones)
    auto props = m_Bridge->GetNodeProperties(m_Root1Uid);
    CPPUNIT_ASSERT(props.has_value());
    CPPUNIT_ASSERT(props.value().contains("newProp1"));
    CPPUNIT_ASSERT(props.value().contains("newProp2"));
    CPPUNIT_ASSERT(!props.value().contains("oldProp1"));
    CPPUNIT_ASSERT(!props.value().contains("oldProp2"));
  }

  void ComplexPropertySerializationRoundTrip()
  {
    // Step 1: Set an initial color property directly on the node
    mitk::Color initialColor;
    initialColor.SetRed(0.5f);
    initialColor.SetGreen(0.25f);
    initialColor.SetBlue(0.75f);
    m_Root1->SetColor(initialColor);

    // Step 2: Get the property via the bridge and verify the format
    auto prop = m_Bridge->GetNodeProperty(m_Root1Uid, "color");
    CPPUNIT_ASSERT(prop.has_value());
    CPPUNIT_ASSERT(prop.value().contains("color"));

    // Verify the complex property format: {"color": {"value": [...], "type": "ColorProperty"}}
    auto colorJson = prop.value()["color"];
    CPPUNIT_ASSERT(colorJson.is_object());
    CPPUNIT_ASSERT(colorJson.contains("value"));
    CPPUNIT_ASSERT(colorJson.contains("type"));
    CPPUNIT_ASSERT_EQUAL(std::string("ColorProperty"), colorJson["type"].get<std::string>());

    // Verify the value array
    auto colorArray = colorJson["value"];
    CPPUNIT_ASSERT(colorArray.is_array());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), colorArray.size());

    // Verify the initial values match
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, colorArray[0].get<double>(), 1e-5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.25, colorArray[1].get<double>(), 1e-5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.75, colorArray[2].get<double>(), 1e-5);

    // Step 3: Set new values via the bridge using the complex property format
    nlohmann::json newColorValue;
    newColorValue["value"] = nlohmann::json::array({0.1, 0.2, 0.3});
    newColorValue["type"] = "ColorProperty";

    bool set = m_Bridge->SetNodeProperty(m_Root1Uid, "color", newColorValue);
    CPPUNIT_ASSERT(set);

    // Step 4: Get the property again and verify the new values
    prop = m_Bridge->GetNodeProperty(m_Root1Uid, "color");
    CPPUNIT_ASSERT(prop.has_value());
    CPPUNIT_ASSERT(prop.value().contains("color"));

    colorJson = prop.value()["color"];
    CPPUNIT_ASSERT(colorJson.contains("value"));
    colorArray = colorJson["value"];

    // Verify the new values via bridge
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, colorArray[0].get<double>(), 1e-5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.2, colorArray[1].get<double>(), 1e-5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.3, colorArray[2].get<double>(), 1e-5);

    // Step 5: Verify the underlying property is updated correctly
    auto* colorProp = dynamic_cast<mitk::ColorProperty*>(m_Root1->GetProperty("color"));
    CPPUNIT_ASSERT(colorProp != nullptr);

    auto color = colorProp->GetColor();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, color.GetRed(), 1e-5f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.2, color.GetGreen(), 1e-5f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.3, color.GetBlue(), 1e-5f);
  }

  void NameFilterWithExactMatch()
  {
    // Filter for exact name match
    mitk::NodeQueryParams params;
    params.propertyFilters.push_back({"name", "Child2", false});

    auto result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), result.nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("Child2"), result.nodes[0]["name"].get<std::string>());
  }

  void NameFilterWithWildcard()
  {
    // Test 1: Prefix wildcard - names starting with "Child"
    mitk::NodeQueryParams params;
    params.propertyFilters.push_back({"name", "Child*", false});

    auto result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), result.nodes.size());  // Child1, Child2, Child3

    // Verify the correct nodes are returned
    std::set<std::string> returnedNames;
    for (const auto& node : result.nodes)
    {
      returnedNames.insert(node["name"].get<std::string>());
    }
    CPPUNIT_ASSERT(returnedNames.count("Child1") == 1);
    CPPUNIT_ASSERT(returnedNames.count("Child2") == 1);
    CPPUNIT_ASSERT(returnedNames.count("Child3") == 1);

    // Test 2: Suffix wildcard - names ending with "1"
    params.propertyFilters.clear();
    params.propertyFilters.push_back({"name", "*1", false});

    result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), result.nodes.size());  // Root1, Child1, Grandchild1

    returnedNames.clear();
    for (const auto& node : result.nodes)
    {
      returnedNames.insert(node["name"].get<std::string>());
    }
    CPPUNIT_ASSERT(returnedNames.count("Root1") == 1);
    CPPUNIT_ASSERT(returnedNames.count("Child1") == 1);
    CPPUNIT_ASSERT(returnedNames.count("Grandchild1") == 1);

    // Test 3: Suffix wildcard - names ending with "child1"
    params.propertyFilters.clear();
    params.propertyFilters.push_back({"name", "*child1", false});

    result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), result.nodes.size());  // Only Grandchild1
    CPPUNIT_ASSERT_EQUAL(std::string("Grandchild1"), result.nodes[0]["name"].get<std::string>());

    // Test 4: No match case - wildcard that matches nothing
    params.propertyFilters.clear();
    params.propertyFilters.push_back({"name", "NonExistent*", false});

    result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), result.nodes.size());
    CPPUNIT_ASSERT_EQUAL(0, result.totalCount);

    // Test 5: No match case - suffix wildcard that matches nothing
    params.propertyFilters.clear();
    params.propertyFilters.push_back({"name", "*XYZ", false});

    result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), result.nodes.size());
    CPPUNIT_ASSERT_EQUAL(0, result.totalCount);
  }

  void BoolPropertyFilterUsesJsonRepresentation()
  {
    // This test verifies that property filtering uses JSON representation ("true"/"false")
    // rather than GetValueAsString() representation ("1"/"0") for boolean properties.
    // This makes the filter behavior consistent with what users see in REST API responses.

    // Filter for nodes where visible = true (using JSON representation)
    mitk::NodeQueryParams params;
    params.propertyFilters.push_back({"visible", "true", false});

    auto result = m_Bridge->GetNodes(params);
    // Should return 7 nodes (all except Child2 which has visible=false)
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(7), result.nodes.size());

    // Verify Child2 is NOT in the results
    for (const auto& node : result.nodes)
    {
      CPPUNIT_ASSERT(node["name"].get<std::string>() != "Child2");
    }

    // Filter for nodes where visible = false (using JSON representation)
    params.propertyFilters.clear();
    params.propertyFilters.push_back({"visible", "false", false});

    result = m_Bridge->GetNodes(params);
    // Should return only Child2
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), result.nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("Child2"), result.nodes[0]["name"].get<std::string>());

    // Verify that old-style "1"/"0" filtering does NOT work anymore
    // (This ensures we're using JSON representation, not GetValueAsString())
    params.propertyFilters.clear();
    params.propertyFilters.push_back({"visible", "1", false});  // Old style - should not match

    result = m_Bridge->GetNodes(params);
    // Should return 0 nodes because "1" doesn't match JSON boolean "true"
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), result.nodes.size());

    params.propertyFilters.clear();
    params.propertyFilters.push_back({"visible", "0", false});  // Old style - should not match

    result = m_Bridge->GetNodes(params);
    // Should return 0 nodes because "0" doesn't match JSON boolean "false"
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), result.nodes.size());
  }

  void NegatedPropertyFilter()
  {
    // Filter for nodes where visible != true (only Child2 has visible=false)
    // Note: Filtering uses JSON representation where BoolProperty is "true"/"false" (not "1"/"0")
    mitk::NodeQueryParams params;
    params.propertyFilters.push_back({"visible", "true", true});  // negated = true, filter for visible != true

    auto result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), result.nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("Child2"), result.nodes[0]["name"].get<std::string>());

    // Verify at the source: Child2 should have visible=false
    bool visibleValue = true;
    m_Child2->GetBoolProperty("visible", visibleValue);
    CPPUNIT_ASSERT_EQUAL(false, visibleValue);
  }

  void TimestampIsInteger()
  {
    auto nodeResult = m_Bridge->GetNode(m_Root1Uid);
    CPPUNIT_ASSERT(nodeResult.has_value());

    // Verify timestamp is an integer (ITK MTime)
    CPPUNIT_ASSERT(nodeResult.value()["timestamp"].is_number_unsigned());

    // The timestamp should be greater than 0
    unsigned long timestamp = nodeResult.value()["timestamp"].get<unsigned long>();
    CPPUNIT_ASSERT(timestamp > 0);
  }

  void GetNodeAvailableContexts()
  {
    auto contexts = m_Bridge->GetNodeAvailableContexts(m_Root1Uid);
    CPPUNIT_ASSERT(contexts.has_value());
    CPPUNIT_ASSERT(contexts.value().is_array());

    // Should contain at least null (default context)
    CPPUNIT_ASSERT(!contexts.value().empty());
    CPPUNIT_ASSERT(contexts.value()[0].is_null());
  }

  void SortByTimestamp()
  {
    // Modify Root1 to update its timestamp
    m_Root1->Modified();

    // Sort by timestamp ascending
    mitk::NodeQueryParams params;
    params.sort = mitk::SortSpec{"timestamp", true};

    auto result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), result.nodes.size());

    // Verify timestamps are in ascending order
    for (size_t i = 1; i < result.nodes.size(); ++i)
    {
      unsigned long prev = result.nodes[i - 1]["timestamp"].get<unsigned long>();
      unsigned long curr = result.nodes[i]["timestamp"].get<unsigned long>();
      CPPUNIT_ASSERT(prev <= curr);
    }

    // Sort descending
    params.sort = mitk::SortSpec{"timestamp", false};
    result = m_Bridge->GetNodes(params);

    for (size_t i = 1; i < result.nodes.size(); ++i)
    {
      unsigned long prev = result.nodes[i - 1]["timestamp"].get<unsigned long>();
      unsigned long curr = result.nodes[i]["timestamp"].get<unsigned long>();
      CPPUNIT_ASSERT(prev >= curr);
    }
  }

  void SortByName()
  {
    // Sort by name ascending
    mitk::NodeQueryParams params;
    params.sort = mitk::SortSpec{"name", true};

    auto result = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), result.nodes.size());

    // Verify names are in ascending order
    for (size_t i = 1; i < result.nodes.size(); ++i)
    {
      std::string prev = result.nodes[i - 1]["name"].get<std::string>();
      std::string curr = result.nodes[i]["name"].get<std::string>();
      CPPUNIT_ASSERT(prev <= curr);
    }

    // Sort descending
    params.sort = mitk::SortSpec{"name", false};
    result = m_Bridge->GetNodes(params);

    for (size_t i = 1; i < result.nodes.size(); ++i)
    {
      std::string prev = result.nodes[i - 1]["name"].get<std::string>();
      std::string curr = result.nodes[i]["name"].get<std::string>();
      CPPUNIT_ASSERT(prev >= curr);
    }
  }

  // ===== Internal property tests =====

  void InternalPropertiesNotLeakedInGetProperties()
  {
    // The restapi.uid property is set by the bridge internally
    // Verify it exists on the node but is not exposed via GetNodeProperties

    // Verify the internal property exists on the node directly
    std::string uidValue;
    CPPUNIT_ASSERT(m_Root1->GetStringProperty(mitk::NodeUidMapper::UID_PROPERTY_KEY, uidValue));
    CPPUNIT_ASSERT(!uidValue.empty());

    // GetNodeProperties should NOT include restapi.* properties
    auto props = m_Bridge->GetNodeProperties(m_Root1Uid);
    CPPUNIT_ASSERT(props.has_value());
    CPPUNIT_ASSERT(!props.value().contains(mitk::NodeUidMapper::UID_PROPERTY_KEY));

    // Also test the names-only mode
    mitk::PropertyQueryParams params;
    params.includeContent = false;
    auto names = m_Bridge->GetNodeProperties(m_Root1Uid, params);
    CPPUNIT_ASSERT(names.has_value());
    CPPUNIT_ASSERT(names.value().is_array());
    for (const auto& name : names.value())
    {
      std::string nameStr = name.get<std::string>();
      CPPUNIT_ASSERT(nameStr.rfind("restapi.", 0) != 0);
    }
  }

  void InternalPropertyCannotBeAccessed()
  {
    // Trying to get an internal property should return not found
    auto prop = m_Bridge->GetNodeProperty(m_Root1Uid, mitk::NodeUidMapper::UID_PROPERTY_KEY);
    CPPUNIT_ASSERT(!prop.has_value());

    // Also try with a different restapi.* property name
    prop = m_Bridge->GetNodeProperty(m_Root1Uid, "restapi.internal");
    CPPUNIT_ASSERT(!prop.has_value());
  }

  void InternalPropertyCannotBeSet()
  {
    // Trying to set an internal property should fail
    nlohmann::json value;
    value["value"] = "hacked_uid";

    bool result = m_Bridge->SetNodeProperty(m_Root1Uid, mitk::NodeUidMapper::UID_PROPERTY_KEY, value);
    CPPUNIT_ASSERT(!result);

    // Verify the original UID is unchanged
    std::string uidValue;
    CPPUNIT_ASSERT(m_Root1->GetStringProperty(mitk::NodeUidMapper::UID_PROPERTY_KEY, uidValue));
    CPPUNIT_ASSERT(uidValue != "hacked_uid");
    CPPUNIT_ASSERT_EQUAL(m_Root1Uid, FindUidByName("Root1"));
  }

  void InternalPropertyCannotBeDeleted()
  {
    // Trying to delete an internal property should fail
    bool result = m_Bridge->DeleteNodeProperty(m_Root1Uid, mitk::NodeUidMapper::UID_PROPERTY_KEY);
    CPPUNIT_ASSERT(!result);

    // Verify the property still exists
    std::string uidValue;
    CPPUNIT_ASSERT(m_Root1->GetStringProperty(mitk::NodeUidMapper::UID_PROPERTY_KEY, uidValue));
  }

  void ReplacePropertiesPreservesInternalProperties()
  {
    // Get current UID before replace
    std::string originalUid = m_Root1Uid;

    // Verify internal property exists
    std::string uidBefore;
    CPPUNIT_ASSERT(m_Root1->GetStringProperty(mitk::NodeUidMapper::UID_PROPERTY_KEY, uidBefore));

    // Replace all properties with a new set
    nlohmann::json newProps;
    newProps["customProp"] = true;
    newProps["anotherProp"] = 42;
    // Even if client tries to set internal property, it should be ignored
    newProps[mitk::NodeUidMapper::UID_PROPERTY_KEY] = "should_be_ignored";

    auto result = m_Bridge->ReplaceNodeProperties(m_Root1Uid, newProps);
    CPPUNIT_ASSERT(result.has_value());

    // Verify internal property was NOT removed
    std::string uidAfter;
    CPPUNIT_ASSERT(m_Root1->GetStringProperty(mitk::NodeUidMapper::UID_PROPERTY_KEY, uidAfter));
    CPPUNIT_ASSERT_EQUAL(uidBefore, uidAfter);

    // Verify we can still find the node by its original UID
    auto nodeResult = m_Bridge->GetNode(originalUid);
    CPPUNIT_ASSERT(nodeResult.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("Root1"), nodeResult.value()["name"].get<std::string>());

    // Verify the new properties were set
    auto props = m_Bridge->GetNodeProperties(originalUid);
    CPPUNIT_ASSERT(props.has_value());
    CPPUNIT_ASSERT(props.value().contains("customProp"));
    CPPUNIT_ASSERT(props.value().contains("anotherProp"));

    // Verify internal property is NOT in the response
    CPPUNIT_ASSERT(!props.value().contains(mitk::NodeUidMapper::UID_PROPERTY_KEY));
  }

  // ===== Modification tracking tests =====

  void CreateNodeSetsModificationTracking()
  {
    nlohmann::json nodeData;
    nodeData["name"] = "NewNodeForModTracking";

    auto result = m_Bridge->CreateNode(nodeData);
    CPPUNIT_ASSERT(result.success);

    // Find the created node directly in DataStorage
    auto allNodes = m_DataStorage->GetAll();
    mitk::DataNode* newNode = nullptr;
    for (auto it = allNodes->Begin(); it != allNodes->End(); ++it)
    {
      if (it->Value()->GetName() == "NewNodeForModTracking")
      {
        newNode = it->Value();
        break;
      }
    }
    CPPUNIT_ASSERT(newNode != nullptr);

    // Verify modification tracking properties are set
    bool modified = false;
    CPPUNIT_ASSERT(newNode->GetBoolProperty(mitk::DataStorageBridge::MODIFIED_PROPERTY_KEY, modified));
    CPPUNIT_ASSERT(modified);

    std::string lastMod;
    CPPUNIT_ASSERT(newNode->GetStringProperty(mitk::DataStorageBridge::LAST_MODIFICATION_PROPERTY_KEY, lastMod));
    CPPUNIT_ASSERT_EQUAL(std::string("created"), lastMod);
  }

  void UpdateNodeSetsModificationTracking()
  {
    // Reparent Child1 to Root3
    nlohmann::json updates;
    updates["parent_uid"] = m_Root3Uid;

    bool success = m_Bridge->UpdateNode(m_Child1Uid, updates);
    CPPUNIT_ASSERT(success);

    // Verify modification tracking properties are set
    bool modified = false;
    CPPUNIT_ASSERT(m_Child1->GetBoolProperty(mitk::DataStorageBridge::MODIFIED_PROPERTY_KEY, modified));
    CPPUNIT_ASSERT(modified);

    std::string lastMod;
    CPPUNIT_ASSERT(m_Child1->GetStringProperty(mitk::DataStorageBridge::LAST_MODIFICATION_PROPERTY_KEY, lastMod));
    CPPUNIT_ASSERT_EQUAL(std::string("reparented"), lastMod);
  }

  void SetNodePropertySetsModificationTracking()
  {
    nlohmann::json value;
    value["value"] = 0.75;

    bool set = m_Bridge->SetNodeProperty(m_Root1Uid, "test_opacity", value);
    CPPUNIT_ASSERT(set);

    // Verify modification tracking properties are set
    bool modified = false;
    CPPUNIT_ASSERT(m_Root1->GetBoolProperty(mitk::DataStorageBridge::MODIFIED_PROPERTY_KEY, modified));
    CPPUNIT_ASSERT(modified);

    std::string lastMod;
    CPPUNIT_ASSERT(m_Root1->GetStringProperty(mitk::DataStorageBridge::LAST_MODIFICATION_PROPERTY_KEY, lastMod));
    CPPUNIT_ASSERT_EQUAL(std::string("property_set:test_opacity"), lastMod);
  }

  void DeleteNodePropertySetsModificationTracking()
  {
    // First set a custom property
    nlohmann::json value;
    value["value"] = "to_be_deleted";
    m_Bridge->SetNodeProperty(m_Root2Uid, "deletable_prop", value);

    // Delete the property
    bool deleted = m_Bridge->DeleteNodeProperty(m_Root2Uid, "deletable_prop");
    CPPUNIT_ASSERT(deleted);

    // Verify modification tracking properties are set
    bool modified = false;
    CPPUNIT_ASSERT(m_Root2->GetBoolProperty(mitk::DataStorageBridge::MODIFIED_PROPERTY_KEY, modified));
    CPPUNIT_ASSERT(modified);

    std::string lastMod;
    CPPUNIT_ASSERT(m_Root2->GetStringProperty(mitk::DataStorageBridge::LAST_MODIFICATION_PROPERTY_KEY, lastMod));
    CPPUNIT_ASSERT_EQUAL(std::string("property_deleted:deletable_prop"), lastMod);
  }

  void ReplaceNodePropertiesSetsModificationTracking()
  {
    nlohmann::json newProps;
    newProps["replaced_prop1"] = true;
    newProps["replaced_prop2"] = 42;

    auto result = m_Bridge->ReplaceNodeProperties(m_Root3Uid, newProps);
    CPPUNIT_ASSERT(result.has_value());

    // Verify modification tracking properties are set
    bool modified = false;
    CPPUNIT_ASSERT(m_Root3->GetBoolProperty(mitk::DataStorageBridge::MODIFIED_PROPERTY_KEY, modified));
    CPPUNIT_ASSERT(modified);

    std::string lastMod;
    CPPUNIT_ASSERT(m_Root3->GetStringProperty(mitk::DataStorageBridge::LAST_MODIFICATION_PROPERTY_KEY, lastMod));
    CPPUNIT_ASSERT_EQUAL(std::string("properties_replaced"), lastMod);
  }

  void ReadOperationsDoNotSetModificationTracking()
  {
    // Create a fresh node that has NOT been accessed via bridge yet
    auto freshNode = mitk::DataNode::New();
    freshNode->SetName("FreshNode");
    m_DataStorage->Add(freshNode);

    // Verify no modification tracking properties exist yet
    bool modified = false;
    CPPUNIT_ASSERT(!freshNode->GetBoolProperty(mitk::DataStorageBridge::MODIFIED_PROPERTY_KEY, modified));
    std::string lastMod;
    CPPUNIT_ASSERT(!freshNode->GetStringProperty(mitk::DataStorageBridge::LAST_MODIFICATION_PROPERTY_KEY, lastMod));

    // Perform read operations
    auto freshUid = FindUidByName("FreshNode");

    // GetNode (read)
    m_Bridge->GetNode(freshUid);

    // GetNodes (read)
    mitk::NodeQueryParams params;
    m_Bridge->GetNodes(params);

    // GetNodeProperties (read)
    m_Bridge->GetNodeProperties(freshUid);

    // GetNodeProperty (read)
    m_Bridge->GetNodeProperty(freshUid, "name");

    // GetNodeAvailableContexts (read)
    m_Bridge->GetNodeAvailableContexts(freshUid);

    // Verify modification tracking properties are still NOT set
    // (restapi.uid is set by GetOrCreateUid, but modified/lastmodification should not be)
    CPPUNIT_ASSERT(!freshNode->GetBoolProperty(mitk::DataStorageBridge::MODIFIED_PROPERTY_KEY, modified));
    CPPUNIT_ASSERT(!freshNode->GetStringProperty(mitk::DataStorageBridge::LAST_MODIFICATION_PROPERTY_KEY, lastMod));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDataStorageBridge)
