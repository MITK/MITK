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
#include "QmitkModelViewSelectionConnector.h"

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkRenderingTestHelper.h>

extern std::vector<std::string> globalCmdLineArgs;

class TestWidget : public QmitkAbstractNodeSelectionWidget
{
public:
  explicit TestWidget(QWidget* parent = nullptr) : QmitkAbstractNodeSelectionWidget(parent), m_UpdateInfo(0), m_OnNodePredicateChanged(0), m_OnDataStorageChanged(0),
    m_OnInternalSelectionChanged(0), m_OnNodeAddedToStorage(0), m_OnNodeRemovedFromStorage(0), m_ReviseSelectionChanged(0), m_AllowEmissionOfSelection(0), m_Allow(true), m_NewSelectionEmited(0)
  {
    connect(this, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &TestWidget::NewSelectionEmited);
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
  void OnNodeAddedToStorage(const mitk::DataNode* /*node*/) override
  {
    m_OnNodeAddedToStorage++;
  };

  int m_OnNodeRemovedFromStorage;
  void OnNodeRemovedFromStorage(const mitk::DataNode* /*node*/) override
  {
    m_OnNodeRemovedFromStorage++;
  };

  int m_ReviseSelectionChanged;
  void ReviseSelectionChanged(const NodeList& /*oldInternalSelection*/, NodeList& /*newInternalSelection*/) override
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

  int m_NewSelectionEmited;
  QmitkAbstractNodeSelectionWidget::NodeList m_LastNewEmision;
  void NewSelectionEmited(NodeList selection)
  {
    m_NewSelectionEmited++;
    m_LastNewEmision = selection;
  };
};

class QmitkAbstractNodeSelectionWidgetTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkAbstractNodeSelectionWidgetTestSuite);
  MITK_TEST(SetDataStorageTest);
  MITK_TEST(DataStorageEventTest);
  MITK_TEST(NodePredicateTest);
  MITK_TEST(SelectOnlyVisibleNodesTest);
  MITK_TEST(OtherSetterAndGetterTest);
  MITK_TEST(AllowEmissionOfSelectionTest);
  MITK_TEST(OnNodeModifiedTest);
  MITK_TEST(SignalRecursionTest);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer m_Node1;
  mitk::DataNode::Pointer m_Node1_2;
  mitk::DataNode::Pointer m_Node2;
  mitk::DataNode::Pointer m_Node3;

  QApplication* m_TestApp;

