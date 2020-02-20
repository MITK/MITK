/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAbstractNodeSelectionWidget.h"
#include <QApplication>
#include <mitkNodePredicateFunction.h>
#include <mitkStandaloneDataStorage.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class TestWidget : public QmitkAbstractNodeSelectionWidget
{
public:
  explicit TestWidget(QWidget* parent = nullptr) : QmitkAbstractNodeSelectionWidget(parent), m_UpdateInfo(0), m_OnNodePredicateChanged(0), m_OnDataStorageChanged(0),
    m_OnInternalSelectionChanged(0), m_OnNodeAddedToStorage(0), m_OnNodeRemovedFromStorage(0), m_ReviseSelectionChanged(0), m_AllowEmissionOfSelection(0), m_Allow(true)
  {
  };

  int m_UpdateInfo;
  void UpdateInfo() override
  {
    m_UpdateInfo++;
  };

  int m_OnNodePredicateChanged;
  void OnNodePredicateChanged()  override
  {
    m_OnNodePredicateChanged++;
  };

  int m_OnDataStorageChanged;
  void OnDataStorageChanged() override
  {
    m_OnDataStorageChanged++;
  };

  int m_OnInternalSelectionChanged;
  void OnInternalSelectionChanged() override
  {
    m_OnInternalSelectionChanged++;
  };

  int m_OnNodeAddedToStorage;
  void OnNodeAddedToStorage(const mitk::DataNode* node) override
  {
    m_OnNodeAddedToStorage++;
  };

  int m_OnNodeRemovedFromStorage;
  void OnNodeRemovedFromStorage(const mitk::DataNode* node) override
  {
    m_OnNodeRemovedFromStorage++;
  };

  int m_ReviseSelectionChanged;
  void ReviseSelectionChanged(const NodeList& oldInternalSelection, NodeList& newInternalSelection) override
  {
    m_ReviseSelectionChanged++;
  };

  mutable int m_AllowEmissionOfSelection;
  bool m_Allow;
  bool AllowEmissionOfSelection(const NodeList& emissionCandidates) const override
  {
    m_AllowEmissionOfSelection++;
    if (m_Allow) return QmitkAbstractNodeSelectionWidget::AllowEmissionOfSelection(emissionCandidates);

    return false;
  };
};

bool SelectionsAreEqual(const QmitkAbstractNodeSelectionWidget::NodeList& list1, const QmitkAbstractNodeSelectionWidget::NodeList& list2)
{
  return list1 == list2;
}

class QmitkAbstractNodeSelectionWidgetTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkAbstractNodeSelectionWidgetTestSuite);
  MITK_TEST(InstantiationTest);
  MITK_TEST(SetDataStorageTest);
  MITK_TEST(DataStorageEventTest);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer m_Node1;
  mitk::DataNode::Pointer m_Node2;
  mitk::DataNode::Pointer m_Node3;

  QApplication* m_TestApp;

public:
  void setUp() override
  {
    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Node1 = mitk::DataNode::New();
    m_Node1->SetName("node1");

    m_Node2 = mitk::DataNode::New();
    m_Node2->SetName("node2");

    m_Node3 = mitk::DataNode::New();
    m_Node3->SetName("node3");

    m_DataStorage->Add(m_Node1);
    m_DataStorage->Add(m_Node2);
    m_DataStorage->Add(m_Node3);

    int argc = 0;
    char** argv = nullptr;
    m_TestApp = new QApplication(argc, argv);
  }

  void tearDown() override
  {
    delete m_TestApp;
  }


  void InstantiationTest()
  {
    TestWidget* widget;
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Normal constructor fails", widget = new TestWidget());
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Normal destructor fails", delete widget);
  }

  void SetDataStorageTest()
  {
    TestWidget widget;

    widget.SetDataStorage(nullptr);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Set same data storage but triggered change", 0, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_AllowEmissionOfSelection);

    widget.SetDataStorage(m_DataStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_AllowEmissionOfSelection);

    widget.SetCurrentSelection({ m_Node1 });
    CPPUNIT_ASSERT_EQUAL(2, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_AllowEmissionOfSelection);

    widget.SetDataStorage(m_DataStorage);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Set same data storage but triggered change", 1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT(SelectionsAreEqual({ m_Node1 }, widget.GetSelectedNodes()));

    widget.SetDataStorage(nullptr);
    CPPUNIT_ASSERT_EQUAL(3, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Set same data storage but triggered change", 2, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(3, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(3, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(3, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT(SelectionsAreEqual({}, widget.GetSelectedNodes()));
  }

  void DataStorageEventTest()
  {
    TestWidget widget;
    auto newNode = mitk::DataNode::New();

    widget.SetDataStorage(m_DataStorage);
    m_DataStorage->Add(newNode);

    CPPUNIT_ASSERT_EQUAL(1, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_AllowEmissionOfSelection);

    widget.SetCurrentSelection({ newNode });
    m_DataStorage->Remove(m_Node1);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT(SelectionsAreEqual({ newNode }, widget.GetSelectedNodes()));

    m_DataStorage->Remove(newNode);
    CPPUNIT_ASSERT_EQUAL(3, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(3, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(3, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(3, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT(SelectionsAreEqual({}, widget.GetSelectedNodes()));

    widget.SetCurrentSelection({ m_Node2 });
    m_DataStorage = nullptr;
    CPPUNIT_ASSERT_EQUAL(5, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(5, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(5, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(5, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT(SelectionsAreEqual({}, widget.GetSelectedNodes()));
  }

};

MITK_TEST_SUITE_REGISTRATION(QmitkAbstractNodeSelectionWidget)
