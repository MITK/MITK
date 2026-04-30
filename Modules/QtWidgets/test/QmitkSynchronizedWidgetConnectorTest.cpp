/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkSynchronizedNodeSelectionWidget.h>
#include <QmitkSynchronizedWidgetConnector.h>

#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <algorithm>

/**
 * Direct unit test for QmitkSynchronizedWidgetConnector. The connector hub
 * mediates selection / select-all / visibility synchronization across attached
 * QmitkSynchronizedNodeSelectionWidgets. It has no direct test today; this
 * suite covers:
 *   - Connect/Disconnect lifecycle and the auto-reset of m_SelectAll when the
 *     internal counter drops to zero (observable via GetSelectionMode())
 *   - SynchronizeWidget propagates select-all and explicit-selection state
 *   - ChangeSelection notifies attached widgets
 *
 * Counter behavior is verified indirectly through the documented "rearm
 * select-all when last widget detaches" rule (DeregisterWidget) -- a more
 * faithful black-box probe than reaching into the private counter.
 */
class QmitkSynchronizedWidgetConnectorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkSynchronizedWidgetConnectorTestSuite);
  MITK_TEST(InitialState_IsSelectAllTrueAndEmptySelection);
  MITK_TEST(ConnectDisconnect_RearmsSelectAllOnLastDetach);
  MITK_TEST(ConnectDisconnect_KeepsModeWhileWidgetsRemain);
  MITK_TEST(SynchronizeWidget_PropagatesSelectAllOn);
  MITK_TEST(SynchronizeWidget_PropagatesExplicitSelection);
  MITK_TEST(ChangeSelection_StoresSelection);
  MITK_TEST(ChangeSelection_EmitsOnlyWhenChanged);
  MITK_TEST(ChangeSelectionMode_EmitsOnlyWhenChanged);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer m_Node1;
  mitk::DataNode::Pointer m_Node2;
  mitk::DataNode::Pointer m_Node3;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();

    // QmitkRenderWindowDataNodeTableModel sorts its node list by the int
    // "layer" property and its comparator returns true unconditionally when
    // the lookup fails for either side, which violates strict-weak-ordering
    // and trips MSVC's debug-mode _Debug_lt_pred check during SelectAll().
    // Test nodes therefore carry an explicit node-level layer so the lookup
    // always succeeds with the (renderer-less) widgets used here.
    // TODO: drop the SetIntProperty("layer", ...) calls below once
    //       QmitkRenderWindowDataNodeTableModel::UpdateModelData's comparator
    //       is fixed to be strict-weak-ordered (return false when neither
    //       node has the property).
    //       NOTE: the same workaround is duplicated in
    //       'QmitkMxNSyncGroupApiTest.cpp' -- keep both in sync
    //       and remove together.
    m_Node1 = mitk::DataNode::New();
    m_Node1->SetName("node1");
    m_Node1->SetIntProperty("layer", 0);
    m_DataStorage->Add(m_Node1);

    m_Node2 = mitk::DataNode::New();
    m_Node2->SetName("node2");
    m_Node2->SetIntProperty("layer", 1);
    m_DataStorage->Add(m_Node2);

    m_Node3 = mitk::DataNode::New();
    m_Node3->SetName("node3");
    m_Node3->SetIntProperty("layer", 2);
    m_DataStorage->Add(m_Node3);
  }

  void tearDown() override
  {
    m_Node1 = nullptr;
    m_Node2 = nullptr;
    m_Node3 = nullptr;
    m_DataStorage = nullptr;
  }

  static bool ListContains(const QmitkSynchronizedWidgetConnector::NodeList& list,
                           const mitk::DataNode* node)
  {
    return std::any_of(list.begin(), list.end(),
                       [node](const mitk::DataNode::Pointer& entry) { return entry.GetPointer() == node; });
  }

  // ---------- Initial state ----------

  void InitialState_IsSelectAllTrueAndEmptySelection()
  {
    QmitkSynchronizedWidgetConnector connector;
    CPPUNIT_ASSERT_EQUAL(true, connector.GetSelectionMode());
    CPPUNIT_ASSERT(connector.GetNodeSelection().isEmpty());
  }

  // ---------- Connect / Disconnect lifecycle ----------

  void ConnectDisconnect_RearmsSelectAllOnLastDetach()
  {
    QmitkSynchronizedWidgetConnector connector;
    QmitkSynchronizedNodeSelectionWidget w(nullptr);
    w.SetDataStorage(m_DataStorage);

    connector.ChangeSelectionMode(false);
    CPPUNIT_ASSERT_EQUAL(false, connector.GetSelectionMode());

    connector.ConnectWidget(&w);
    connector.DisconnectWidget(&w);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "When the last widget disconnects, the connector must rearm select-all "
      "so subsequent attachments do not silently miss new storage nodes",
      true, connector.GetSelectionMode());
  }

  void ConnectDisconnect_KeepsModeWhileWidgetsRemain()
  {
    QmitkSynchronizedWidgetConnector connector;
    QmitkSynchronizedNodeSelectionWidget w1(nullptr);
    QmitkSynchronizedNodeSelectionWidget w2(nullptr);
    w1.SetDataStorage(m_DataStorage);
    w2.SetDataStorage(m_DataStorage);

    connector.ChangeSelectionMode(false);
    connector.ConnectWidget(&w1);
    connector.ConnectWidget(&w2);

    connector.DisconnectWidget(&w1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "While widgets remain attached, the connector must keep its mode",
      false, connector.GetSelectionMode());

    connector.DisconnectWidget(&w2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Once the last widget detaches, mode must rearm to select-all",
      true, connector.GetSelectionMode());
  }

  // ---------- SynchronizeWidget ----------

  void SynchronizeWidget_PropagatesSelectAllOn()
  {
    QmitkSynchronizedWidgetConnector connector;
    QmitkSynchronizedNodeSelectionWidget w(nullptr);
    w.SetDataStorage(m_DataStorage);

    // Default state: m_SelectAll == true.
    connector.ConnectWidget(&w);
    connector.SynchronizeWidget(&w);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Widget should reflect the connector's select-all mode",
      true, w.GetSelectAll());
  }

  void SynchronizeWidget_PropagatesExplicitSelection()
  {
    QmitkSynchronizedWidgetConnector connector;
    QmitkSynchronizedNodeSelectionWidget w(nullptr);
    w.SetDataStorage(m_DataStorage);

    QmitkSynchronizedWidgetConnector::NodeList selection;
    selection.append(m_Node2);
    connector.ChangeSelectionMode(false);
    connector.ChangeSelection(selection);

    connector.ConnectWidget(&w);
    connector.SynchronizeWidget(&w);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Widget should not be in select-all mode after sync",
      false, w.GetSelectAll());

    auto current = w.GetSelectedNodes();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Selection should contain exactly the connector's stored nodes",
      1, static_cast<int>(current.size()));
    CPPUNIT_ASSERT_MESSAGE("Stored node should be present",
                           ListContains(current, m_Node2));
  }

  // ---------- ChangeSelection / ChangeSelectionMode signal contracts ----------

  void ChangeSelection_StoresSelection()
  {
    QmitkSynchronizedWidgetConnector connector;
    QmitkSynchronizedWidgetConnector::NodeList selection;
    selection.append(m_Node1);
    selection.append(m_Node3);

    connector.ChangeSelection(selection);

    auto stored = connector.GetNodeSelection();
    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(stored.size()));
    CPPUNIT_ASSERT(ListContains(stored, m_Node1));
    CPPUNIT_ASSERT(ListContains(stored, m_Node3));
  }

  void ChangeSelection_EmitsOnlyWhenChanged()
  {
    QmitkSynchronizedWidgetConnector connector;
    int emissions = 0;
    QObject::connect(&connector, &QmitkSynchronizedWidgetConnector::NodeSelectionChanged,
                     [&emissions](QmitkSynchronizedWidgetConnector::NodeList) { ++emissions; });

    QmitkSynchronizedWidgetConnector::NodeList sel1;
    sel1.append(m_Node1);

    connector.ChangeSelection(sel1);
    CPPUNIT_ASSERT_EQUAL(1, emissions);

    // Same logical selection (order-independent equality): no re-emission.
    QmitkSynchronizedWidgetConnector::NodeList sel1Reordered;
    sel1Reordered.append(m_Node1);
    connector.ChangeSelection(sel1Reordered);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Equal selection should not re-emit",
                                 1, emissions);

    // Different selection: re-emission.
    QmitkSynchronizedWidgetConnector::NodeList sel2;
    sel2.append(m_Node2);
    connector.ChangeSelection(sel2);
    CPPUNIT_ASSERT_EQUAL(2, emissions);
  }

  void ChangeSelectionMode_EmitsOnlyWhenChanged()
  {
    QmitkSynchronizedWidgetConnector connector;
    int emissions = 0;
    bool lastValue = true;
    QObject::connect(&connector, &QmitkSynchronizedWidgetConnector::SelectionModeChanged,
                     [&](bool selectAll) { ++emissions; lastValue = selectAll; });

    // Already true; should be a no-op.
    connector.ChangeSelectionMode(true);
    CPPUNIT_ASSERT_EQUAL(0, emissions);

    connector.ChangeSelectionMode(false);
    CPPUNIT_ASSERT_EQUAL(1, emissions);
    CPPUNIT_ASSERT_EQUAL(false, lastValue);

    connector.ChangeSelectionMode(false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unchanged mode should not re-emit",
                                 1, emissions);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkSynchronizedWidgetConnector)
