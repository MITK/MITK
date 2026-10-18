/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDataNode.h>
#include <mitkStandaloneDataStorage.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace
{
  struct EventCounter
  {
    int Added = 0;
    int Removed = 0;
    int SourcesChanged = 0;

    void OnAdd(const mitk::DataNode *) { ++Added; }
    void OnRemove(const mitk::DataNode *) { ++Removed; }
    void OnSourcesChanged(const mitk::DataNode *) { ++SourcesChanged; }
  };

  mitk::DataNode::Pointer MakeNode(const std::string &name)
  {
    auto node = mitk::DataNode::New();
    node->SetName(name);
    return node;
  }

  mitk::DataStorage::SetOfObjects::Pointer MakeSet(const std::vector<mitk::DataNode::Pointer> &nodes)
  {
    auto set = mitk::DataStorage::SetOfObjects::New();

    for (const auto &node : nodes)
      set->push_back(node);

    return set;
  }

  std::vector<mitk::DataNode::Pointer> ToVector(const mitk::DataStorage::SetOfObjects *nodes)
  {
    return nodes->CastToSTLConstContainer();
  }
}

class mitkDataStorageSourcesTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDataStorageSourcesTestSuite);
  MITK_TEST(Remove_NodeWithSource_DerivationTakesOverSource);
  MITK_TEST(Remove_NodeAmongSeveralSources_SourcesTakeItsPlace);
  MITK_TEST(Remove_SourceHeldAlready_NoDuplicate);
  MITK_TEST(Remove_NodeWithoutSources_DerivationLosesSource);
  MITK_TEST(Remove_NodeWithDerivations_EmitsSourcesChangedForEach);
  MITK_TEST(SetSources_OtherSource_KeepsDerivations);
  MITK_TEST(SetSources_Nullptr_LeavesNodeWithoutSources);
  MITK_TEST(SetSources_OtherSource_EmitsOnlySourcesChanged);
  MITK_TEST(SetSources_SameSources_EmitsNothing);
  MITK_TEST(SetSources_SourceDerivedFromNode_ThrowsAndKeepsSources);
  MITK_TEST(SetSources_NodeItself_Throws);
  MITK_TEST(SetSources_SourceNotInStorage_Throws);
  MITK_TEST(SetSources_NodeNotInStorage_Throws);
  CPPUNIT_TEST_SUITE_END();

  mitk::StandaloneDataStorage::Pointer m_DataStorage;

public:
  void setUp() override
  {
    m_DataStorage = mitk::StandaloneDataStorage::New();
  }

  void tearDown() override
  {
    m_DataStorage = nullptr;
  }

  void Remove_NodeWithSource_DerivationTakesOverSource()
  {
    // image -> segmentation -> surface, like a polygon model of a label.
    auto image = MakeNode("image");
    auto segmentation = MakeNode("segmentation");
    auto surface = MakeNode("surface");
    m_DataStorage->Add(image);
    m_DataStorage->Add(segmentation, image);
    m_DataStorage->Add(surface, segmentation);

    m_DataStorage->Remove(segmentation);

    CPPUNIT_ASSERT(ToVector(m_DataStorage->GetSources(surface)) == std::vector<mitk::DataNode::Pointer>{ image });
    CPPUNIT_ASSERT(ToVector(m_DataStorage->GetDerivations(image)) == std::vector<mitk::DataNode::Pointer>{ surface });
  }

  void Remove_NodeAmongSeveralSources_SourcesTakeItsPlace()
  {
    auto a = MakeNode("a");
    auto b = MakeNode("b");
    auto d = MakeNode("d");
    auto removed = MakeNode("removed");
    auto derivation = MakeNode("derivation");
    m_DataStorage->Add(a);
    m_DataStorage->Add(b);
    m_DataStorage->Add(d);
    m_DataStorage->Add(removed, MakeSet({ a, b }));
    m_DataStorage->Add(derivation, MakeSet({ removed, d }));

    m_DataStorage->Remove(removed);

    const std::vector<mitk::DataNode::Pointer> expected{ a, b, d };
    CPPUNIT_ASSERT(ToVector(m_DataStorage->GetSources(derivation)) == expected);
    CPPUNIT_ASSERT(ToVector(m_DataStorage->GetDerivations(b)) == std::vector<mitk::DataNode::Pointer>{ derivation });
  }

  void Remove_SourceHeldAlready_NoDuplicate()
  {
    auto a = MakeNode("a");
    auto removed = MakeNode("removed");
    auto derivation = MakeNode("derivation");
    m_DataStorage->Add(a);
    m_DataStorage->Add(removed, a);
    m_DataStorage->Add(derivation, MakeSet({ removed, a }));

    m_DataStorage->Remove(removed);

    CPPUNIT_ASSERT(ToVector(m_DataStorage->GetSources(derivation)) == std::vector<mitk::DataNode::Pointer>{ a });
    CPPUNIT_ASSERT(ToVector(m_DataStorage->GetDerivations(a)) == std::vector<mitk::DataNode::Pointer>{ derivation });
  }

  void Remove_NodeWithoutSources_DerivationLosesSource()
  {
    auto removed = MakeNode("removed");
    auto derivation = MakeNode("derivation");
    m_DataStorage->Add(removed);
    m_DataStorage->Add(derivation, removed);

    m_DataStorage->Remove(removed);

    CPPUNIT_ASSERT(m_DataStorage->Exists(derivation));
    CPPUNIT_ASSERT_EQUAL(0u, m_DataStorage->GetSources(derivation)->Size());
  }

  void Remove_NodeWithDerivations_EmitsSourcesChangedForEach()
  {
    auto removed = MakeNode("removed");
    m_DataStorage->Add(removed);
    m_DataStorage->Add(MakeNode("first derivation"), removed);
    m_DataStorage->Add(MakeNode("second derivation"), removed);

    EventCounter counter;
    this->Listen(counter);
    m_DataStorage->Remove(removed);
    this->StopListening(counter);

    CPPUNIT_ASSERT_EQUAL(1, counter.Removed);
    CPPUNIT_ASSERT_EQUAL(2, counter.SourcesChanged);
  }

  void SetSources_OtherSource_KeepsDerivations()
  {
    auto a = MakeNode("a");
    auto b = MakeNode("b");
    auto node = MakeNode("node");
    auto derivation = MakeNode("derivation");
    m_DataStorage->Add(a);
    m_DataStorage->Add(b);
    m_DataStorage->Add(node, a);
    m_DataStorage->Add(derivation, node);

    m_DataStorage->SetSources(node, MakeSet({ b }));

    CPPUNIT_ASSERT(ToVector(m_DataStorage->GetSources(node)) == std::vector<mitk::DataNode::Pointer>{ b });
    CPPUNIT_ASSERT(ToVector(m_DataStorage->GetSources(derivation)) == std::vector<mitk::DataNode::Pointer>{ node });
    CPPUNIT_ASSERT_EQUAL(0u, m_DataStorage->GetDerivations(a)->Size());
    CPPUNIT_ASSERT(ToVector(m_DataStorage->GetDerivations(b)) == std::vector<mitk::DataNode::Pointer>{ node });
  }

  void SetSources_Nullptr_LeavesNodeWithoutSources()
  {
    auto a = MakeNode("a");
    auto node = MakeNode("node");
    m_DataStorage->Add(a);
    m_DataStorage->Add(node, a);

    m_DataStorage->SetSources(node, nullptr);

    CPPUNIT_ASSERT_EQUAL(0u, m_DataStorage->GetSources(node)->Size());
    CPPUNIT_ASSERT_EQUAL(0u, m_DataStorage->GetDerivations(a)->Size());
  }

  void SetSources_OtherSource_EmitsOnlySourcesChanged()
  {
    auto a = MakeNode("a");
    auto b = MakeNode("b");
    auto node = MakeNode("node");
    m_DataStorage->Add(a);
    m_DataStorage->Add(b);
    m_DataStorage->Add(node, a);

    EventCounter counter;
    this->Listen(counter);
    m_DataStorage->SetSources(node, MakeSet({ b }));
    this->StopListening(counter);

    CPPUNIT_ASSERT_EQUAL(0, counter.Added);
    CPPUNIT_ASSERT_EQUAL(0, counter.Removed);
    CPPUNIT_ASSERT_EQUAL(1, counter.SourcesChanged);
  }

  void SetSources_SameSources_EmitsNothing()
  {
    auto a = MakeNode("a");
    auto node = MakeNode("node");
    m_DataStorage->Add(a);
    m_DataStorage->Add(node, a);

    EventCounter counter;
    this->Listen(counter);
    m_DataStorage->SetSources(node, MakeSet({ a }));
    this->StopListening(counter);

    CPPUNIT_ASSERT_EQUAL(0, counter.SourcesChanged);
    CPPUNIT_ASSERT(ToVector(m_DataStorage->GetDerivations(a)) == std::vector<mitk::DataNode::Pointer>{ node });
  }

  void SetSources_SourceDerivedFromNode_ThrowsAndKeepsSources()
  {
    auto a = MakeNode("a");
    auto node = MakeNode("node");
    auto derivation = MakeNode("derivation");
    auto indirectDerivation = MakeNode("indirectDerivation");
    m_DataStorage->Add(a);
    m_DataStorage->Add(node, a);
    m_DataStorage->Add(derivation, node);
    m_DataStorage->Add(indirectDerivation, derivation);

    CPPUNIT_ASSERT_THROW(m_DataStorage->SetSources(node, MakeSet({ indirectDerivation })), std::invalid_argument);
    CPPUNIT_ASSERT(ToVector(m_DataStorage->GetSources(node)) == std::vector<mitk::DataNode::Pointer>{ a });
  }

  void SetSources_NodeItself_Throws()
  {
    auto node = MakeNode("node");
    m_DataStorage->Add(node);

    CPPUNIT_ASSERT_THROW(m_DataStorage->SetSources(node, MakeSet({ node })), std::invalid_argument);
  }

  void SetSources_SourceNotInStorage_Throws()
  {
    auto node = MakeNode("node");
    m_DataStorage->Add(node);

    CPPUNIT_ASSERT_THROW(m_DataStorage->SetSources(node, MakeSet({ MakeNode("outside") })), std::invalid_argument);
  }

  void SetSources_NodeNotInStorage_Throws()
  {
    auto a = MakeNode("a");
    m_DataStorage->Add(a);

    CPPUNIT_ASSERT_THROW(m_DataStorage->SetSources(MakeNode("outside"), MakeSet({ a })), std::invalid_argument);
  }

private:
  void Listen(EventCounter &counter)
  {
    m_DataStorage->AddNodeEvent += mitk::MessageDelegate1<EventCounter, const mitk::DataNode *>(&counter, &EventCounter::OnAdd);
    m_DataStorage->RemoveNodeEvent += mitk::MessageDelegate1<EventCounter, const mitk::DataNode *>(&counter, &EventCounter::OnRemove);
    m_DataStorage->SourcesChangedNodeEvent +=
      mitk::MessageDelegate1<EventCounter, const mitk::DataNode *>(&counter, &EventCounter::OnSourcesChanged);
  }

  void StopListening(EventCounter &counter)
  {
    m_DataStorage->AddNodeEvent -= mitk::MessageDelegate1<EventCounter, const mitk::DataNode *>(&counter, &EventCounter::OnAdd);
    m_DataStorage->RemoveNodeEvent -= mitk::MessageDelegate1<EventCounter, const mitk::DataNode *>(&counter, &EventCounter::OnRemove);
    m_DataStorage->SourcesChangedNodeEvent -=
      mitk::MessageDelegate1<EventCounter, const mitk::DataNode *>(&counter, &EventCounter::OnSourcesChanged);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDataStorageSources)
