/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNMultiWidget.h>
#include <QmitkSynchronizedNodeSelectionWidget.h>
#include <QmitkSynchronizedWidgetConnector.h>

#include <mitkException.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

/**
 * Tests the canonical sync-group lifecycle API on QmitkMxNMultiWidget:
 *   - AddSynchronizationGroup throws on invalid index / null storage
 *   - AddSynchronizationGroup is idempotent
 *   - SetSynchronizationGroup auto-creates, moves between groups, and is
 *     edge-idempotent on the same group
 *   - NextFreeSyncGroupIndex / OnCreateNewSyncGroupRequested pick gaps
 *
 * The test surfaces the protected 'GetSyncGroupConnector' / 'GetSyncGroupCount'
 * accessors via a thin subclass to inspect connector identity and registered
 * group count without leaking these into the production public API. Connector
 * state (m_SelectAll auto-reset when the connection counter drops to zero) is
 * used to detect double-Connect regressions without probing the counter.
 */
class TestableQmitkMxNMultiWidget : public QmitkMxNMultiWidget
{
public:
  using QmitkMxNMultiWidget::QmitkMxNMultiWidget;
  using QmitkMxNMultiWidget::GetSyncGroupConnector;
  using QmitkMxNMultiWidget::GetSyncGroupCount;
};

class QmitkMxNSyncGroupApiTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNSyncGroupApiTestSuite);
  MITK_TEST(Add_RejectsZero);
  MITK_TEST(Add_RejectsNegative);
  MITK_TEST(Add_ThrowsWithoutDataStorage);
  MITK_TEST(Add_EmitsSignalOnFirstCall);
  MITK_TEST(Add_IsIdempotent);
  MITK_TEST(Set_RejectsNullWidget);
  MITK_TEST(Set_AutoCreatesMissingGroup);
  MITK_TEST(Set_AssignsWidgetToTargetGroup);
  MITK_TEST(Set_MovesWidgetBetweenGroups);
  MITK_TEST(Set_IsIdempotentOnSameGroup);
  MITK_TEST(NextFreeSyncGroupIndex_PicksLowestUnused);
  MITK_TEST(OnCreateNewSyncGroupRequested_AssignsNextFree);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer m_Node1;
  mitk::DataNode::Pointer m_Node2;

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
    //       'QmitkSynchronizedWidgetConnectorTest.cpp' -- keep both in sync
    //       and remove together.
    m_Node1 = mitk::DataNode::New();
    m_Node1->SetName("node1");
    m_Node1->SetIntProperty("layer", 0);
    m_DataStorage->Add(m_Node1);

    m_Node2 = mitk::DataNode::New();
    m_Node2->SetName("node2");
    m_Node2->SetIntProperty("layer", 1);
    m_DataStorage->Add(m_Node2);
  }

  void tearDown() override
  {
    m_Node1 = nullptr;
    m_Node2 = nullptr;
    m_DataStorage = nullptr;
  }

  /**
   * Records SyncGroupAdded emissions via a lambda connection. Avoids the
   * QSignalSpy dependency on Qt::Test (not linked into MitkQtWidgets).
   */
  struct AddedRecorder
  {
    int count = 0;
    int lastIndex = -1;
    QMetaObject::Connection conn;

    explicit AddedRecorder(QmitkMxNMultiWidget& widget)
    {
      conn = QObject::connect(&widget, &QmitkMxNMultiWidget::SyncGroupAdded,
                              [this](int index) { ++count; lastIndex = index; });
    }
    ~AddedRecorder() { QObject::disconnect(conn); }
  };

  /** Returns the connector for `index` or nullptr if no group exists. */
  static QmitkSynchronizedWidgetConnector* ConnectorOf(const TestableQmitkMxNMultiWidget& w,
                                                       int index)
  {
    return w.GetSyncGroupConnector(index);
  }

  static std::size_t ConnectorCount(const TestableQmitkMxNMultiWidget& w)
  {
    return w.GetSyncGroupCount();
  }

  // ---------- Add ----------

  void Add_RejectsZero()
  {
    TestableQmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    AddedRecorder rec(widget);

    CPPUNIT_ASSERT_THROW(widget.AddSynchronizationGroup(0), mitk::Exception);
    CPPUNIT_ASSERT_EQUAL(std::size_t{0}, ConnectorCount(widget));
    CPPUNIT_ASSERT_EQUAL(0, rec.count);
  }

  void Add_RejectsNegative()
  {
    TestableQmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    AddedRecorder rec(widget);

    CPPUNIT_ASSERT_THROW(widget.AddSynchronizationGroup(-1), mitk::Exception);
    CPPUNIT_ASSERT_EQUAL(std::size_t{0}, ConnectorCount(widget));
    CPPUNIT_ASSERT_EQUAL(0, rec.count);
  }

  void Add_ThrowsWithoutDataStorage()
  {
    TestableQmitkMxNMultiWidget widget;
    // Deliberately do not set a data storage.
    CPPUNIT_ASSERT_THROW(widget.AddSynchronizationGroup(1), mitk::Exception);
    CPPUNIT_ASSERT_EQUAL(std::size_t{0}, ConnectorCount(widget));
  }

  void Add_EmitsSignalOnFirstCall()
  {
    TestableQmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    AddedRecorder rec(widget);

    widget.AddSynchronizationGroup(7);

    CPPUNIT_ASSERT_EQUAL(1, rec.count);
    CPPUNIT_ASSERT_EQUAL(7, rec.lastIndex);
    CPPUNIT_ASSERT(nullptr != ConnectorOf(widget, 7));
  }

  void Add_IsIdempotent()
  {
    TestableQmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    AddedRecorder rec(widget);

    widget.AddSynchronizationGroup(1);
    auto* const firstConnector = ConnectorOf(widget, 1);
    CPPUNIT_ASSERT(nullptr != firstConnector);

    // Mutate connector state so a silent replacement would be observable.
    firstConnector->ChangeSelectionMode(false);
    CPPUNIT_ASSERT_EQUAL(false, firstConnector->GetSelectionMode());

    widget.AddSynchronizationGroup(1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Idempotent Add must not re-emit",
                                 1, rec.count);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Map size must remain 1",
                                 std::size_t{1}, ConnectorCount(widget));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Connector instance must be preserved",
                                 firstConnector, ConnectorOf(widget, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Connector state must be preserved",
                                 false, firstConnector->GetSelectionMode());
  }

  // ---------- Set ----------

  void Set_RejectsNullWidget()
  {
    TestableQmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    CPPUNIT_ASSERT_THROW(widget.SetSynchronizationGroup(nullptr, 1), mitk::Exception);
  }

  void Set_AutoCreatesMissingGroup()
  {
    TestableQmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    AddedRecorder rec(widget);

    QmitkSynchronizedNodeSelectionWidget nodeWidget(nullptr);
    nodeWidget.SetDataStorage(m_DataStorage);

    widget.SetSynchronizationGroup(&nodeWidget, 5);

    CPPUNIT_ASSERT_EQUAL(1, rec.count);
    CPPUNIT_ASSERT_EQUAL(5, rec.lastIndex);
    CPPUNIT_ASSERT(nullptr != ConnectorOf(widget, 5));
  }

  void Set_AssignsWidgetToTargetGroup()
  {
    TestableQmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    QmitkSynchronizedNodeSelectionWidget nodeWidget(nullptr);
    nodeWidget.SetDataStorage(m_DataStorage);

    widget.SetSynchronizationGroup(&nodeWidget, 3);

    CPPUNIT_ASSERT_EQUAL(3, nodeWidget.GetSyncGroup());
  }

  void Set_MovesWidgetBetweenGroups()
  {
    TestableQmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    QmitkSynchronizedNodeSelectionWidget nodeWidget(nullptr);
    nodeWidget.SetDataStorage(m_DataStorage);

    widget.SetSynchronizationGroup(&nodeWidget, 1);
    CPPUNIT_ASSERT_EQUAL(1, nodeWidget.GetSyncGroup());

    widget.SetSynchronizationGroup(&nodeWidget, 2);
    CPPUNIT_ASSERT_EQUAL(2, nodeWidget.GetSyncGroup());

    auto* const g1 = ConnectorOf(widget, 1);
    auto* const g2 = ConnectorOf(widget, 2);
    CPPUNIT_ASSERT(nullptr != g1);
    CPPUNIT_ASSERT(nullptr != g2);

    // Toggling the target group's mode should propagate to the widget.
    g2->ChangeSelectionMode(false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Widget must follow its target group",
                                 false, nodeWidget.GetSelectAll());

    // Reset; toggling the old group must NOT propagate to the moved widget.
    nodeWidget.SetSelectAll(true);
    g1->ChangeSelectionMode(false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Widget must be disconnected from its old group",
                                 true, nodeWidget.GetSelectAll());
  }

  void Set_IsIdempotentOnSameGroup()
  {
    TestableQmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    QmitkSynchronizedNodeSelectionWidget nodeWidget(nullptr);
    nodeWidget.SetDataStorage(m_DataStorage);

    widget.SetSynchronizationGroup(&nodeWidget, 1);
    auto* const connector = ConnectorOf(widget, 1);
    CPPUNIT_ASSERT(nullptr != connector);

    // Drive m_SelectAll false. After ONE legitimate Disconnect the connection
    // counter must reach zero and the connector auto-rearms m_SelectAll = true
    // (see QmitkSynchronizedWidgetConnector::DeregisterWidget). If Set had
    // double-Connected, one disconnect would leave the counter at 1 and
    // m_SelectAll would stay false.
    connector->ChangeSelectionMode(false);
    CPPUNIT_ASSERT_EQUAL(false, connector->GetSelectionMode());

    widget.SetSynchronizationGroup(&nodeWidget, 1);  // idempotent
    widget.SetSynchronizationGroup(&nodeWidget, 1);  // idempotent again

    widget.SetSynchronizationGroup(&nodeWidget, 2);  // single Disconnect on group 1

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "A single Disconnect must drive group-1's counter to zero "
      "(no double-Connect)",
      true, connector->GetSelectionMode());
  }

  // ---------- NextFreeSyncGroupIndex / OnCreateNewSyncGroupRequested ----------

  void NextFreeSyncGroupIndex_PicksLowestUnused()
  {
    TestableQmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    CPPUNIT_ASSERT_EQUAL(1, widget.NextFreeSyncGroupIndex());

    widget.AddSynchronizationGroup(1);
    CPPUNIT_ASSERT_EQUAL(2, widget.NextFreeSyncGroupIndex());

    widget.AddSynchronizationGroup(3);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should fill the gap at 2",
                                 2, widget.NextFreeSyncGroupIndex());

    widget.AddSynchronizationGroup(2);
    CPPUNIT_ASSERT_EQUAL(4, widget.NextFreeSyncGroupIndex());
  }

  void OnCreateNewSyncGroupRequested_AssignsNextFree()
  {
    TestableQmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    widget.AddSynchronizationGroup(1);
    widget.AddSynchronizationGroup(3);

    QmitkSynchronizedNodeSelectionWidget nodeWidget(nullptr);
    nodeWidget.SetDataStorage(m_DataStorage);

    widget.OnCreateNewSyncGroupRequested(&nodeWidget);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should pick gap index 2, not 4",
                                 2, nodeWidget.GetSyncGroup());
    CPPUNIT_ASSERT(nullptr != ConnectorOf(widget, 2));
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNSyncGroupApi)