public:
  void setUp() override
  {
    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Node1 = mitk::DataNode::New();
    m_Node1->SetName("node1_1");

    m_Node2 = mitk::DataNode::New();
    m_Node2->SetName("node2");

    m_Node3 = mitk::DataNode::New();
    m_Node3->SetName("node3");

    m_Node1_2 = mitk::DataNode::New();
    m_Node1_2->SetName("node1_2");

    m_DataStorage->Add(m_Node1);
    m_DataStorage->Add(m_Node2);
    m_DataStorage->Add(m_Node3);
    m_DataStorage->Add(m_Node1_2);

    mitk::RenderingTestHelper::ArgcHelperClass cmdLineArgs(globalCmdLineArgs);
    auto argc = cmdLineArgs.GetArgc();
    auto argv = cmdLineArgs.GetArgv();
    m_TestApp = new QApplication(argc, argv);
  }

  mitk::NodePredicateBase::Pointer GeneratTestPredicate(const std::string& name)
  {
    auto check = [name](const mitk::DataNode * node)
    {
      return node->GetName().find(name,0) == 0;
    };
    auto predicate = mitk::NodePredicateFunction::New(check);
    return predicate.GetPointer();
  }

  void tearDown() override
  {
    delete m_TestApp;
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
    CPPUNIT_ASSERT_EQUAL(0, widget.m_NewSelectionEmited);

    widget.SetDataStorage(m_DataStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_NewSelectionEmited);

    widget.SetCurrentSelection({ m_Node1 });
    CPPUNIT_ASSERT_EQUAL(1, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1 }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1 }, widget.GetSelectedNodes()));

    widget.SetDataStorage(m_DataStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Set same data storage but triggered change", 1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1 }, widget.GetSelectedNodes()));

    widget.SetDataStorage(nullptr);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Set same data storage but triggered change", 2, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({}, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({}, widget.GetSelectedNodes()));
  }

  void DataStorageEventTest()
  {
    TestWidget widget;
    auto newNode = mitk::DataNode::New();

    widget.SetDataStorage(m_DataStorage);
    m_DataStorage->Add(newNode);

    CPPUNIT_ASSERT_EQUAL(0, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_NewSelectionEmited);

    widget.SetCurrentSelection({ newNode });
    m_DataStorage->Remove(m_Node1);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ newNode }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ newNode }, widget.GetSelectedNodes()));

    m_DataStorage->Remove(newNode);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({}, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({}, widget.GetSelectedNodes()));

    widget.SetCurrentSelection({ m_Node2 });
    m_DataStorage = nullptr;
    CPPUNIT_ASSERT_EQUAL(4, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(4, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(4, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(4, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(4, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({}, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({}, widget.GetSelectedNodes()));
  }

  void NodePredicateTest()
  {
    TestWidget widget;

    CPPUNIT_ASSERT(nullptr == widget.GetNodePredicate());
    auto testPred = GeneratTestPredicate("node2");
    widget.SetNodePredicate(testPred);
    CPPUNIT_ASSERT(testPred == widget.GetNodePredicate());

    CPPUNIT_ASSERT_EQUAL(0, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_NewSelectionEmited);

    widget.SetDataStorage(m_DataStorage);

    widget.SetCurrentSelection({ m_Node3 });
    CPPUNIT_ASSERT_EQUAL(0, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({}, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({}, widget.GetSelectedNodes()));

    widget.SetCurrentSelection({ m_Node2 });
    CPPUNIT_ASSERT_EQUAL(1, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node2 }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node2 }, widget.GetSelectedNodes()));

    //change predicate while nodes are selected
    widget.SetNodePredicate(GeneratTestPredicate("node1"));
    CPPUNIT_ASSERT_EQUAL(2, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({}, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({}, widget.GetSelectedNodes()));

    //change selection to mixed one (valid nodes and invalid ones)
    widget.SetCurrentSelection({ m_Node1, m_Node2, m_Node1_2 });
    CPPUNIT_ASSERT_EQUAL(3, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(3, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(3, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(3, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(3, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1, m_Node1_2 }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1, m_Node1_2 }, widget.GetSelectedNodes()));
  }

  void SelectOnlyVisibleNodesTest()
  {
    TestWidget widget;

    CPPUNIT_ASSERT_EQUAL(true, widget.GetSelectOnlyVisibleNodes());

    CPPUNIT_ASSERT(nullptr == widget.GetNodePredicate());
    auto testPred = GeneratTestPredicate("node2");
    widget.SetNodePredicate(testPred);
    widget.SetDataStorage(m_DataStorage);
    widget.SetCurrentSelection({ m_Node3 });
    CPPUNIT_ASSERT_EQUAL(0, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({}, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({}, widget.GetSelectedNodes()));

    widget.SetSelectOnlyVisibleNodes(false);
    CPPUNIT_ASSERT_EQUAL(false, widget.GetSelectOnlyVisibleNodes());
    CPPUNIT_ASSERT_EQUAL(0, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node3 }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node3 }, widget.GetSelectedNodes()));

    //change selection to mixed one (valid nodes and invalid ones)
    widget.SetCurrentSelection({ m_Node1, m_Node2, m_Node1_2 });
    CPPUNIT_ASSERT_EQUAL(1, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1, m_Node2, m_Node1_2 }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1, m_Node2, m_Node1_2 }, widget.GetSelectedNodes()));

    //change predicate while nodes are selected
    widget.SetNodePredicate(GeneratTestPredicate("node1"));
    CPPUNIT_ASSERT_EQUAL(2, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(3, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1, m_Node2, m_Node1_2 }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1, m_Node2, m_Node1_2 }, widget.GetSelectedNodes()));

    widget.SetSelectOnlyVisibleNodes(true);
    CPPUNIT_ASSERT_EQUAL(true, widget.GetSelectOnlyVisibleNodes());
    CPPUNIT_ASSERT_EQUAL(2, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(4, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(3, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1, m_Node1_2 }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1, m_Node1_2 }, widget.GetSelectedNodes()));
  }

  void OtherSetterAndGetterTest()
  {
    TestWidget widget;

    CPPUNIT_ASSERT("Error. Select data." == widget.GetInvalidInfo());
    CPPUNIT_ASSERT("Empty. Make a selection." == widget.GetEmptyInfo());
    CPPUNIT_ASSERT("Select a data node" == widget.GetPopUpTitel());
    CPPUNIT_ASSERT("" == widget.GetPopUpHint());
    CPPUNIT_ASSERT(false == widget.GetSelectionIsOptional());

    widget.SetInvalidInfo("SetInvalidInfo");
    widget.SetEmptyInfo("SetEmptyInfo");
    widget.SetPopUpTitel("SetPopUpTitel");
    widget.SetPopUpHint("SetPopUpHint");
    widget.SetSelectionIsOptional(true);

    CPPUNIT_ASSERT("SetInvalidInfo" == widget.GetInvalidInfo());
    CPPUNIT_ASSERT("SetEmptyInfo" == widget.GetEmptyInfo());
    CPPUNIT_ASSERT("SetPopUpTitel" == widget.GetPopUpTitel());
    CPPUNIT_ASSERT("SetPopUpHint" == widget.GetPopUpHint());
    CPPUNIT_ASSERT(true == widget.GetSelectionIsOptional());
  }

  void AllowEmissionOfSelectionTest()
  {
    TestWidget widget;

    widget.SetDataStorage(m_DataStorage);
    widget.SetCurrentSelection({ m_Node3 });
    CPPUNIT_ASSERT_EQUAL(1, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node3 }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node3 }, widget.GetSelectedNodes()));

    widget.m_Allow = false;

    widget.SetCurrentSelection({ m_Node1 });
    CPPUNIT_ASSERT_EQUAL(2, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node3 }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1 }, widget.GetSelectedNodes()));
  }

  void OnNodeModifiedTest()
  {
    TestWidget widget;

    widget.SetDataStorage(m_DataStorage);
    widget.SetCurrentSelection({ m_Node3 });

    //Check OnNodeModified behavour without predicate
    m_Node3->SetName("NewName");
    CPPUNIT_ASSERT_EQUAL(1, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node3 }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node3 }, widget.GetSelectedNodes()));

    //simulate behaviour if allowance is changed to true
    widget.m_Allow = false;
    m_Node3->SetName("NewName_temp");
    widget.m_Allow = true;
    m_Node3->SetName("NewName");
    CPPUNIT_ASSERT_EQUAL(3, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(6, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node3 }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node3 }, widget.GetSelectedNodes()));

    //change irrelevant node
    m_Node2->SetName("IrrelevantNode");
    CPPUNIT_ASSERT_EQUAL(3, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(6, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node3 }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node3 }, widget.GetSelectedNodes()));

    //test with predicate set
    widget.SetNodePredicate(GeneratTestPredicate("node1"));
    m_Node3->SetName("NowAlsoIrrelevantNode");
    CPPUNIT_ASSERT_EQUAL(4, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(7, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(2, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ }, widget.GetSelectedNodes()));

    widget.SetCurrentSelection({ m_Node1, m_Node1_2 });
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1, m_Node1_2 }, widget.m_LastNewEmision));

    m_Node1->SetName("NodeSortedOutByPredicate");
    CPPUNIT_ASSERT_EQUAL(6, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(4, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(4, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(9, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(4, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1_2 }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1_2 }, widget.GetSelectedNodes()));
  }

  void SignalRecursionTest()
  {
    TestWidget widget;
    widget.SetDataStorage(m_DataStorage);

    TestWidget widget2;
    widget2.SetDataStorage(m_DataStorage);

    QmitkAbstractNodeSelectionWidget::connect(&widget, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, &widget2, &QmitkAbstractNodeSelectionWidget::SetCurrentSelection);
    QmitkAbstractNodeSelectionWidget::connect(&widget2, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, &widget, &QmitkAbstractNodeSelectionWidget::SetCurrentSelection);

    widget.SetCurrentSelection({ m_Node1 });
    CPPUNIT_ASSERT_EQUAL(1, widget.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(1, widget.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1 }, widget.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1 }, widget.GetSelectedNodes()));

    CPPUNIT_ASSERT_EQUAL(1, widget2.m_UpdateInfo);
    CPPUNIT_ASSERT_EQUAL(0, widget2.m_OnNodePredicateChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget2.m_OnDataStorageChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget2.m_OnInternalSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(0, widget2.m_OnNodeAddedToStorage);
    CPPUNIT_ASSERT_EQUAL(0, widget2.m_OnNodeRemovedFromStorage);
    CPPUNIT_ASSERT_EQUAL(1, widget2.m_ReviseSelectionChanged);
    CPPUNIT_ASSERT_EQUAL(1, widget2.m_AllowEmissionOfSelection);
    CPPUNIT_ASSERT_EQUAL(1, widget2.m_NewSelectionEmited);
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1 }, widget2.m_LastNewEmision));
    CPPUNIT_ASSERT(EqualNodeSelections({ m_Node1 }, widget2.GetSelectedNodes()));
  }

};

MITK_TEST_SUITE_REGISTRATION(QmitkAbstractNodeSelectionWidget)
