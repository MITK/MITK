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

#include <QApplication>
#include <QColor>
#include <QPalette>

#include <string>
#include <vector>

/**
 * Tests the canonical sync-group lifecycle API on QmitkMxNMultiWidget:
 *   - AddSynchronizationGroup throws on invalid index / null storage
 *   - AddSynchronizationGroup is idempotent
 *   - SetSynchronizationGroup auto-creates, moves between groups, and is
 *     edge-idempotent on the same group
 *   - NextFreeSyncGroupIndex picks gaps
 *   - The reserved group 'global' cannot be created, joined, removed,
 *     renamed or recoloured; its display name and a band of colours around
 *     the colours it takes are reserved as well
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
  MITK_TEST(Add_ExplicitNameTakenByOtherIndex_Throws);
  MITK_TEST(Add_InvalidExplicitName_Throws);
  MITK_TEST(Set_RejectsNullWidget);
  MITK_TEST(Set_AutoCreatesMissingGroup);
  MITK_TEST(Set_AssignsWidgetToTargetGroup);
  MITK_TEST(Set_MovesWidgetBetweenGroups);
  MITK_TEST(Set_IsIdempotentOnSameGroup);
  MITK_TEST(NextFreeSyncGroupIndex_PicksLowestUnused);
  MITK_TEST(SetSelectedPosition_UnknownWindowIsIgnored);
  MITK_TEST(Reserved_AddSynchronizationGroup_Throws);
  MITK_TEST(Reserved_SetSyncLink_ThrowsOnEveryDimension);
  MITK_TEST(Reserved_SetCellSelectionGroup_Throws);
  MITK_TEST(Reserved_RemoveSynchronizationGroup_Throws);
  MITK_TEST(Reserved_SetSyncGroupDisplayNameOfGlobal_Throws);
  MITK_TEST(Reserved_SetSyncGroupColorOfGlobal_Throws);
  MITK_TEST(Reserved_DisplayNameGlobal_Throws);
  MITK_TEST(ReservedBand_RefusesNearWhiteAndNearBlack);
  MITK_TEST(GetSyncGroupInfos_DoesNotListGlobal);
  MITK_TEST(GetSyncGroupColorAndName_GlobalIsReserved);
  MITK_TEST(GetSyncGroupColor_GlobalFollowsTheStyleSheet);
  MITK_TEST(GlobalTakesNoPaletteSlot);
  MITK_TEST(ResolveCellGroupIdentity_FreshWindowStaysMono);
  MITK_TEST(SyncAxes_AreInFamilyOrder);
  MITK_TEST(SyncAxisDimension_TimeAndSelectionHaveNone);
  MITK_TEST(BuildBarcodeSlots_NineSlotsInFamilyOrder_TimeIsGlobal);
  MITK_TEST(BuildBarcodeSlots_GlobalUsesTheOverlayColorInEveryStyle);
  MITK_TEST(CellsSharingAxis_TimeIsGlobalOnly);
  MITK_TEST(ResolveCellAxisGroup_TimeNeverTouchesSelection);
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

  void Add_ExplicitNameTakenByOtherIndex_Throws()
  {
    TestableQmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.AddSynchronizationGroup(1, "main");
    AddedRecorder rec(widget);

    CPPUNIT_ASSERT_THROW(widget.AddSynchronizationGroup(3, "main"), mitk::Exception);

    CPPUNIT_ASSERT_MESSAGE("A rejected name must not leave a connector behind",
                           nullptr == ConnectorOf(widget, 3));
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, ConnectorCount(widget));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A rejected name must not announce a group", 0, rec.count);
  }

  void Add_InvalidExplicitName_Throws()
  {
    TestableQmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    AddedRecorder rec(widget);

    CPPUNIT_ASSERT_THROW(widget.AddSynchronizationGroup(2, "not/url safe"), mitk::Exception);

    CPPUNIT_ASSERT_EQUAL(std::size_t{0}, ConnectorCount(widget));
    CPPUNIT_ASSERT_EQUAL(0, rec.count);
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

  // ---------- NextFreeSyncGroupIndex ----------

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

  void SetSelectedPosition_UnknownWindowIsIgnored()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();

    mitk::Point3D position;
    position.Fill(0.0);
    CPPUNIT_ASSERT_NO_THROW(widget.SetSelectedPosition(position, QStringLiteral("mxn__nosuchwindow")));
    CPPUNIT_ASSERT_EQUAL(1u, widget.GetNumberOfRenderWindowWidgets());
  }

  // ---------- The reserved group 'global' ----------

  /** The id of the reserved group, in the spellings that all name it. */
  static std::vector<std::string> GlobalSpellings()
  {
    return { "global", "Global", "GLOBAL", "gLoBaL" };
  }

  static QString FirstWindow(const QmitkMxNMultiWidget& widget)
  {
    return widget.ListWindowDescriptors().front().id;
  }

  void Reserved_AddSynchronizationGroup_Throws()
  {
    TestableQmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.AddSynchronizationGroup(1);
    AddedRecorder rec(widget);

    for (const auto& id : GlobalSpellings())
    {
      CPPUNIT_ASSERT_THROW_MESSAGE(id, widget.AddSynchronizationGroup(2, id), QmitkMxNReservedGroupException);
      // Naming it is a caller's bug also where the call would be idempotent.
      CPPUNIT_ASSERT_THROW_MESSAGE(id, widget.AddSynchronizationGroup(1, id), QmitkMxNReservedGroupException);
    }
    CPPUNIT_ASSERT_EQUAL(std::size_t{ 1 }, ConnectorCount(widget));
    CPPUNIT_ASSERT_EQUAL(0, rec.count);
  }

  void Reserved_SetSyncLink_ThrowsOnEveryDimension()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();
    const auto windowId = FirstWindow(widget);

    for (const auto dimension : QmitkMxNAllSyncDimensions)
    {
      const auto before = widget.GetSyncLink(windowId, dimension);
      for (const auto& id : GlobalSpellings())
      {
        CPPUNIT_ASSERT_THROW_MESSAGE(std::string(QmitkMxNSyncDimensionToLinkKey(dimension)) + " " + id,
                                     widget.SetSyncLink(windowId, dimension, id), QmitkMxNReservedGroupException);
      }
      const auto after = widget.GetSyncLink(windowId, dimension);
      CPPUNIT_ASSERT_MESSAGE(std::string("The link on ") + QmitkMxNSyncDimensionToLinkKey(dimension) + " stays",
                             before.has_value() == after.has_value()
                               && (!before.has_value() || before->group == after->group));
    }
  }

  void Reserved_SetCellSelectionGroup_Throws()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();
    const auto windowId = FirstWindow(widget);
    const auto before = widget.GetCellSelectionGroup(windowId);

    for (const auto& id : GlobalSpellings())
    {
      CPPUNIT_ASSERT_THROW_MESSAGE(id, widget.SetCellSelectionGroup(windowId, id), QmitkMxNReservedGroupException);
    }
    CPPUNIT_ASSERT_EQUAL(before, widget.GetCellSelectionGroup(windowId));
  }

  void Reserved_RemoveSynchronizationGroup_Throws()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();

    for (const auto& id : GlobalSpellings())
    {
      CPPUNIT_ASSERT_THROW_MESSAGE(id, widget.RemoveSynchronizationGroup(id), QmitkMxNReservedGroupException);
    }
  }

  void Reserved_SetSyncGroupDisplayNameOfGlobal_Throws()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();

    for (const auto& id : GlobalSpellings())
    {
      CPPUNIT_ASSERT_THROW_MESSAGE(id, widget.SetSyncGroupDisplayName(id, "Everything"),
                                   QmitkMxNReservedGroupException);
    }
    CPPUNIT_ASSERT_EQUAL(std::string("Global"), widget.GetSyncGroupDisplayName(QmitkMxNGlobalGroupId));
  }

  void Reserved_SetSyncGroupColorOfGlobal_Throws()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();
    const auto before = widget.GetSyncGroupColor(QmitkMxNGlobalGroupId);

    for (const auto& id : GlobalSpellings())
    {
      CPPUNIT_ASSERT_THROW_MESSAGE(id, widget.SetSyncGroupColor(id, QColor("#E1707A")),
                                   QmitkMxNReservedGroupException);
    }
    CPPUNIT_ASSERT(before == widget.GetSyncGroupColor(QmitkMxNGlobalGroupId));
  }

  void Reserved_DisplayNameGlobal_Throws()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();
    const auto group = widget.GetDefaultSyncGroupName();
    widget.SetSyncGroupDisplayName(group, "Tumor");

    for (const auto* name : { "Global", "global", " global ", "GLOBAL\t" })
    {
      CPPUNIT_ASSERT_THROW_MESSAGE(name, widget.SetSyncGroupDisplayName(group, name), QmitkMxNReservedGroupException);
    }
    CPPUNIT_ASSERT_EQUAL(std::string("Tumor"), widget.GetSyncGroupDisplayName(group));

    // Only the name itself is reserved.
    widget.SetSyncGroupDisplayName(group, "Global view");
    CPPUNIT_ASSERT_EQUAL(std::string("Global view"), widget.GetSyncGroupDisplayName(group));
  }

  void ReservedBand_RefusesNearWhiteAndNearBlack()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();
    const auto group = widget.GetDefaultSyncGroupName();
    widget.SetSyncGroupColor(group, QColor("#ff0000"));

    // Below a CIE76 distance of 15 to #f1f1f1 or to black.
    for (const auto* color : { "#f1f1f1", "#ffffff", "#d0d0d0", "#fff0f0", "#000000", "#202020" })
    {
      CPPUNIT_ASSERT_THROW_MESSAGE(color, widget.SetSyncGroupColor(group, QColor(color)),
                                   QmitkMxNReservedGroupException);
      CPPUNIT_ASSERT_MESSAGE(std::string("The colour stays after refusing ") + color,
                             QColor("#ff0000") == widget.GetSyncGroupColor(group));
    }

    // Just outside the band, and every default hue.
    for (const auto* color : { "#c0c0c0", "#303030", "#E1707A", "#6FA8DC", "#93C47D", "#F0B26B", "#B08FD9",
                               "#76C7C0", "#C9C97A", "#D08FB8" })
    {
      CPPUNIT_ASSERT_NO_THROW_MESSAGE(color, widget.SetSyncGroupColor(group, QColor(color)));
      CPPUNIT_ASSERT_MESSAGE(color, QColor(color) == widget.GetSyncGroupColor(group));
    }
  }

  void GetSyncGroupInfos_DoesNotListGlobal()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();

    const auto infos = widget.GetSyncGroupInfos();
    CPPUNIT_ASSERT_EQUAL(std::size_t{ 1 }, infos.size());
    CPPUNIT_ASSERT_EQUAL(widget.GetDefaultSyncGroupName(), infos.front().id);
  }

  void GetSyncGroupColorAndName_GlobalIsReserved()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();

    CPPUNIT_ASSERT_EQUAL(std::string("Global"), widget.GetSyncGroupDisplayName(QmitkMxNGlobalGroupId));

    for (const auto* text : { "#000000", "#f1f1f1", "#123456" })
    {
      auto palette = widget.palette();
      palette.setColor(QPalette::WindowText, QColor(text));
      widget.setPalette(palette);
      CPPUNIT_ASSERT_MESSAGE(std::string("The reserved group takes the style's text colour ") + text,
                             QColor(text) == widget.GetSyncGroupColor(QmitkMxNGlobalGroupId));
    }
  }

  void GetSyncGroupColor_GlobalFollowsTheStyleSheet()
  {
    // The Dark style sets the text colour in a style sheet, which reaches a
    // widget's palette only once the widget is polished.
    struct StyleSheetGuard
    {
      ~StyleSheetGuard() { qApp->setStyleSheet(QString()); }
    } guard;
    qApp->setStyleSheet(QStringLiteral("QWidget { background-color: #2d2d30; color: #f1f1f1; }"));

    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();
    CPPUNIT_ASSERT_EQUAL(std::string("#f1f1f1"),
                         widget.GetSyncGroupColor(QmitkMxNGlobalGroupId).name().toStdString());
  }

  void GlobalTakesNoPaletteSlot()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();
    widget.GetSyncGroupColor(QmitkMxNGlobalGroupId);
    widget.AddSynchronizationGroup(widget.NextFreeSyncGroupIndex());

    const auto infos = widget.GetSyncGroupInfos();
    CPPUNIT_ASSERT_EQUAL(std::size_t{ 2 }, infos.size());
    CPPUNIT_ASSERT_EQUAL(std::string("#e1707a"), infos[0].color.name().toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("#6fa8dc"), infos[1].color.name().toStdString());
  }

  void ResolveCellGroupIdentity_FreshWindowStaysMono()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();

    const auto identity = widget.ResolveCellGroupIdentity(FirstWindow(widget));
    CPPUNIT_ASSERT(QmitkMxNMultiWidget::CellGroupIdentityKind::Mono == identity.kind);
    CPPUNIT_ASSERT(widget.GetSyncGroupColor(widget.GetDefaultSyncGroupName()) == identity.hue);
  }

  // ---------- The axes and Time ----------

  void SyncAxes_AreInFamilyOrder()
  {
    using Axis = QmitkMxNSyncAxis;
    const std::vector<Axis> expected{ Axis::Orientation, Axis::Pan,       Axis::Zoom,
                                      Axis::Slice,       Axis::Time,      Axis::Crosshair,
                                      Axis::Selection,   Axis::Windowing, Axis::Lut };
    CPPUNIT_ASSERT_EQUAL(9, QmitkMxNSyncAxisCount);
    for (int slot = 0; slot < QmitkMxNSyncAxisCount; ++slot)
    {
      const auto axis = expected[static_cast<std::size_t>(slot)];
      CPPUNIT_ASSERT_MESSAGE("slot " + std::to_string(slot), axis == QmitkMxNAllSyncAxes[static_cast<std::size_t>(slot)]);
      CPPUNIT_ASSERT_EQUAL(slot, QmitkMxNSyncAxisToSlot(axis));
      CPPUNIT_ASSERT(axis == *QmitkMxNSyncAxisFromSlot(slot));
    }
    CPPUNIT_ASSERT(!QmitkMxNSyncAxisFromSlot(QmitkMxNSyncAxisCount).has_value());
  }

  void SyncAxisDimension_TimeAndSelectionHaveNone()
  {
    CPPUNIT_ASSERT(!QmitkMxNSyncAxisDimension(QmitkMxNSyncAxis::Time).has_value());
    CPPUNIT_ASSERT(!QmitkMxNSyncAxisDimension(QmitkMxNSyncAxis::Selection).has_value());
    for (const auto dimension : QmitkMxNAllSyncDimensions)
    {
      const auto axis = QmitkMxNSyncAxisOf(dimension);
      CPPUNIT_ASSERT(dimension == *QmitkMxNSyncAxisDimension(axis));
      CPPUNIT_ASSERT_EQUAL(std::string(QmitkMxNSyncDimensionToLinkKey(dimension)),
                           std::string(QmitkMxNSyncAxisToLinkKey(axis)));
    }
    CPPUNIT_ASSERT_EQUAL(std::string("time"), std::string(QmitkMxNSyncAxisToLinkKey(QmitkMxNSyncAxis::Time)));
    CPPUNIT_ASSERT_EQUAL(std::string("selection"),
                         std::string(QmitkMxNSyncAxisToLinkKey(QmitkMxNSyncAxis::Selection)));
  }

  void BuildBarcodeSlots_NineSlotsInFamilyOrder_TimeIsGlobal()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();

    const auto axisSlots = widget.BuildBarcodeSlots(FirstWindow(widget));
    CPPUNIT_ASSERT_EQUAL(QmitkMxNSyncAxisCount, static_cast<int>(axisSlots.size()));
    for (const auto axis : QmitkMxNAllSyncAxes)
    {
      CPPUNIT_ASSERT(axis == axisSlots[QmitkMxNSyncAxisToSlot(axis)].axis);
    }

    const auto& time = axisSlots[QmitkMxNSyncAxisToSlot(QmitkMxNSyncAxis::Time)];
    CPPUNIT_ASSERT_EQUAL(std::string("#f1f1f1"), time.color.name().toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("Time"), time.label.toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("Time - group Global"), time.tooltip.toStdString());
    CPPUNIT_ASSERT(!time.hasOffset && time.offsetText.isEmpty());
    CPPUNIT_ASSERT(time.applicable);
  }

  void BuildBarcodeSlots_GlobalUsesTheOverlayColorInEveryStyle()
  {
    // The strip and the plates are dark in every style, so they keep the light
    // gray while the style's own text colour is black.
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();
    auto palette = widget.palette();
    palette.setColor(QPalette::WindowText, QColor(Qt::black));
    widget.setPalette(palette);

    CPPUNIT_ASSERT_EQUAL(std::string("#000000"), widget.GetSyncGroupColor(QmitkMxNGlobalGroupId).name().toStdString());
    const auto axisSlots = widget.BuildBarcodeSlots(FirstWindow(widget));
    CPPUNIT_ASSERT_EQUAL(std::string("#f1f1f1"),
                         axisSlots[QmitkMxNSyncAxisToSlot(QmitkMxNSyncAxis::Time)].color.name().toStdString());
  }

  void CellsSharingAxis_TimeIsGlobalOnly()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();
    widget.SetLayout(2, 2);

    QStringList every;
    for (const auto& descriptor : widget.ListWindowDescriptors())
    {
      every.append(descriptor.id);
    }
    CPPUNIT_ASSERT_EQUAL(4, static_cast<int>(every.size()));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Every window, in pre-order, shares Time through 'global'",
                                 every.join(QLatin1Char(',')).toStdString(),
                                 widget.CellsSharingAxis(QStringLiteral("global"), QmitkMxNSyncAxis::Time)
                                   .join(QLatin1Char(',')).toStdString());
    CPPUNIT_ASSERT_MESSAGE("A user group shares no Time",
                           widget.CellsSharingAxis(QString::fromStdString(widget.GetDefaultSyncGroupName()),
                                                   QmitkMxNSyncAxis::Time).isEmpty());
    CPPUNIT_ASSERT_MESSAGE("'global' takes no other axis yet",
                           widget.CellsSharingAxis(QStringLiteral("global"), QmitkMxNSyncAxis::Selection).isEmpty());
  }

  void ResolveCellAxisGroup_TimeNeverTouchesSelection()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();
    widget.SetLayout(1, 2);
    const auto descriptors = widget.ListWindowDescriptors();
    widget.SetCellSelectionGroup(descriptors.front().id, "tumor");

    for (const auto& descriptor : descriptors)
    {
      const auto selectionBefore = widget.GetCellSelectionGroup(descriptor.id);
      CPPUNIT_ASSERT_EQUAL(std::string("global"),
                           widget.ResolveCellAxisGroup(descriptor.id, QmitkMxNSyncAxis::Time));
      CPPUNIT_ASSERT_EQUAL(selectionBefore, widget.GetCellSelectionGroup(descriptor.id));
    }
    CPPUNIT_ASSERT_EQUAL(std::string("tumor"),
                         widget.ResolveCellAxisGroup(descriptors.front().id, QmitkMxNSyncAxis::Selection));
    CPPUNIT_ASSERT_MESSAGE("An unknown window has no Time group",
                           widget.ResolveCellAxisGroup(QStringLiteral("mxn__nosuch"), QmitkMxNSyncAxis::Time).empty());
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNSyncGroupApi)
