/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

#include "mitkNodeUidMapper.h"
#include <mitkStandaloneDataStorage.h>
#include <mitkStringProperty.h>

class mitkNodeUidMapperTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNodeUidMapperTestSuite);
  MITK_TEST(GeneratesUniqueUids);
  MITK_TEST(ReturnsSameUidForSameNode);
  MITK_TEST(FindsNodeByUidAmongManyNodes);
  MITK_TEST(ClearsUidOnNodeRemoval);
  MITK_TEST(ClearsTransientPropertiesOnConnect);
  MITK_TEST(ClearsTransientPropertiesOnNodeAdd);
  MITK_TEST(HasUidReturnsFalseForUnknownUid);
  MITK_TEST(ThrowsOnNullNode);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  std::unique_ptr<mitk::NodeUidMapper> m_Mapper;

public:
  void setUp() override
  {
    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Mapper = std::make_unique<mitk::NodeUidMapper>();
    m_Mapper->SetDataStorage(m_DataStorage);
  }

  void tearDown() override
  {
    m_Mapper->ClearDataStorage();
    m_Mapper.reset();
    m_DataStorage = nullptr;
  }

  void GeneratesUniqueUids()
  {
    auto node1 = mitk::DataNode::New();
    auto node2 = mitk::DataNode::New();
    m_DataStorage->Add(node1);
    m_DataStorage->Add(node2);

    auto uid1 = m_Mapper->GetOrCreateUid(node1);
    auto uid2 = m_Mapper->GetOrCreateUid(node2);

    CPPUNIT_ASSERT(!uid1.empty());
    CPPUNIT_ASSERT(!uid2.empty());
    CPPUNIT_ASSERT(uid1 != uid2);
  }

  void ReturnsSameUidForSameNode()
  {
    auto node = mitk::DataNode::New();
    m_DataStorage->Add(node);

    auto uid1 = m_Mapper->GetOrCreateUid(node);
    auto uid2 = m_Mapper->GetOrCreateUid(node);

    CPPUNIT_ASSERT_EQUAL(uid1, uid2);
  }

  void FindsNodeByUidAmongManyNodes()
  {
    // Create multiple nodes to ensure lookup is not just luck
    auto nodeA = mitk::DataNode::New();
    nodeA->SetName("NodeA");
    m_DataStorage->Add(nodeA);

    auto nodeB = mitk::DataNode::New();
    nodeB->SetName("NodeB");
    m_DataStorage->Add(nodeB);

    auto targetNode = mitk::DataNode::New();
    targetNode->SetName("TargetNode");
    m_DataStorage->Add(targetNode);

    auto nodeC = mitk::DataNode::New();
    nodeC->SetName("NodeC");
    m_DataStorage->Add(nodeC);

    auto nodeD = mitk::DataNode::New();
    nodeD->SetName("NodeD");
    m_DataStorage->Add(nodeD);

    // Get UIDs for all nodes
    auto uidA = m_Mapper->GetOrCreateUid(nodeA);
    auto uidB = m_Mapper->GetOrCreateUid(nodeB);
    auto targetUid = m_Mapper->GetOrCreateUid(targetNode);
    auto uidC = m_Mapper->GetOrCreateUid(nodeC);
    auto uidD = m_Mapper->GetOrCreateUid(nodeD);

    // Verify all UIDs are unique
    CPPUNIT_ASSERT(uidA != uidB);
    CPPUNIT_ASSERT(uidA != targetUid);
    CPPUNIT_ASSERT(uidA != uidC);
    CPPUNIT_ASSERT(uidA != uidD);
    CPPUNIT_ASSERT(uidB != targetUid);
    CPPUNIT_ASSERT(targetUid != uidC);
    CPPUNIT_ASSERT(targetUid != uidD);

    // Find the target node by UID - this should return exactly the right node
    auto* foundNode = m_Mapper->FindNodeByUid(targetUid);

    CPPUNIT_ASSERT(foundNode != nullptr);
    CPPUNIT_ASSERT_EQUAL(std::string("TargetNode"), foundNode->GetName());
    CPPUNIT_ASSERT(foundNode == targetNode.GetPointer());

    // Verify we can find other nodes too
    auto* foundA = m_Mapper->FindNodeByUid(uidA);
    CPPUNIT_ASSERT(foundA != nullptr);
    CPPUNIT_ASSERT_EQUAL(std::string("NodeA"), foundA->GetName());

    auto* foundD = m_Mapper->FindNodeByUid(uidD);
    CPPUNIT_ASSERT(foundD != nullptr);
    CPPUNIT_ASSERT_EQUAL(std::string("NodeD"), foundD->GetName());
  }

  void ClearsUidOnNodeRemoval()
  {
    auto node = mitk::DataNode::New();
    m_DataStorage->Add(node);

    auto uid = m_Mapper->GetOrCreateUid(node);
    CPPUNIT_ASSERT(m_Mapper->HasUid(uid));

    m_DataStorage->Remove(node);

    CPPUNIT_ASSERT(!m_Mapper->HasUid(uid));
    CPPUNIT_ASSERT(m_Mapper->FindNodeByUid(uid) == nullptr);
  }

  void ClearsTransientPropertiesOnConnect()
  {
    // Disconnect mapper first
    m_Mapper->SetDataStorage(nullptr);

    // Simulate a node from a "loaded scene" with pre-existing UID property
    auto node = mitk::DataNode::New();
    m_DataStorage->Add(node);
    // Set the property AFTER adding to DataStorage (simulating loaded session data)
    node->SetStringProperty(mitk::NodeUidMapper::UID_PROPERTY_KEY, "old_uid");

    // Verify the property exists before reconnecting
    std::string value;
    CPPUNIT_ASSERT(node->GetStringProperty(mitk::NodeUidMapper::UID_PROPERTY_KEY, value));
    CPPUNIT_ASSERT_EQUAL(std::string("old_uid"), value);

    // Reconnect (simulates fresh REST API session starting)
    // OnNodeAdded should clear the old UID properties
    m_Mapper->SetDataStorage(m_DataStorage);

    // Old UID property should be cleared
    CPPUNIT_ASSERT(!node->GetStringProperty(mitk::NodeUidMapper::UID_PROPERTY_KEY, value));
  }

  void ClearsTransientPropertiesOnNodeAdd()
  {
    // This tests the "load scene" use case:
    // When the scene deserializer loads a scene and adds nodes to the storage,
    // any pre-existing REST UID properties from a previous session should be removed.
    // This happens automatically when nodes are added while the mapper is connected.

    // Create a node outside the storage first
    auto node = mitk::DataNode::New();
    node->SetName("LoadedNode");

    // Simulate that this node was deserialized from a scene file that had
    // a REST UID property from a previous REST API session
    node->SetStringProperty(mitk::NodeUidMapper::UID_PROPERTY_KEY, "old_session_uid_12345");

    // Verify the property exists before adding to storage
    std::string value;
    CPPUNIT_ASSERT(node->GetStringProperty(mitk::NodeUidMapper::UID_PROPERTY_KEY, value));
    CPPUNIT_ASSERT_EQUAL(std::string("old_session_uid_12345"), value);

    // Add the node to the data storage (mapper is already connected via setUp)
    // This should trigger OnNodeAdded which clears the old UID property
    m_DataStorage->Add(node);

    // The old UID property should be cleared
    CPPUNIT_ASSERT(!node->GetStringProperty(mitk::NodeUidMapper::UID_PROPERTY_KEY, value));

    // Now when we request a UID, it should generate a fresh one
    auto newUid = m_Mapper->GetOrCreateUid(node);
    CPPUNIT_ASSERT(!newUid.empty());
    CPPUNIT_ASSERT(newUid != "old_session_uid_12345");

    // The new UID should be stored on the node
    CPPUNIT_ASSERT(node->GetStringProperty(mitk::NodeUidMapper::UID_PROPERTY_KEY, value));
    CPPUNIT_ASSERT_EQUAL(newUid, value);
  }

  void HasUidReturnsFalseForUnknownUid()
  {
    CPPUNIT_ASSERT(!m_Mapper->HasUid("nonexistent_uid"));
    CPPUNIT_ASSERT(!m_Mapper->HasUid("node_999999"));
  }

  void ThrowsOnNullNode()
  {
    CPPUNIT_ASSERT_THROW(m_Mapper->GetOrCreateUid(nullptr), std::invalid_argument);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkNodeUidMapper)
