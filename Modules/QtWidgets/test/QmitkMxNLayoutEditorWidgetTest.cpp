/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNCellMapWidget.h>
#include <QmitkMxNGroupJoinMode.h>
#include <QmitkMxNLayoutEditorWidget.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkMxNSyncBarcodeWidget.h>
#include <QmitkRenderWindowUtilityWidget.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkException.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QCoreApplication>
#include <QLayout>

#include <algorithm>

/**
 * Drives the layout editor widget's mutation API against a real
 * QmitkMxNMultiWidget (no delegate, no mocks) and asserts the resulting
 * engine link state - the widget-level guarantee that survives the interim
 * sync popup's removal. The underlying engine behavior itself is covered by
 * QmitkMxNSyncGroupApiTest / QmitkMxNNavLinksTest.
 */
class QmitkMxNLayoutEditorWidgetTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNLayoutEditorWidgetTestSuite);

  MITK_TEST(CreateGroup_RegistersEngineGroup);
  MITK_TEST(Join_EmptyGroup_LinksNavigationBundleByDefault);
  MITK_TEST(Join_GroupWithDimensions_LinksThoseDimensions);
  MITK_TEST(AssignCells_JoinsEveryGivenCell);
  MITK_TEST(Leave_ClearsEveryDimension);
  MITK_TEST(ApplyDimension_TogglesForAllMembers);
  MITK_TEST(GroupBarcode_TriStateAllNoneSome);
  MITK_TEST(EditorChange_RefreshesCellBarcode);
  MITK_TEST(Selection_TogglesViaEditorAndRoundTrips);
  MITK_TEST(Selection_MoveIsSingleValued);
  MITK_TEST(Rebuild_PopulatesWithoutTouchingLinks);
  MITK_TEST(LayoutShrink_EmitsLayoutChanged);
  MITK_TEST(IncrementalCards_AddKeepsExistingCards);
  MITK_TEST(IncrementalCards_RemoveDropsOnlyItsCard);
  MITK_TEST(IncrementalCards_OrderMatchesEngine);
  MITK_TEST(EmptyGroupCache_TogglesWithoutTouchingEngine);
  MITK_TEST(EmptyGroupCache_AppliedOnFirstAssignmentThenCleared);
  MITK_TEST(EmptyGroup_AxisClickCaches_EvenWithSelection);

  MITK_TEST(NewCell_DefaultLinksWindowingAndLutToMain);
  MITK_TEST(NewCell_FrameIsMonoMain);
  MITK_TEST(Frame_SelectionDivergesFromNav_IsComplex);
  MITK_TEST(Frame_WhollyOneGroup_IsMono);
  MITK_TEST(AssignReplace_ClearsOtherGroupLinks);
  MITK_TEST(WidenedMembership_SelectionTieCountsAsMember);
  MITK_TEST(MainCard_Live_AxisClickHomogenizes);
  MITK_TEST(AssignReplace_ReclaimsEmptiedSelectionGroup);
  MITK_TEST(AssignReplace_MultiCellReclaim_OnlyWhenLastLeaves);
  MITK_TEST(MainCard_LinkNavigation_LinksAllCells);
  MITK_TEST(NewCell_BarcodePaintsAppearanceDefault);
  MITK_TEST(LoadedLayout_NoWindowingInjection);

  MITK_TEST(Assign_ReplaceMode_ClearsOthers);
  MITK_TEST(Assign_FillEmptyMode_KeepsLinkedAxes);
  MITK_TEST(Assign_FillEmptyMode_SelectionOnlyWhenResting);
  MITK_TEST(Assign_MergeMode_OverwritesCoveredKeepsRest);
  MITK_TEST(JoinModeFromModifiers_MapsKeys);
  MITK_TEST(Selection_TogglesForUserGroup);
  MITK_TEST(MultiSelect_SurvivesActiveMirror);

  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  std::unique_ptr<QmitkMxNMultiWidget> m_Editor;
  std::unique_ptr<QmitkMxNLayoutEditorWidget> m_Widget;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();

    m_Editor = std::make_unique<QmitkMxNMultiWidget>();
    m_Editor->SetDataStorage(m_DataStorage);
    m_Editor->InitializeMultiWidget();
    m_Editor->SetLayout(1, 3);  // cells: mxn__widget0 .. mxn__widget2

    m_Widget = std::make_unique<QmitkMxNLayoutEditorWidget>();
    m_Widget->SetMultiWidget(m_Editor.get());
  }

  void tearDown() override
  {
    m_Widget.reset();
    m_Editor.reset();
    m_DataStorage = nullptr;
  }

  static QString CellId(std::size_t index)
  {
    return QStringLiteral("mxn__widget") + QString::number(index);
  }

  bool IsLinked(std::size_t cell, QmitkMxNSyncDimension dimension, const std::string& group) const
  {
    const auto link = m_Editor->GetSyncLink(CellId(cell), dimension);
    return link.has_value() && link->group == group;
  }

  /** Fire the coalesced, QTimer::singleShot(0)-deferred card rebuild. Twice, so a
   *  refresh that schedules follow-up work still settles. */
  static void Pump()
  {
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
  }

  /** The barcode axis index of a sync dimension (selection is the last axis). */
  static int AxisIndexOf(QmitkMxNSyncDimension dimension)
  {
    for (std::size_t i = 0; i < QmitkMxNAllSyncDimensions.size(); ++i)
    {
      if (QmitkMxNAllSyncDimensions[i] == dimension)
      {
        return static_cast<int>(i);
      }
    }
    return -1;
  }

  /** The group's card widget by its stable object name, or nullptr. */
  QWidget* CardFor(const std::string& groupId) const
  {
    return m_Widget->findChild<QWidget*>(
      QStringLiteral("mxnGroupCard__") + QString::fromStdString(groupId));
  }

  /** The group ids of the cards in the order they sit in the card layout, read
   *  through the public layout of the container a card is parented to. */
  QStringList CardOrder() const
  {
    const QString prefix = QStringLiteral("mxnGroupCard__");
    QWidget* container = nullptr;
    for (auto* w : m_Widget->findChildren<QWidget*>())
    {
      if (w->objectName().startsWith(prefix))
      {
        container = w->parentWidget();
        break;
      }
    }
    QStringList order;
    if (nullptr == container || nullptr == container->layout())
    {
      return order;
    }
    auto* layout = container->layout();
    for (int i = 0; i < layout->count(); ++i)
    {
      auto* item = layout->itemAt(i);
      auto* w = (nullptr == item) ? nullptr : item->widget();
      if (nullptr != w && w->objectName().startsWith(prefix))
      {
        order << w->objectName().mid(prefix.length());
      }
    }
    return order;
  }

  void CreateGroup_RegistersEngineGroup()
  {
    const auto id = m_Widget->CreateGroup();
    CPPUNIT_ASSERT(!id.empty());

    const auto infos = m_Editor->GetSyncGroupInfos();
    const auto found = std::any_of(infos.begin(), infos.end(),
                                   [&id](const auto& info) { return info.id == id; });
    CPPUNIT_ASSERT_MESSAGE("A created group must appear in the group registry", found);
  }

  void Join_EmptyGroup_LinksNavigationBundleByDefault()
  {
    const auto id = m_Widget->CreateGroup();

    m_Widget->SetCellMembership(CellId(0), id, true);
    m_Widget->SetCellMembership(CellId(1), id, true);

    for (std::size_t cell : { std::size_t(0), std::size_t(1) })
    {
      for (const auto dimension : { QmitkMxNSyncDimension::Pan, QmitkMxNSyncDimension::Zoom,
                                    QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Crosshair })
      {
        CPPUNIT_ASSERT_MESSAGE("Joining an empty group links the navigation bundle",
                               IsLinked(cell, dimension, id));
      }
      CPPUNIT_ASSERT_MESSAGE("Windowing is not part of the navigation bundle",
                             !IsLinked(cell, QmitkMxNSyncDimension::Windowing, id));
    }
    CPPUNIT_ASSERT_MESSAGE("A non-member stays unlinked",
                           !m_Editor->GetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice).has_value());
  }

  void Join_GroupWithDimensions_LinksThoseDimensions()
  {
    // Group already synchronizes windowing only (established via cell 0).
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "wl");

    m_Widget->SetCellMembership(CellId(1), "wl", true);

    CPPUNIT_ASSERT(IsLinked(1, QmitkMxNSyncDimension::Windowing, "wl"));
    CPPUNIT_ASSERT_MESSAGE("Joining follows the group's existing dimension set, not the bundle",
                           !IsLinked(1, QmitkMxNSyncDimension::Slice, "wl"));
  }

  void AssignCells_JoinsEveryGivenCell()
  {
    // The map's drag-and-drop / assign-selection path.
    const auto id = m_Widget->CreateGroup();

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(2) }, id);

    CPPUNIT_ASSERT(IsLinked(0, QmitkMxNSyncDimension::Slice, id));
    CPPUNIT_ASSERT(IsLinked(2, QmitkMxNSyncDimension::Slice, id));
    CPPUNIT_ASSERT_MESSAGE("The unassigned cell stays unlinked",
                           !m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice).has_value());
  }

  void Leave_ClearsEveryDimension()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "other");

    m_Widget->SetCellMembership(CellId(0), "nav", false);

    CPPUNIT_ASSERT(!m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice).has_value());
    CPPUNIT_ASSERT(!m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing).has_value());
    CPPUNIT_ASSERT_MESSAGE("Links to other groups survive leaving",
                           IsLinked(0, QmitkMxNSyncDimension::Zoom, "other"));
  }

  void ApplyDimension_TogglesForAllMembers()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav");

    m_Widget->ApplyDimensionToGroup("nav", QmitkMxNSyncDimension::Windowing, true);
    CPPUNIT_ASSERT(IsLinked(0, QmitkMxNSyncDimension::Windowing, "nav"));
    CPPUNIT_ASSERT(IsLinked(1, QmitkMxNSyncDimension::Windowing, "nav"));
    CPPUNIT_ASSERT_MESSAGE("A non-member keeps its default 'main' windowing link, untouched by the group toggle",
                           IsLinked(2, QmitkMxNSyncDimension::Windowing, "main"));

    m_Widget->ApplyDimensionToGroup("nav", QmitkMxNSyncDimension::Windowing, false);
    CPPUNIT_ASSERT(!m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing).has_value());
    CPPUNIT_ASSERT(!m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Windowing).has_value());
    CPPUNIT_ASSERT_MESSAGE("Disabling a dimension keeps the membership dimension",
                           IsLinked(0, QmitkMxNSyncDimension::Slice, "nav"));
  }

  void GroupBarcode_TriStateAllNoneSome()
  {
    // The group perspective is tri-state per axis over the group's members:
    // all linked, none linked, or some. Link Slice on all three cells, Pan on
    // only two, and leave Windowing unlinked; the group header's slots must read
    // solid / dashed / gap respectively.
    const auto id = m_Widget->CreateGroup();
    for (std::size_t cell = 0; cell < 3; ++cell)
    {
      m_Editor->SetSyncLink(CellId(cell), QmitkMxNSyncDimension::Slice, id);
    }
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan, id);
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Pan, id);

    const auto slotIndex = [](QmitkMxNSyncDimension dimension)
    {
      for (std::size_t i = 0; i < QmitkMxNAllSyncDimensions.size(); ++i)
      {
        if (QmitkMxNAllSyncDimensions[i] == dimension)
        {
          return static_cast<int>(i);
        }
      }
      return -1;
    };

    const auto axisSlots = m_Widget->BuildGroupBarcodeSlots(id);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(QmitkMxNAllSyncDimensions.size()) + 1,
                         static_cast<int>(axisSlots.size()));

    const auto& sliceSlot = axisSlots[slotIndex(QmitkMxNSyncDimension::Slice)];
    CPPUNIT_ASSERT_MESSAGE("all members linked on Slice -> solid",
                           sliceSlot.color.isValid() && !sliceSlot.partial);

    const auto& panSlot = axisSlots[slotIndex(QmitkMxNSyncDimension::Pan)];
    CPPUNIT_ASSERT_MESSAGE("some but not all members linked on Pan -> partial",
                           panSlot.color.isValid() && panSlot.partial);

    const auto& windowingSlot = axisSlots[slotIndex(QmitkMxNSyncDimension::Windowing)];
    CPPUNIT_ASSERT_MESSAGE("no members linked on Windowing -> gap",
                           !windowingSlot.color.isValid());
  }

  void EditorChange_RefreshesCellBarcode()
  {
    // Regression: an edit made in the layout editor must refresh the per-cell
    // utility-strip barcode, not just the editor's own view. The editor's
    // mutators route through RefreshSyncControls (SyncLinksChanged) for that;
    // before the fix they called only the editor-local rebuild, leaving the
    // barcode stale.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Widget->ApplyDimensionToGroup("nav", QmitkMxNSyncDimension::Windowing, true);

    const auto cell = m_Editor->GetRenderWindowWidget(CellId(0));
    CPPUNIT_ASSERT(nullptr != cell);
    auto* utility = cell->GetUtilityWidget();
    CPPUNIT_ASSERT(nullptr != utility);
    auto* barcode = utility->findChild<QmitkMxNSyncBarcodeWidget*>();
    CPPUNIT_ASSERT(nullptr != barcode);

    int windowingSlot = -1;
    for (std::size_t i = 0; i < QmitkMxNAllSyncDimensions.size(); ++i)
    {
      if (QmitkMxNAllSyncDimensions[i] == QmitkMxNSyncDimension::Windowing)
      {
        windowingSlot = static_cast<int>(i);
        break;
      }
    }
    CPPUNIT_ASSERT(windowingSlot >= 0);

    const auto axisSlots = barcode->Slots();
    CPPUNIT_ASSERT_MESSAGE(
      "An editor dimension toggle must refresh the cell's barcode without a manual refresh",
      axisSlots[windowingSlot].color.isValid());
  }

  void Selection_TogglesViaEditorAndRoundTrips()
  {
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, id);  // nav-links them

    m_Widget->ApplySelectionToGroup(id, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Enabling selection joins the members' selection group",
                                 id, m_Editor->GetCellSelectionGroup(CellId(0)));
    CPPUNIT_ASSERT_EQUAL(id, m_Editor->GetCellSelectionGroup(CellId(1)));
    CPPUNIT_ASSERT_MESSAGE("A non-member keeps the default selection group",
                           m_Editor->GetCellSelectionGroup(CellId(2)) != id);

    // The selection group survives a layout-document round-trip like any axis.
    const auto doc = m_Editor->SerializeLayout();
    m_Editor->ApplyLayout(doc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Selection group survives serialize/apply",
                                 id, m_Editor->GetCellSelectionGroup(CellId(0)));
    CPPUNIT_ASSERT_EQUAL(id, m_Editor->GetCellSelectionGroup(CellId(1)));

    m_Widget->ApplySelectionToGroup(id, false);
    CPPUNIT_ASSERT_MESSAGE("Disabling selection returns the cell to the default group",
                           m_Editor->GetCellSelectionGroup(CellId(0)) != id);
  }

  void Selection_MoveIsSingleValued()
  {
    // Cell 0 belongs to two groups on different dimensions, so it is a member of
    // both for the selection toggle.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan, "A");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "B");

    m_Widget->ApplySelectionToGroup("A", true);
    CPPUNIT_ASSERT_EQUAL(std::string("A"), m_Editor->GetCellSelectionGroup(CellId(0)));

    // Selection is single-valued per cell: enabling it on B moves the cell.
    m_Widget->ApplySelectionToGroup("B", true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The cell's single selection group moves A -> B",
                                 std::string("B"), m_Editor->GetCellSelectionGroup(CellId(0)));
  }

  void Rebuild_PopulatesWithoutTouchingLinks()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav");

    // Let the coalesced rebuild run (registered via SetMultiWidget and the
    // engine's change signals).
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();

    CPPUNIT_ASSERT_MESSAGE("A rebuild must not mutate engine state",
                           IsLinked(0, QmitkMxNSyncDimension::Slice, "nav")
                             && IsLinked(1, QmitkMxNSyncDimension::Slice, "nav"));
  }

  void LayoutShrink_EmitsLayoutChanged()
  {
    // Removing cells through the layout controls must notify layout-tracking
    // furniture; a silent shrink leaves e.g. the layout editor rendering
    // dead cells.
    int layoutChanges = 0;
    const auto conn = QObject::connect(m_Editor.get(), &QmitkMxNMultiWidget::LayoutChanged,
                                       [&layoutChanges]() { ++layoutChanges; });

    m_Editor->SetLayout(1, 2);

    QObject::disconnect(conn);
    CPPUNIT_ASSERT_MESSAGE("Shrinking the layout must emit LayoutChanged", layoutChanges >= 1);
  }

  void IncrementalCards_AddKeepsExistingCards()
  {
    Pump();
    CPPUNIT_ASSERT(nullptr != CardFor("main"));

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, "alpha");
    Pump();
    auto* mainCard = CardFor("main");
    auto* alphaCard = CardFor("alpha");
    CPPUNIT_ASSERT(nullptr != mainCard);
    CPPUNIT_ASSERT(nullptr != alphaCard);

    // Adding a third group must not destroy or recreate the existing cards.
    m_Widget->AssignCellsToGroup(QStringList{ CellId(1) }, "beta");
    Pump();
    CPPUNIT_ASSERT_MESSAGE("The 'main' card must survive an add", mainCard == CardFor("main"));
    CPPUNIT_ASSERT_MESSAGE("The 'alpha' card must survive an add", alphaCard == CardFor("alpha"));
    CPPUNIT_ASSERT_MESSAGE("The new 'beta' card must exist", nullptr != CardFor("beta"));
  }

  void IncrementalCards_RemoveDropsOnlyItsCard()
  {
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, "alpha");
    m_Widget->AssignCellsToGroup(QStringList{ CellId(1) }, "beta");
    Pump();
    auto* mainCard = CardFor("main");
    auto* betaCard = CardFor("beta");
    CPPUNIT_ASSERT(nullptr != CardFor("alpha"));
    CPPUNIT_ASSERT(nullptr != mainCard);
    CPPUNIT_ASSERT(nullptr != betaCard);

    // Unlinking alpha's only member drops it from the group set.
    m_Widget->SetCellMembership(CellId(0), "alpha", false);
    Pump();
    CPPUNIT_ASSERT_MESSAGE("The removed group's card must be gone", nullptr == CardFor("alpha"));
    CPPUNIT_ASSERT_MESSAGE("Other cards survive a remove (moved, not recreated)",
                           mainCard == CardFor("main"));
    CPPUNIT_ASSERT_MESSAGE("Other cards survive a remove (moved, not recreated)",
                           betaCard == CardFor("beta"));
  }

  void IncrementalCards_OrderMatchesEngine()
  {
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, "alpha");
    m_Widget->AssignCellsToGroup(QStringList{ CellId(1) }, "beta");
    Pump();

    QStringList expected;
    for (const auto& info : m_Editor->GetSyncGroupInfos())
    {
      expected << QString::fromStdString(info.id);
    }
    CPPUNIT_ASSERT_EQUAL(expected.join(QStringLiteral(",")).toStdString(),
                         CardOrder().join(QStringLiteral(",")).toStdString());
  }

  void EmptyGroupCache_TogglesWithoutTouchingEngine()
  {
    const auto group = m_Widget->CreateGroup();
    CPPUNIT_ASSERT(!group.empty());
    Pump();

    // No map selection (nothing set the active window since attach), so an
    // axis click on this empty group toggles the intent cache, not the engine.
    const int sliceAxis = AxisIndexOf(QmitkMxNSyncDimension::Slice);
    const int windowingAxis = AxisIndexOf(QmitkMxNSyncDimension::Windowing);
    m_Widget->ToggleGroupAxis(group, sliceAxis);
    m_Widget->ToggleGroupAxis(group, windowingAxis);

    // The barcode reflects exactly those two axes as linked (a valid hue).
    const auto barcodeSlots = m_Widget->BuildGroupBarcodeSlots(group);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(QmitkMxNAllSyncDimensions.size()) + 1,
                         static_cast<int>(barcodeSlots.size()));
    for (int i = 0; i < barcodeSlots.size(); ++i)
    {
      const bool expectedOn = (i == sliceAxis || i == windowingAxis);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Cache toggle sets exactly the toggled axes",
                                   expectedOn, barcodeSlots[i].color.isValid());
    }

    // The engine is untouched: no cell links or selects the group. (If the map
    // had a selection, the click would have bootstrapped and this would fail.)
    for (std::size_t cell = 0; cell < 3; ++cell)
    {
      for (const auto dimension : QmitkMxNAllSyncDimensions)
      {
        CPPUNIT_ASSERT_MESSAGE("Cache toggle must not mutate the engine",
                               !IsLinked(cell, dimension, group));
      }
      CPPUNIT_ASSERT_MESSAGE("Cache toggle must not set a selection group",
                             m_Editor->GetCellSelectionGroup(CellId(cell)) != group);
    }

    // Toggling both axes back off returns the barcode to all-gap.
    m_Widget->ToggleGroupAxis(group, sliceAxis);
    m_Widget->ToggleGroupAxis(group, windowingAxis);
    for (const auto& slot : m_Widget->BuildGroupBarcodeSlots(group))
    {
      CPPUNIT_ASSERT_MESSAGE("Toggling every axis back off clears the displayed intent",
                             !slot.color.isValid());
    }
  }

  void EmptyGroupCache_AppliedOnFirstAssignmentThenCleared()
  {
    const auto group = m_Widget->CreateGroup();
    Pump();
    m_Widget->ToggleGroupAxis(group, AxisIndexOf(QmitkMxNSyncDimension::Slice));  // cache Slice only

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, group);

    // Exactly the cached axis lands - not SetCellMembership's nav-bundle default.
    CPPUNIT_ASSERT_MESSAGE("The cached Slice axis is applied to the assigned window",
                           IsLinked(0, QmitkMxNSyncDimension::Slice, group));
    for (const auto dimension : { QmitkMxNSyncDimension::Pan, QmitkMxNSyncDimension::Zoom,
                                  QmitkMxNSyncDimension::Crosshair })
    {
      CPPUNIT_ASSERT_MESSAGE("Only the cached axis is applied, not the nav-bundle default",
                             !IsLinked(0, dimension, group));
    }

    // The cache was cleared: emptying the group again shows an all-gap barcode
    // rather than the flushed intent re-appearing.
    m_Widget->SetCellMembership(CellId(0), group, false);
    for (const auto& slot : m_Widget->BuildGroupBarcodeSlots(group))
    {
      CPPUNIT_ASSERT_MESSAGE("A flushed cache must not re-appear when the group empties",
                             !slot.color.isValid());
    }
  }

  void EmptyGroup_AxisClickCaches_EvenWithSelection()
  {
    const auto group = m_Widget->CreateGroup();
    Pump();

    // Make widget1 the active window; the editor mirrors that into the map's
    // selection. Configuring an empty group must still only toggle the intent
    // cache - it must NOT assign the active/selected cell to the group.
    m_Editor->SetActiveRenderWindowWidget(m_Editor->GetRenderWindowWidget(CellId(1)));

    m_Widget->ToggleGroupAxis(group, AxisIndexOf(QmitkMxNSyncDimension::Slice));

    for (std::size_t cell = 0; cell < 3; ++cell)
    {
      CPPUNIT_ASSERT_MESSAGE("Configuring an empty group must not assign the selected cell",
                             !IsLinked(cell, QmitkMxNSyncDimension::Slice, group));
      CPPUNIT_ASSERT_MESSAGE("Configuring an empty group must not set a cell's selection group",
                             m_Editor->GetCellSelectionGroup(CellId(cell)) != group);
    }
    // The click cached the axis instead (visible through the group's barcode).
    const auto axisSlots = m_Widget->BuildGroupBarcodeSlots(group);
    CPPUNIT_ASSERT_MESSAGE("The clicked axis is cached, shown on the group barcode",
                           axisSlots[AxisIndexOf(QmitkMxNSyncDimension::Slice)].color.isValid());
  }

  void Selection_TogglesForUserGroup()
  {
    // The data-selection axis toggles on/off for a non-default group like any
    // other axis (this is the empty-group path's non-empty counterpart).
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan, "G");  // cell 0 is a member of G
    const int selectionAxis = static_cast<int>(QmitkMxNAllSyncDimensions.size());

    m_Widget->ToggleGroupAxis("G", selectionAxis);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Toggling the selection axis on links the member's selection to the group",
                                 std::string("G"), m_Editor->GetCellSelectionGroup(CellId(0)));

    m_Widget->ToggleGroupAxis("G", selectionAxis);
    CPPUNIT_ASSERT_MESSAGE("Toggling it off returns the member to the default selection group",
                           m_Editor->GetCellSelectionGroup(CellId(0)) != "G");
  }

  // --- Selection as the 8th axis, "main" default, (a)-replace join ------------

  void NewCell_DefaultLinksWindowingAndLutToMain()
  {
    // A fresh interactive cell auto-links the appearance axes to "main" so its
    // level/window and LUT stay inside the editor's group instead of the classic
    // node-global write. The other axes stay unlinked; selection is "main".
    for (std::size_t cell = 0; cell < 3; ++cell)
    {
      CPPUNIT_ASSERT_MESSAGE("A fresh cell links Windowing to 'main'",
                             IsLinked(cell, QmitkMxNSyncDimension::Windowing, "main"));
      CPPUNIT_ASSERT_MESSAGE("A fresh cell links LUT to 'main'",
                             IsLinked(cell, QmitkMxNSyncDimension::Lut, "main"));
      for (const auto dimension : { QmitkMxNSyncDimension::Pan, QmitkMxNSyncDimension::Zoom,
                                    QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Crosshair,
                                    QmitkMxNSyncDimension::Orientation })
      {
        CPPUNIT_ASSERT_MESSAGE("A fresh cell leaves the navigation/orientation axes unlinked",
                               !m_Editor->GetSyncLink(CellId(cell), dimension).has_value());
      }
      CPPUNIT_ASSERT_EQUAL_MESSAGE("A fresh cell's selection group is 'main'",
                                   std::string("main"), m_Editor->GetCellSelectionGroup(CellId(cell)));
    }
  }

  void NewCell_FrameIsMonoMain()
  {
    // The fresh cell reads a solid "main" frame: it links Windowing, LUT, and
    // selection to "main", so the eight-axis resolve is Mono("main").
    const auto identity = m_Editor->ResolveCellGroupIdentity(CellId(0));
    CPPUNIT_ASSERT_MESSAGE("A fresh cell's frame is Mono",
                           QmitkMxNMultiWidget::CellGroupIdentityKind::Mono == identity.kind);
    CPPUNIT_ASSERT_MESSAGE("A fresh cell's frame hue is 'main's color",
                           identity.hue == m_Editor->GetSyncGroupColor("main"));
  }

  void Frame_SelectionDivergesFromNav_IsComplex()
  {
    // Selection is the 8th frame axis: a cell whose navigation/intensity axes all
    // name one group but whose selection stays on "main" is an honest split ->
    // Complex (gray). With selection excluded it would misread as Mono.
    for (const auto dimension : QmitkMxNAllSyncDimensions)
    {
      m_Editor->ClearSyncLink(CellId(0), dimension);
    }
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan, "X");  // nav on X, selection still main

    const auto identity = m_Editor->ResolveCellGroupIdentity(CellId(0));
    CPPUNIT_ASSERT_MESSAGE("Nav on X but selection on 'main' spans two groups -> Complex",
                           QmitkMxNMultiWidget::CellGroupIdentityKind::Complex == identity.kind);
  }

  void Frame_WhollyOneGroup_IsMono()
  {
    // A cell wholly on one group across all eight axes reads Mono.
    for (const auto dimension : QmitkMxNAllSyncDimensions)
    {
      m_Editor->ClearSyncLink(CellId(0), dimension);
    }
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan, "X");
    m_Editor->SetCellSelectionGroup(CellId(0), "X");

    const auto identity = m_Editor->ResolveCellGroupIdentity(CellId(0));
    CPPUNIT_ASSERT_MESSAGE("All eight axes on X -> Mono(X)",
                           QmitkMxNMultiWidget::CellGroupIdentityKind::Mono == identity.kind);
    CPPUNIT_ASSERT_MESSAGE("Mono hue is X's color",
                           identity.hue == m_Editor->GetSyncGroupColor("X"));
  }

  void AssignReplace_ClearsOtherGroupLinks()
  {
    // Default join is mode (a) "replace": assigning a cell to a group clears its
    // links to every other group, so it ends synchronized only on the target.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan, "X");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "Y");
    // Give G a known synchronized dimension (Slice) via a second cell.
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "G");

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, "G");

    CPPUNIT_ASSERT_MESSAGE("Replace links the target group's dimension",
                           IsLinked(0, QmitkMxNSyncDimension::Slice, "G"));
    CPPUNIT_ASSERT_MESSAGE("Replace clears the cell's link to another group (Pan/X)",
                           !m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan).has_value());
    CPPUNIT_ASSERT_MESSAGE("Replace clears the cell's link to another group (Windowing/Y)",
                           !m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing).has_value());
  }

  void WidenedMembership_SelectionTieCountsAsMember()
  {
    // Membership counts the selection tie (the 8th axis): a cell tied to a group
    // only through data selection is a member, so a group axis toggle reaches it.
    // Driven through the public ApplyDimensionToGroup, whose members come from
    // the (now widened) GroupMembers.
    m_Editor->SetCellSelectionGroup(CellId(0), "S");  // sole tie to S is selection

    m_Widget->ApplyDimensionToGroup("S", QmitkMxNSyncDimension::Slice, true);

    CPPUNIT_ASSERT_MESSAGE("A selection-only member is reached by a group dimension toggle",
                           IsLinked(0, QmitkMxNSyncDimension::Slice, "S"));
  }

  void MainCard_Live_AxisClickHomogenizes()
  {
    // With every cell a member of "main" (via the appearance axes), the "main"
    // card is a live group: an axis-glyph click homogenizes that axis across all
    // members rather than toggling an empty-group intent cache.
    m_Widget->ToggleGroupAxis("main", AxisIndexOf(QmitkMxNSyncDimension::Slice));

    for (std::size_t cell = 0; cell < 3; ++cell)
    {
      CPPUNIT_ASSERT_MESSAGE("Clicking Slice on the live 'main' card links every cell",
                             IsLinked(cell, QmitkMxNSyncDimension::Slice, "main"));
    }
  }

  void AssignReplace_ReclaimsEmptiedSelectionGroup()
  {
    // A drop under (a) reverts the cell's selection to "main"; when that was the
    // last tie to a method-allocated group, the group is reclaimed and drops out
    // of the registry. Pins the intended reclaim (not a leak).
    m_Editor->SetCellSelectionGroup(CellId(0), "H");  // allocates selection group H
    const auto hasH = [this]()
    {
      const auto infos = m_Editor->GetSyncGroupInfos();
      return std::any_of(infos.begin(), infos.end(), [](const auto& info) { return info.id == "H"; });
    };
    CPPUNIT_ASSERT_MESSAGE("H exists once a cell selects it", hasH());

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, "G");  // (a): reverts selection to main

    CPPUNIT_ASSERT_MESSAGE("H is reclaimed when its last selection tie leaves", !hasH());
  }

  void AssignReplace_MultiCellReclaim_OnlyWhenLastLeaves()
  {
    // Two cells share a method-allocated selection group; a replace of the first
    // must not reclaim it while the second still holds it - only the last tie
    // leaving reclaims. Guards the per-cell reclaim ordering the single-cell test
    // cannot exercise.
    m_Editor->SetCellSelectionGroup(CellId(0), "H");
    m_Editor->SetCellSelectionGroup(CellId(1), "H");
    const auto hasH = [this]()
    {
      const auto infos = m_Editor->GetSyncGroupInfos();
      return std::any_of(infos.begin(), infos.end(), [](const auto& info) { return info.id == "H"; });
    };
    CPPUNIT_ASSERT_MESSAGE("H exists while two cells select it", hasH());

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, "G");
    CPPUNIT_ASSERT_MESSAGE("H survives while the other cell still selects it", hasH());

    m_Widget->AssignCellsToGroup(QStringList{ CellId(1) }, "G");
    CPPUNIT_ASSERT_MESSAGE("H is reclaimed once its last selection tie leaves", !hasH());
  }

  void MainCard_LinkNavigation_LinksAllCells()
  {
    // "main" is a live members-bearing group (every cell, via the appearance
    // axes), so its "Link navigation" action links the navigation bundle across
    // every cell - the editor-wide reach the live main card now has.
    m_Widget->LinkNavigationBundle("main");

    for (std::size_t cell = 0; cell < 3; ++cell)
    {
      for (const auto dimension : { QmitkMxNSyncDimension::Pan, QmitkMxNSyncDimension::Zoom,
                                    QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Crosshair })
      {
        CPPUNIT_ASSERT_MESSAGE("Link navigation on the live 'main' card links every cell",
                               IsLinked(cell, dimension, "main"));
      }
    }
  }

  void NewCell_BarcodePaintsAppearanceDefault()
  {
    // The fresh cell's utility-strip barcode reflects the "main" appearance
    // default with no intervening mutation: SetLayout emits LayoutChanged after
    // creating the cells with their default links, which refreshes the barcodes.
    const auto cell = m_Editor->GetRenderWindowWidget(CellId(0));
    CPPUNIT_ASSERT(nullptr != cell);
    auto* utility = cell->GetUtilityWidget();
    CPPUNIT_ASSERT(nullptr != utility);
    auto* barcode = utility->findChild<QmitkMxNSyncBarcodeWidget*>();
    CPPUNIT_ASSERT(nullptr != barcode);

    const auto axisSlots = barcode->Slots();
    int windowingSlot = -1;
    int lutSlot = -1;
    for (std::size_t i = 0; i < QmitkMxNAllSyncDimensions.size(); ++i)
    {
      if (QmitkMxNAllSyncDimensions[i] == QmitkMxNSyncDimension::Windowing) { windowingSlot = static_cast<int>(i); }
      if (QmitkMxNAllSyncDimensions[i] == QmitkMxNSyncDimension::Lut) { lutSlot = static_cast<int>(i); }
    }
    CPPUNIT_ASSERT(windowingSlot >= 0 && lutSlot >= 0);
    CPPUNIT_ASSERT_MESSAGE("A fresh cell's Windowing barcode slot is filled (main hue), no manual refresh",
                           axisSlots[windowingSlot].color.isValid());
    CPPUNIT_ASSERT_MESSAGE("A fresh cell's LUT barcode slot is filled (main hue), no manual refresh",
                           axisSlots[lutSlot].color.isValid());
  }

  void LoadedLayout_NoWindowingInjection()
  {
    // The appearance default lives in the interactive create path only. A
    // document that declares no Windowing/LUT link must round-trip verbatim -
    // ApplyLayout injects nothing, and serialize/apply is identity.
    for (std::size_t cell = 0; cell < 3; ++cell)
    {
      m_Editor->ClearSyncLink(CellId(cell), QmitkMxNSyncDimension::Windowing);
      m_Editor->ClearSyncLink(CellId(cell), QmitkMxNSyncDimension::Lut);
    }
    m_Editor->ApplyLayout(m_Editor->SerializeLayout());  // "load" the appearance-link-free document

    for (std::size_t cell = 0; cell < 3; ++cell)
    {
      CPPUNIT_ASSERT_MESSAGE("ApplyLayout injects no Windowing link a document did not declare",
                             !m_Editor->GetSyncLink(CellId(cell), QmitkMxNSyncDimension::Windowing).has_value());
      CPPUNIT_ASSERT_MESSAGE("ApplyLayout injects no LUT link a document did not declare",
                             !m_Editor->GetSyncLink(CellId(cell), QmitkMxNSyncDimension::Lut).has_value());
    }

    // Round-trip identity from a loaded state onward - a fixpoint, robust to the
    // SetLayout-vs-ApplyLayout normalization of the initial grid.
    const auto loaded = m_Editor->SerializeLayout();
    m_Editor->ApplyLayout(loaded);
    CPPUNIT_ASSERT_MESSAGE("A loaded layout round-trips to itself",
                           loaded == m_Editor->SerializeLayout());
  }

  // --- Stage 2: the (b)/(c) join-mode overrides -------------------------------

  void Assign_ReplaceMode_ClearsOthers()
  {
    // Explicit Replace matches the default: the cell's links to other groups are
    // cleared, so it ends synchronized only on the target.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan, "X");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "G");

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, "G", QmitkMxNGroupJoinMode::Replace);

    CPPUNIT_ASSERT_MESSAGE("Replace links the target group's dimension",
                           IsLinked(0, QmitkMxNSyncDimension::Slice, "G"));
    CPPUNIT_ASSERT_MESSAGE("Replace clears the cell's link to another group",
                           !m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan).has_value());
  }

  void Assign_FillEmptyMode_KeepsLinkedAxes()
  {
    // FillEmpty sets only the cell's currently-unlinked axes; an axis already
    // linked (to any group) is left untouched.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan, "X");
    // G synchronizes Pan and Slice (established via a second cell).
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Pan, "G");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "G");

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, "G", QmitkMxNGroupJoinMode::FillEmpty);

    CPPUNIT_ASSERT_MESSAGE("FillEmpty leaves an already-linked axis on its group",
                           IsLinked(0, QmitkMxNSyncDimension::Pan, "X"));
    CPPUNIT_ASSERT_MESSAGE("FillEmpty sets a previously-unlinked covered axis to the group",
                           IsLinked(0, QmitkMxNSyncDimension::Slice, "G"));
  }

  void Assign_FillEmptyMode_SelectionOnlyWhenResting()
  {
    // FillEmpty adopts a selection-syncing group's selection only for a cell
    // resting on the default group; a cell explicitly placed elsewhere keeps it.
    // The mode is load-bearing: under Replace, cell 2's move off "P" would reclaim
    // the (empty) "P" group and the "keeps P" assertion would flip.
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Pan, "G");
    m_Editor->SetCellSelectionGroup(CellId(1), "G");  // G synchronizes selection
    m_Editor->SetCellSelectionGroup(CellId(2), "P");  // cell 2 is explicitly on P

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, "G", QmitkMxNGroupJoinMode::FillEmpty);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A cell resting on the default adopts the group's selection",
                                 std::string("G"), m_Editor->GetCellSelectionGroup(CellId(0)));

    m_Widget->AssignCellsToGroup(QStringList{ CellId(2) }, "G", QmitkMxNGroupJoinMode::FillEmpty);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A cell explicitly on another selection group keeps it under FillEmpty",
                                 std::string("P"), m_Editor->GetCellSelectionGroup(CellId(2)));
  }

  void Assign_MergeMode_OverwritesCoveredKeepsRest()
  {
    // Merge overwrites the group's covered axes (collisions) but keeps the cell's
    // links on axes the group does not cover.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Pan, "X");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "Y");
    // G synchronizes Pan only.
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Pan, "G");

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, "G",
                                 QmitkMxNGroupJoinMode::MergeOverwriteCollisions);

    CPPUNIT_ASSERT_MESSAGE("Merge overwrites the covered axis (Pan X -> G)",
                           IsLinked(0, QmitkMxNSyncDimension::Pan, "G"));
    CPPUNIT_ASSERT_MESSAGE("Merge keeps a link on an axis the group does not cover (Windowing/Y)",
                           IsLinked(0, QmitkMxNSyncDimension::Windowing, "Y"));
  }

  void MultiSelect_SurvivesActiveMirror()
  {
    // Selecting several tiles makes the first the active render window, which
    // fires ActiveRenderWindowChanged back into SelectActiveWindowTile. That
    // mirror must not collapse the multi-selection to the single active cell.
    auto* cellMap = m_Widget->findChild<QmitkMxNCellMapWidget*>();
    CPPUNIT_ASSERT(nullptr != cellMap);
    Pump();

    cellMap->SetSelectedWindowIds(QStringList{ CellId(0), CellId(1) });
    Pump();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A multi-selection must survive the active-window mirror",
                                 2, static_cast<int>(cellMap->GetSelectedWindowIds().size()));
  }

  void JoinModeFromModifiers_MapsKeys()
  {
    // The drop-time modifier contract both drop targets share.
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::Replace
                   == QmitkMxNCellMapWidget::JoinModeFromModifiers(Qt::NoModifier));
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::MergeOverwriteCollisions
                   == QmitkMxNCellMapWidget::JoinModeFromModifiers(Qt::AltModifier));
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::FillEmpty
                   == QmitkMxNCellMapWidget::JoinModeFromModifiers(Qt::ShiftModifier));
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNLayoutEditorWidget)
