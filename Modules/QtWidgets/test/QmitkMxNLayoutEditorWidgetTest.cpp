/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkAutomatedLayoutWidget.h>
#include <QmitkMxNArrangeMode.h>
#include <QmitkMxNGroupJoinMode.h>
#include <QmitkMxNLayoutEditorWidget.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkMxNSyncBarcodeWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowUtilityWidget.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkBaseRenderer.h>
#include <mitkException.h>
#include <mitkImageGenerator.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkSliceNavigationHelper.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <nlohmann/json.hpp>

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QDropEvent>
#include <QElapsedTimer>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPointer>
#include <QPointF>
#include <QPushButton>
#include <QScrollBar>
#include <QSpinBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QTimer>
#include <QToolButton>

#include <algorithm>
#include <array>
#include <memory>
#include <variant>

/**
 * Drives the layout editor widget's mutation API against a real
 * QmitkMxNMultiWidget (no delegate, no mocks) and asserts the resulting
 * engine link state. The underlying engine behavior itself is covered by
 * QmitkMxNSyncGroupApiTest / QmitkMxNNavLinksTest.
 */
class QmitkMxNLayoutEditorWidgetTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNLayoutEditorWidgetTestSuite);

  MITK_TEST(CreateGroup_RegistersEngineGroup);
  MITK_TEST(Assign_EmptyGroup_LinksNavigationBundleByDefault);
  MITK_TEST(Assign_GroupWithDimensions_LinksThoseDimensions);
  MITK_TEST(AssignCells_JoinsEveryGivenCell);
  MITK_TEST(Remove_ClearsEveryDimension);
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
  MITK_TEST(Membership_SelectionTieCountsAsMember);
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
  MITK_TEST(Selection_TogglesForUserGroup);
  MITK_TEST(MultiSelect_SurvivesActiveMirror);

  MITK_TEST(NonTrivialConfig_FalseForFreshDefault);
  MITK_TEST(NonTrivialConfig_TrueOnceASecondGroupExists);
  MITK_TEST(NonTrivialSyncConfig_FalseForSingleCustomDefaultGroup);
  MITK_TEST(NonTrivialConfig_TrueForNavigationLinksOnDefaultGroup);
  MITK_TEST(NonTrivialConfig_TrueForDetachedWindowing);
  MITK_TEST(MultiTileDrop_AssignsEverySelectedCell);
  MITK_TEST(CardDrop_AskMenuIsParentless);
  MITK_TEST(CardDrop_AskMenuSurvivesCardRemoval);
  MITK_TEST(GridDialog_DataBasedLayoutClosesDialogAndAnchorsPopup);

  MITK_TEST(SyncHighlight_CellsSharingDimensionAxis);
  MITK_TEST(SyncHighlight_CellsSharingSelectionAxis);
  MITK_TEST(ArrangeSelection_ARebindStopsTheOldEditorsMirror);
  MITK_TEST(ArrangeRequests_AssignAndRemoveTheirWindows);
  MITK_TEST(SyncHighlight_CellAxisResolvesFromHoveredCell);

  MITK_TEST(DeleteGroup_RemovesMemberBearingGroup);
  MITK_TEST(DeleteGroup_RemovesEmptyCreatedGroup);
  MITK_TEST(DeleteGroup_RecreatedGroupStartsClean);
  MITK_TEST(DeleteGroup_MainIsNoOp);

  MITK_TEST(Matrix_UnlinkedCellCarriesNoChipOrOffset);
  MITK_TEST(Matrix_IsOnlyAsTallAsItsRows);
  MITK_TEST(ArrangeHint_WarnsWhileAWindowIsMaximized);
  MITK_TEST(Matrix_LinkedCellChipNamesGroupAndOffset);
  MITK_TEST(Matrix_AxisAssignAndClearRoundTrip);
  MITK_TEST(Matrix_SelectionAxisClearReturnsToDefault);
  MITK_TEST(Matrix_RegroupKeepsTheAuthoredOffset);
  MITK_TEST(Group_LinkActionsKeepAuthoredOffsets);
  MITK_TEST(Save_NavigationOnlyGroupRoundTripsToItself);
  MITK_TEST(Save_NavigationOnlyGroupStaysRegistered);
  MITK_TEST(DefaultGroup_SurvivesRoundTripWithoutSelectionMembers);
  MITK_TEST(Matrix_OffsetOnUnlinkedCellIsIgnored);
  MITK_TEST(Matrix_SliceRampSpreadsOverCellsInOrder);
  MITK_TEST(Matrix_SliceRampShowsAMovieFrame);
  MITK_TEST(Matrix_TracksAnEditMadeOnTheCards);
  MITK_TEST(Matrix_DetachLeavesNoChipsBehind);
  MITK_TEST(Matrix_CtrlClickOnRowHeaderAddsTheRow);
  MITK_TEST(Matrix_MixedOffsetsReadAsMultiple);
  MITK_TEST(Matrix_MixedOffsetMarkerIsNotWritable);
  MITK_TEST(Matrix_RampHiddenForCellsInDifferentGroups);
  MITK_TEST(Matrix_ColumnGrowsToFitALongGroupName);
  MITK_TEST(Matrix_EditNotifiesTheSyncFurniture);
  MITK_TEST(Matrix_BatchNotifiesOnceForTheWholeGesture);
  MITK_TEST(SyncHighlight_MatrixMarksTheSharedCells);
  MITK_TEST(SyncHighlight_MatrixMarkingClearsWithTheMap);
  MITK_TEST(Offset_CellBarcodeMarksAndWordsTheOffset);
  MITK_TEST(Offset_GroupBarcodeMarksWithoutNumbering);
  MITK_TEST(Offset_WordingIsTheSameOnEverySurface);
  MITK_TEST(ActionBar_NamesTheAxisWithItsGlyph);
  MITK_TEST(ActionBar_OffsetCommitAfterDetachIsHarmless);
  MITK_TEST(Arrange_BatchRemoveNotifiesOnce);
  MITK_TEST(Matrix_OffsetFocusPassWritesNothing);
  MITK_TEST(Matrix_OffsetFocusPassKeepsPrecision);
  MITK_TEST(Matrix_OffsetRealEditWrites);
  MITK_TEST(Matrix_PanOffsetEditKeepsMillimetreFractions);
  MITK_TEST(EmptyGroupCache_DroppedOnceTheGroupGainsAMember);
  MITK_TEST(Matrix_FirstBuildMirrorsThePlateSelection);
  MITK_TEST(Assign_ReplaceDropStartsTheOffsetFresh);
  MITK_TEST(Refresh_DeferredWhileApplyingLayout);

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
    return QmitkMxNSyncAxisToSlot(QmitkMxNSyncAxisOf(dimension));
  }

  /** Raise the "Advanced" face, which is what makes the matrix track the engine
   *  (see QmitkMxNLayoutEditorWidget::AdvancedFaceIsCurrent), and let its build
   *  settle. */
  void RaiseAdvancedFace() const
  {
    auto* faces = m_Widget->findChild<QTabWidget*>(QStringLiteral("QmitkMxNLayoutEditorFaces"));
    CPPUNIT_ASSERT_MESSAGE("The editor has its two faces on a tab widget", nullptr != faces);
    faces->setCurrentIndex(1);
    Pump();
  }

  /** Count SyncLinksChanged over a scope: the signal the per-cell barcodes, frame
   *  colors and cell overlays repaint on, so it is the contract an editor
   *  mutation owes to every surface outside this widget. */
  class SyncNotificationCounter
  {
  public:
    explicit SyncNotificationCounter(QmitkMxNMultiWidget* editor)
    {
      m_Connection = QObject::connect(editor, &QmitkMxNMultiWidget::SyncLinksChanged,
                                      [this]() { ++m_Count; });
    }
    ~SyncNotificationCounter() { QObject::disconnect(m_Connection); }
    SyncNotificationCounter(const SyncNotificationCounter&) = delete;
    SyncNotificationCounter& operator=(const SyncNotificationCounter&) = delete;
    int Count() const { return m_Count; }

  private:
    QMetaObject::Connection m_Connection;
    int m_Count = 0;
  };

  /** The advanced matrix, by its stable object name. */
  QTableWidget* Matrix() const
  {
    auto* matrix =
      m_Widget->findChild<QTableWidget*>(QStringLiteral("QmitkMxNLayoutEditorMatrix"));
    CPPUNIT_ASSERT_MESSAGE("The advanced face holds the matrix", nullptr != matrix);
    return matrix;
  }

  /** Select one matrix cell per (row, axis) pair, as a drag across the grid would. */
  void SelectMatrixCells(const std::vector<std::pair<int, int>>& cells) const
  {
    auto* matrix = Matrix();
    QItemSelection selection;
    for (const auto& [row, axis] : cells)
    {
      const auto index = matrix->model()->index(row, axis);
      selection.select(index, index);
    }
    matrix->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
    Pump();
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

  void Assign_EmptyGroup_LinksNavigationBundleByDefault()
  {
    const auto id = m_Widget->CreateGroup();

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, id);

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

  void Assign_GroupWithDimensions_LinksThoseDimensions()
  {
    // Group already synchronizes windowing only (established via cell 0).
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "wl");

    m_Widget->AssignCellsToGroup(QStringList{ CellId(1) }, "wl");

    CPPUNIT_ASSERT(IsLinked(1, QmitkMxNSyncDimension::Windowing, "wl"));
    CPPUNIT_ASSERT_MESSAGE("Joining follows the group's existing dimension set, not the bundle",
                           !IsLinked(1, QmitkMxNSyncDimension::Slice, "wl"));
  }

  void AssignCells_JoinsEveryGivenCell()
  {
    // The drag-and-drop / assign-selection path.
    const auto id = m_Widget->CreateGroup();

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(2) }, id);

    CPPUNIT_ASSERT(IsLinked(0, QmitkMxNSyncDimension::Slice, id));
    CPPUNIT_ASSERT(IsLinked(2, QmitkMxNSyncDimension::Slice, id));
    CPPUNIT_ASSERT_MESSAGE("The unassigned cell stays unlinked",
                           !m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice).has_value());
  }

  void Remove_ClearsEveryDimension()
  {
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing, "nav");
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom, "other");

    m_Widget->RemoveCellsFromGroup(QStringList{ CellId(0) }, "nav");

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
    // An edit made in the layout editor must refresh the per-cell utility-strip
    // barcode, not just the editor's own view; the editor's mutators route
    // through RefreshSyncControls (SyncLinksChanged) for that. A fresh cell
    // already links Windowing to "main", so only the slot's colour tells the
    // refreshed barcode from the stale one.
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

    const QColor navHue = m_Editor->GetSyncGroupColor("nav");
    CPPUNIT_ASSERT_MESSAGE("The two groups must be told apart by hue",
                           navHue != m_Editor->GetSyncGroupColor("main"));
    const auto axisSlots = barcode->Slots();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "An editor dimension toggle must refresh the cell's barcode without a manual refresh",
      navHue.name().toStdString(), axisSlots[windowingSlot].color.name().toStdString());
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
    CPPUNIT_ASSERT_MESSAGE("The rebuild shows the default group's card", nullptr != CardFor("main"));
    CPPUNIT_ASSERT_MESSAGE("The rebuild shows the new group's card", nullptr != CardFor("nav"));
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
    m_Widget->RemoveCellsFromGroup(QStringList{ CellId(0) }, "alpha");
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

    // An axis click on this empty group toggles the intent cache, not the
    // engine.
    const int sliceAxis = AxisIndexOf(QmitkMxNSyncDimension::Slice);
    const int windowingAxis = AxisIndexOf(QmitkMxNSyncDimension::Windowing);
    m_Widget->ToggleGroupAxis(group, QmitkMxNSyncAxis::Slice);
    m_Widget->ToggleGroupAxis(group, QmitkMxNSyncAxis::Windowing);

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

    // The engine is untouched: no cell links or selects the group.
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
    m_Widget->ToggleGroupAxis(group, QmitkMxNSyncAxis::Slice);
    m_Widget->ToggleGroupAxis(group, QmitkMxNSyncAxis::Windowing);
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
    m_Widget->ToggleGroupAxis(group, QmitkMxNSyncAxis::Slice);  // cache Slice only

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, group);

    // Exactly the cached axis lands - not the navigation-bundle default.
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
    m_Widget->RemoveCellsFromGroup(QStringList{ CellId(0) }, group);
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

    // Make widget1 the active window; the editor mirrors that into the window
    // selection. Configuring an empty group must still only toggle the intent
    // cache - it must NOT assign the active/selected cell to the group.
    m_Editor->SetActiveRenderWindowWidget(m_Editor->GetRenderWindowWidget(CellId(1)));

    m_Widget->ToggleGroupAxis(group, QmitkMxNSyncAxis::Slice);

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

    m_Widget->ToggleGroupAxis("G", QmitkMxNSyncAxis::Selection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Toggling the selection axis on links the member's selection to the group",
                                 std::string("G"), m_Editor->GetCellSelectionGroup(CellId(0)));

    m_Widget->ToggleGroupAxis("G", QmitkMxNSyncAxis::Selection);
    CPPUNIT_ASSERT_MESSAGE("Toggling it off returns the member to the default selection group",
                           m_Editor->GetCellSelectionGroup(CellId(0)) != "G");
  }

  // --- Selection as the 8th axis, "main" default, Replace join -----------------

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
    // The default join mode is Replace: assigning a cell to a group clears its
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

  void Membership_SelectionTieCountsAsMember()
  {
    // Membership counts the selection tie (the 8th axis): a cell tied to a group
    // only through data selection is a member, so a group axis toggle reaches it.
    // Driven through the public ApplyDimensionToGroup, whose members come from
    // GroupMembers.
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
    m_Widget->ToggleGroupAxis("main", QmitkMxNSyncAxis::Slice);

    for (std::size_t cell = 0; cell < 3; ++cell)
    {
      CPPUNIT_ASSERT_MESSAGE("Clicking Slice on the live 'main' card links every cell",
                             IsLinked(cell, QmitkMxNSyncDimension::Slice, "main"));
    }
  }

  void AssignReplace_ReclaimsEmptiedSelectionGroup()
  {
    // A Replace drop reverts the cell's selection to "main"; when that was the
    // last tie to a method-allocated group, the group is reclaimed and drops out
    // of the registry. Pins the intended reclaim (not a leak).
    m_Editor->SetCellSelectionGroup(CellId(0), "H");  // allocates selection group H
    const auto hasH = [this]()
    {
      const auto infos = m_Editor->GetSyncGroupInfos();
      return std::any_of(infos.begin(), infos.end(), [](const auto& info) { return info.id == "H"; });
    };
    CPPUNIT_ASSERT_MESSAGE("H exists once a cell selects it", hasH());

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, "G");  // Replace: reverts selection to main

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
    // every cell, so the main card's actions reach the whole editor.
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

  // --- The FillEmpty and MergeOverwriteCollisions join modes -------------------

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
    // Selecting several windows makes the first the active render window, which
    // fires ActiveRenderWindowChanged back into SelectActiveWindow. That mirror
    // must not collapse the multi-selection to the single active cell.
    auto* arrangeMode = m_Editor->GetArrangeMode();
    Pump();

    arrangeMode->SetSelectedWindowIds(QStringList{ CellId(0), CellId(1) });
    Pump();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A multi-selection must survive the active-window mirror",
                                 2, static_cast<int>(arrangeMode->GetSelectedWindowIds().size()));
    CPPUNIT_ASSERT_MESSAGE("The first selected window became the active one",
                           m_Editor->GetActiveRenderWindowWidget() == m_Editor->GetRenderWindowWidget(CellId(0)));
  }

  // --- Destructive-layout-change guard predicate ------------------------------

  void NonTrivialConfig_FalseForFreshDefault()
  {
    // setUp leaves the fresh default: the sole group is "main" and every cell
    // resolves to Mono("main"). That is exactly the "everything bound to main"
    // state the layout-replace warning treats as trivial (no prompt).
    CPPUNIT_ASSERT_MESSAGE("The fresh default configuration is trivial",
                           !m_Widget->HasNonTrivialSyncConfig());
  }

  void NonTrivialConfig_TrueOnceASecondGroupExists()
  {
    // Assigning a cell to a new group adds a group beyond "main" and links
    // synchronization the user built - a layout replace would discard it, so the
    // predicate must report the configuration as non-trivial.
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, "alpha");

    CPPUNIT_ASSERT_MESSAGE("A user-created group makes the configuration non-trivial",
                           m_Widget->HasNonTrivialSyncConfig());
  }

  void NonTrivialSyncConfig_FalseForSingleCustomDefaultGroup()
  {
    // A document whose sole group is not literally named "main" (a custom
    // default label, here "left") is still the fresh/clean configuration when
    // its windows link exactly what a fresh window links: windowing, LUT and
    // selection on that group, nothing else. The predicate must not warn just
    // because that group's name differs from the literal "main".
    const auto doc = nlohmann::json::parse(R"json({
      "version": "3.0",
      "root": {
        "type": "split", "orientation": "horizontal",
        "children": [
          { "type": "window", "id": "mxn__w0", "view_direction": "axial",
            "links": { "windowing": "left", "lut": "left", "selection": "left" } }
        ]
      }
    })json");
    CPPUNIT_ASSERT_NO_THROW(m_Editor->ApplyLayout(doc));

    // Pin the load itself, so a failure below points at the predicate rather
    // than at the document not loading as intended.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The window rests on the document's sole group",
                                 std::string("left"), m_Editor->GetCellSelectionGroup(QStringLiteral("mxn__w0")));
    const auto infos = m_Editor->GetSyncGroupInfos();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The document registers exactly one group", std::size_t{1}, infos.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("That group is 'left', not 'main'", std::string("left"), infos.front().id);
    CPPUNIT_ASSERT_EQUAL(std::string("left"), m_Editor->GetDefaultSyncGroupName());

    CPPUNIT_ASSERT_MESSAGE("A single custom-named default group is trivial, like 'main' would be",
                           !m_Widget->HasNonTrivialSyncConfig());
  }

  void NonTrivialConfig_TrueForNavigationLinksOnDefaultGroup()
  {
    // Navigation links on the default group (the main card's "Link navigation",
    // an axis click, authored offsets) are user-built synchronization even
    // though no second group exists; a layout replace would discard them.
    const auto defaultGroup = m_Editor->GetDefaultSyncGroupName();
    for (const auto dimension : { QmitkMxNSyncDimension::Pan, QmitkMxNSyncDimension::Zoom,
                                  QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Crosshair,
                                  QmitkMxNSyncDimension::Orientation })
    {
      m_Editor->SetSyncLink(CellId(0), dimension, defaultGroup);
      CPPUNIT_ASSERT_MESSAGE(std::string("A '") + QmitkMxNSyncDimensionToLinkKey(dimension)
                               + "' link on the default group makes the configuration non-trivial",
                             m_Widget->HasNonTrivialSyncConfig());
      m_Editor->ClearSyncLink(CellId(0), dimension);
      CPPUNIT_ASSERT_MESSAGE("Clearing the link restores the trivial default",
                             !m_Widget->HasNonTrivialSyncConfig());
    }
  }

  void NonTrivialConfig_TrueForDetachedWindowing()
  {
    // A window detached from the default group's windowing or LUT no longer
    // matches a fresh window, so replacing the layout would discard that choice.
    m_Editor->ClearSyncLink(CellId(1), QmitkMxNSyncDimension::Windowing);
    CPPUNIT_ASSERT_MESSAGE("A window detached from windowing makes the configuration non-trivial",
                           m_Widget->HasNonTrivialSyncConfig());

    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Windowing, m_Editor->GetDefaultSyncGroupName());
    m_Editor->ClearSyncLink(CellId(2), QmitkMxNSyncDimension::Lut);
    CPPUNIT_ASSERT_MESSAGE("A window detached from the LUT makes the configuration non-trivial",
                           m_Widget->HasNonTrivialSyncConfig());
  }

  // --- Multi-tile drag-and-drop -----------------------------------------------

  void MultiTileDrop_AssignsEverySelectedCell()
  {
    // A plate drag carries the whole selection; a drop on a group card must
    // assign every one. Real Qt drag loops cannot run headlessly, so build the
    // drag's payload through the arrange mode and deliver it as a synthetic
    // drop onto the card, driving the real encode/decode seam and the card's
    // AssignCellsToGroup handler.
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice, "grp");  // grp syncs Slice
    Pump();
    auto* card = CardFor("grp");
    CPPUNIT_ASSERT(nullptr != card);

    m_Editor->GetArrangeMode()->SetSelectedWindowIds(QStringList{ CellId(0), CellId(1) });
    Pump();

    const std::unique_ptr<QMimeData> dragged(m_Editor->GetArrangeMode()->CreateDragMimeData());
    QMimeData mime;
    mime.setData(QmitkMxNCellsMimeType, dragged->data(QmitkMxNCellsMimeType));

    // Dispatch straight to the card's virtual event() (via the public QObject
    // overload; QWidget narrows the override to protected). QApplication::notify
    // routes real drag-and-drop events through the platform drag manager, so a
    // plain sendEvent would never reach dropEvent here.
    QDropEvent drop(QPointF(5, 5), Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier);
    static_cast<QObject*>(card)->event(&drop);
    Pump();

    CPPUNIT_ASSERT_MESSAGE("A multi-tile drop assigns the first selected cell",
                           IsLinked(0, QmitkMxNSyncDimension::Slice, "grp"));
    CPPUNIT_ASSERT_MESSAGE("A multi-tile drop assigns the second selected cell",
                           IsLinked(1, QmitkMxNSyncDimension::Slice, "grp"));
    CPPUNIT_ASSERT_MESSAGE("The pre-existing member keeps its link",
                           IsLinked(2, QmitkMxNSyncDimension::Slice, "grp"));
  }

  // --- Nested event loops on a group card ------------------------------------

  void CardDrop_AskMenuIsParentless()
  {
    // A right-button drop asks for its join mode through a menu whose event
    // loop can deliver a layout apply that deletes the card. Owned by the card,
    // the stack-local menu would be deleted from under its own frame.
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice, "grp");
    Pump();
    auto* card = CardFor("grp");
    CPPUNIT_ASSERT(nullptr != card);

    const std::unique_ptr<QMimeData> mime(QmitkMxNCreateCellsMimeData(QStringList{ CellId(0) }, true));

    bool opened = false;
    bool parentless = false;
    // Scopes the timer: if no menu loop runs it, it dies with this test.
    QObject timerContext;
    QTimer::singleShot(0, &timerContext, [&]()
    {
      auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
      opened = nullptr != menu;
      if (!opened)
      {
        return;
      }
      parentless = nullptr == menu->parentWidget();
      menu->close();
    });

    // Dispatched to the card's event() for the reason given in
    // MultiTileDrop_AssignsEverySelectedCell.
    QDropEvent drop(QPointF(5, 5), Qt::CopyAction, mime.get(), Qt::RightButton, Qt::NoModifier);
    static_cast<QObject*>(card)->event(&drop);

    CPPUNIT_ASSERT_MESSAGE("An ask-mode drop on a card opens the join-mode menu", opened);
    CPPUNIT_ASSERT_MESSAGE("The join-mode menu has no parent the card's teardown could delete", parentless);
    CPPUNIT_ASSERT_MESSAGE("A dismissed menu assigns nothing", !IsLinked(0, QmitkMxNSyncDimension::Slice, "grp"));
  }

  void CardDrop_AskMenuSurvivesCardRemoval()
  {
    // A layout applied while the join-mode menu is open (a REST request does
    // that) removes the card's group and so the card. The drop must not touch
    // the deleted card afterwards, even when the user picks a mode.
    const auto baseline = m_Editor->SerializeLayout();
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice, "grp");
    Pump();
    const QPointer<QWidget> card = CardFor("grp");
    CPPUNIT_ASSERT(!card.isNull());

    const std::unique_ptr<QMimeData> mime(QmitkMxNCreateCellsMimeData(QStringList{ CellId(0) }, true));

    bool opened = false;
    bool cardGone = false;
    QObject timerContext;
    QTimer::singleShot(0, &timerContext, [&]()
    {
      const QPointer<QMenu> menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
      opened = !menu.isNull();
      if (!opened)
      {
        return;
      }
      m_Editor->ApplyLayout(baseline);
      Pump();  // runs the deferred card reconcile inside the menu's loop
      cardGone = card.isNull();
      if (menu.isNull())
      {
        return;
      }
      // Pick "Replace" the way a user does, so the menu returns a mode.
      menu->setActiveAction(menu->actions().front());
      QKeyEvent press(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
      QCoreApplication::sendEvent(menu, &press);
    });

    QDropEvent drop(QPointF(5, 5), Qt::CopyAction, mime.get(), Qt::RightButton, Qt::NoModifier);
    static_cast<QObject*>(card.data())->event(&drop);

    CPPUNIT_ASSERT_MESSAGE("An ask-mode drop on a card opens the join-mode menu", opened);
    CPPUNIT_ASSERT_MESSAGE("The applied layout removed the card while its menu was open", cardGone);
    CPPUNIT_ASSERT_MESSAGE("A drop whose card is gone recreates no group", !this->RegistryHasGroup("grp"));
    CPPUNIT_ASSERT_MESSAGE("A drop whose card is gone assigns nothing",
                           !IsLinked(0, QmitkMxNSyncDimension::Slice, "grp"));
  }

  // --- Grid dialog -------------------------------------------------------------

  void GridDialog_DataBasedLayoutClosesDialogAndAnchorsPopup()
  {
    QToolButton* editGrid = nullptr;
    for (auto* button : m_Widget->findChildren<QToolButton*>())
    {
      if (button->text() == QStringLiteral("Edit grid..."))
      {
        editGrid = button;
      }
    }
    CPPUNIT_ASSERT(nullptr != editGrid);

    bool dialogFound = false;
    bool dialogClosed = false;
    bool popupShown = false;
    QPoint expectedTopRight;
    QPoint popupTopRight;
    QObject timerContext;
    QTimer::singleShot(0, &timerContext, [&]()
    {
      auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
      dialogFound = nullptr != dialog;
      if (!dialogFound)
      {
        return;
      }
      // Away from the screen origin, so a position taken relative to the dialog
      // cannot land on the global anchor by accident.
      dialog->move(300, 200);
      auto* dataBased = dialog->findChild<QPushButton*>(QStringLiteral("dataBasedLayoutButton"));
      if (nullptr != dataBased)
      {
        auto* picker = dataBased->parentWidget();
        expectedTopRight = picker->mapToGlobal(QPoint(picker->width(), 0));
        dataBased->click();
        dialogClosed = !dialog->isVisible();
        if (auto* popup = dialog->findChild<QmitkAutomatedLayoutWidget*>())
        {
          popupShown = popup->isVisible();
          popupTopRight = popup->pos() + QPoint(popup->width(), 0);
          popup->close();
        }
      }
      if (dialog->isVisible())
      {
        dialog->reject();
      }
    });
    editGrid->click();

    CPPUNIT_ASSERT_MESSAGE("Edit grid opens the modal grid dialog", dialogFound);
    CPPUNIT_ASSERT_MESSAGE("Data-based layout opens the image chooser", popupShown);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The chooser's top-right corner sits on the picker's (x)",
                                 expectedTopRight.x(), popupTopRight.x());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The chooser's top-right corner sits on the picker's (y)",
                                 expectedTopRight.y(), popupTopRight.y());
    CPPUNIT_ASSERT_MESSAGE("Data-based layout closes the grid dialog, so a dismissed chooser leaves nothing behind",
                           dialogClosed);
  }

  // --- Group removal -----------------------------------------------------------

  bool RegistryHasGroup(const std::string& id) const
  {
    const auto infos = m_Editor->GetSyncGroupInfos();
    return std::any_of(infos.begin(), infos.end(),
                       [&id](const auto& info) { return info.id == id; });
  }

  void DeleteGroup_RemovesMemberBearingGroup()
  {
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, "grp");  // nav-links them
    CPPUNIT_ASSERT(IsLinked(0, QmitkMxNSyncDimension::Slice, "grp"));
    CPPUNIT_ASSERT(this->RegistryHasGroup("grp"));

    m_Widget->DeleteGroup("grp");

    for (std::size_t cell = 0; cell < 2; ++cell)
    {
      for (const auto dimension : QmitkMxNAllSyncDimensions)
      {
        CPPUNIT_ASSERT_MESSAGE("Deleting a group clears every member link to it",
                               !IsLinked(cell, dimension, "grp"));
      }
    }
    CPPUNIT_ASSERT_MESSAGE("A deleted group leaves the registry", !this->RegistryHasGroup("grp"));
  }

  void DeleteGroup_RemovesEmptyCreatedGroup()
  {
    // A "+ Group" group is registered with no members; the per-member reclaim
    // cannot drop it, so removal must erase its registry entry explicitly.
    const auto id = m_Widget->CreateGroup();
    CPPUNIT_ASSERT(!id.empty());
    CPPUNIT_ASSERT_MESSAGE("A freshly created empty group is registered", this->RegistryHasGroup(id));

    m_Widget->DeleteGroup(id);

    CPPUNIT_ASSERT_MESSAGE("Deleting an empty '+ Group' group removes its registry entry",
                           !this->RegistryHasGroup(id));
  }

  void DeleteGroup_RecreatedGroupStartsClean()
  {
    // A group's engine index is recycled by the next CreateGroup() once freed;
    // its cosmetics (display name, color) must not survive the recycling, or a
    // brand-new group reappears already renamed and recolored by a stranger's
    // edits.
    auto groupIds = [this]()
    {
      std::vector<std::string> ids;
      for (const auto& info : m_Editor->GetSyncGroupInfos())
      {
        ids.push_back(info.id);
      }
      return ids;
    };
    // The one id in 'after' that was not in 'before' - the group a single
    // CreateGroup() call just added, read from the registry so the id checks
    // below do not depend on CreateGroup()'s return value, which the final
    // assertion checks separately.
    auto newGroupId = [](const std::vector<std::string>& before, const std::vector<std::string>& after)
    {
      for (const auto& id : after)
      {
        if (std::find(before.begin(), before.end(), id) == before.end())
        {
          return id;
        }
      }
      return std::string();
    };

    const auto beforeA = groupIds();
    m_Widget->CreateGroup();
    const auto idA = newGroupId(beforeA, groupIds());
    CPPUNIT_ASSERT_MESSAGE("Group A registers under a fresh id", !idA.empty());

    const auto beforeB = groupIds();
    m_Widget->CreateGroup();
    const auto idB = newGroupId(beforeB, groupIds());
    CPPUNIT_ASSERT_MESSAGE("Group B registers under a fresh id", !idB.empty());

    const auto colorB = m_Editor->GetSyncGroupColor(idB);

    m_Editor->SetSyncGroupDisplayName(idA, "Tumor");
    m_Editor->SetSyncGroupColor(idA, QColor(Qt::red));

    m_Widget->DeleteGroup(idA);

    const auto beforeA2 = groupIds();
    const auto returnedA2 = m_Widget->CreateGroup();
    const auto idA2 = newGroupId(beforeA2, groupIds());
    CPPUNIT_ASSERT_MESSAGE("The recreated group registers under a fresh id", !idA2.empty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The recreated group reuses A's reclaimed engine id", idA, idA2);

    const auto infos = m_Editor->GetSyncGroupInfos();
    const auto found = std::find_if(infos.begin(), infos.end(),
                                    [&idA2](const auto& info) { return info.id == idA2; });
    CPPUNIT_ASSERT_MESSAGE("The recreated group appears in the registry", infos.end() != found);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A recreated group must not inherit the deleted group's display name",
                                 idA2, found->displayName);
    CPPUNIT_ASSERT_MESSAGE("A recreated group must not inherit the deleted group's color",
                           !found->hasExplicitColor);

    CPPUNIT_ASSERT_MESSAGE("An unrelated group's color is unaffected by another group's delete/recreate",
                           colorB == m_Editor->GetSyncGroupColor(idB));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("CreateGroup must return the new group's id, not its display name",
                                 idA2, returnedA2);
  }

  void DeleteGroup_MainIsNoOp()
  {
    m_Widget->DeleteGroup("main");

    CPPUNIT_ASSERT_MESSAGE("The default 'main' group is never removed", this->RegistryHasGroup("main"));
    CPPUNIT_ASSERT_MESSAGE("Cells keep their default 'main' windowing link",
                           IsLinked(0, QmitkMxNSyncDimension::Windowing, "main"));
  }

  // --- Sync-highlight-on-hover -------------------------------------------------

  void SyncHighlight_CellsSharingDimensionAxis()
  {
    // Two cells joined to a group share the navigation bundle (incl. Slice); the
    // resolver behind the hover highlight lists exactly them for that axis.
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(2) }, id);

    QStringList members =
      m_Editor->CellsSharingAxis(QString::fromStdString(id), QmitkMxNSyncAxis::Slice);
    members.sort();
    QStringList expected{ CellId(0), CellId(2) };
    expected.sort();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The Slice axis lists exactly the group's linked cells",
                                 expected.join(QStringLiteral(",")).toStdString(),
                                 members.join(QStringLiteral(",")).toStdString());

    CPPUNIT_ASSERT_MESSAGE("An axis no member links resolves to nothing",
                           m_Editor->CellsSharingAxis(QString::fromStdString(id),
                             QmitkMxNSyncAxis::Orientation).isEmpty());
    CPPUNIT_ASSERT_MESSAGE("An unknown group resolves to nothing",
                           m_Editor->CellsSharingAxis(QStringLiteral("no-such-group"),
                             QmitkMxNSyncAxis::Slice).isEmpty());
  }

  void ArrangeRequests_AssignAndRemoveTheirWindows()
  {
    // The plates only ask; the layout editor performs the change.
    auto* arrangeMode = m_Editor->GetArrangeMode();
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice, "grp");
    arrangeMode->RequestAssign(QStringLiteral("grp"), CellId(0), QmitkMxNGroupJoinMode::Replace);
    CPPUNIT_ASSERT_MESSAGE("An assign request joins the window to the group",
                           IsLinked(0, QmitkMxNSyncDimension::Slice, "grp"));

    arrangeMode->RequestRemove(QStringLiteral("grp"), CellId(0));
    CPPUNIT_ASSERT_MESSAGE("A remove request takes it out again",
                           !IsLinked(0, QmitkMxNSyncDimension::Slice, "grp"));
    CPPUNIT_ASSERT_MESSAGE("...and leaves the other member alone",
                           IsLinked(2, QmitkMxNSyncDimension::Slice, "grp"));
  }

  void ArrangeSelection_ARebindStopsTheOldEditorsMirror()
  {
    // A second editor with the same name has the same cell ids, so a leaking
    // connection would visibly move its active window.
    QmitkMxNMultiWidget other;
    other.SetDataStorage(m_DataStorage);
    other.InitializeMultiWidget();
    other.SetLayout(1, 3);
    other.SetActiveRenderWindowWidget(other.GetRenderWindowWidget(CellId(1)));
    m_Widget->SetMultiWidget(&other);

    m_Editor->GetArrangeMode()->SetSelectedWindowIds(QStringList{ CellId(0) });
    CPPUNIT_ASSERT_MESSAGE("The previously bound editor's selection no longer drives the widget",
                           other.GetActiveRenderWindowWidget() == other.GetRenderWindowWidget(CellId(1)));

    other.GetArrangeMode()->SetSelectedWindowIds(QStringList{ CellId(2) });
    CPPUNIT_ASSERT_MESSAGE("The bound editor's selection does",
                           other.GetActiveRenderWindowWidget() == other.GetRenderWindowWidget(CellId(2)));
    m_Widget->SetMultiWidget(m_Editor.get());
  }

  void SyncHighlight_CellsSharingSelectionAxis()
  {
    // The data-selection axis (the last barcode slot) resolves over the selection
    // connector, not the per-dimension links.
    m_Editor->SetCellSelectionGroup(CellId(0), "sel");
    m_Editor->SetCellSelectionGroup(CellId(1), "sel");

    QStringList members = m_Editor->CellsSharingAxis(QStringLiteral("sel"), QmitkMxNSyncAxis::Selection);
    members.sort();
    QStringList expected{ CellId(0), CellId(1) };
    expected.sort();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The selection axis lists cells sharing that selection group",
                                 expected.join(QStringLiteral(",")).toStdString(),
                                 members.join(QStringLiteral(",")).toStdString());
  }

  void SyncHighlight_CellAxisResolvesFromHoveredCell()
  {
    // A window's glyph hover: resolve the hovered cell's group for the axis,
    // then highlight every cell sharing it.
    auto* arrangeMode = m_Editor->GetArrangeMode();

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "g5");
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice, "g5");

    m_Widget->HighlightCellAxis(CellId(0), QmitkMxNSyncAxis::Slice);
    QStringList highlight = arrangeMode->GetHighlightedWindowIds();
    highlight.sort();
    QStringList expected{ CellId(0), CellId(2) };
    expected.sort();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hovering a cell's linked axis highlights every cell sharing it",
                                 expected.join(QStringLiteral(",")).toStdString(),
                                 highlight.join(QStringLiteral(",")).toStdString());

    m_Widget->HighlightCellAxis(CellId(0), QmitkMxNSyncAxis::Orientation);
    CPPUNIT_ASSERT_MESSAGE("Hovering an axis the cell does not link clears the highlight",
                           arrangeMode->GetHighlightedWindowIds().isEmpty());
  }

  // --- The advanced matrix -----------------------------------------------------

  void ArrangeHint_WarnsWhileAWindowIsMaximized()
  {
    auto* hint = m_Widget->findChild<QLabel*>(QStringLiteral("QmitkMxNLayoutEditorArrangeHint"));
    CPPUNIT_ASSERT(nullptr != hint);
    CPPUNIT_ASSERT_MESSAGE("The everyday hint is plain", hint->styleSheet().isEmpty());
    const QString everyday = hint->text();

    m_Editor->SetMaximizedCell(CellId(1));
    CPPUNIT_ASSERT_MESSAGE("A maximized window changes the hint", hint->text() != everyday);
    CPPUNIT_ASSERT_MESSAGE("...into a warning", hint->styleSheet().contains(QStringLiteral("bold")));

    m_Editor->SetMaximizedCell(QString());
    CPPUNIT_ASSERT(hint->styleSheet().isEmpty());
    CPPUNIT_ASSERT(everyday == hint->text());
  }

  void Matrix_IsOnlyAsTallAsItsRows()
  {
    // A few windows must not stretch the matrix over the whole page: the action
    // bar belongs right under the rows it edits.
    m_Widget->resize(600, 1200);
    m_Widget->show();
    this->RaiseAdvancedFace();
    Pump();

    auto* matrix = m_Widget->findChild<QTableWidget*>(QStringLiteral("QmitkMxNLayoutEditorMatrix"));
    CPPUNIT_ASSERT(nullptr != matrix);
    int rowsHeight = matrix->horizontalHeader()->height() + 2 * matrix->frameWidth();
    for (int row = 0; row < matrix->rowCount(); ++row)
    {
      rowsHeight += matrix->rowHeight(row);
    }
    CPPUNIT_ASSERT_EQUAL(3, matrix->rowCount());
    CPPUNIT_ASSERT_MESSAGE("Every row is shown without a scroll bar",
                           !matrix->verticalScrollBar()->isVisible());
    CPPUNIT_ASSERT_MESSAGE("...and the matrix is no taller than its rows need",
                           matrix->height() <= rowsHeight + matrix->horizontalScrollBar()->sizeHint().height());
  }

  void Matrix_UnlinkedCellCarriesNoChipOrOffset()
  {
    // A cell that is not linked on a dimension has no group and no offset to
    // show. Zoom in particular: the neutral factor is 1, which must not read as
    // an authored "x1" relationship on a cell that is not in a zoom group.
    this->RaiseAdvancedFace();

    CPPUNIT_ASSERT_MESSAGE("The fixture's cells start unlinked on zoom",
                           !m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Zoom).has_value());

    const auto cell =
      m_Widget->AdvancedMatrixCell(CellId(0), QmitkMxNSyncAxis::Zoom);
    CPPUNIT_ASSERT_MESSAGE("An unlinked cell names no group", cell.group.empty());
    CPPUNIT_ASSERT_MESSAGE("An unlinked cell shows no offset", cell.offset.isEmpty());
  }

  void Matrix_LinkedCellChipNamesGroupAndOffset()
  {
    this->RaiseAdvancedFace();

    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, id);
    m_Widget->SetCellDimensionOffset(CellId(0), QmitkMxNSyncDimension::Slice, 2);
    Pump();

    const auto slice =
      m_Widget->AdvancedMatrixCell(CellId(0), QmitkMxNSyncAxis::Slice);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The chip names the linked group", id, slice.group);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A positive slice offset reads with its sign",
                                 std::string("+2"), slice.offset.toStdString());

    // Crosshair carries no offset, so its chip shows the group alone.
    const auto crosshair =
      m_Widget->AdvancedMatrixCell(CellId(0), QmitkMxNSyncAxis::Crosshair);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The navigation bundle links crosshair too", id, crosshair.group);
    CPPUNIT_ASSERT_MESSAGE("An offset-free dimension shows no offset", crosshair.offset.isEmpty());
  }

  void Matrix_AxisAssignAndClearRoundTrip()
  {
    const auto id = m_Widget->CreateGroup();

    m_Widget->SetCellAxisGroup(CellId(1), QmitkMxNSyncAxis::Pan, id);
    CPPUNIT_ASSERT_MESSAGE("Assigning one axis links exactly it",
                           IsLinked(1, QmitkMxNSyncDimension::Pan, id));
    CPPUNIT_ASSERT_MESSAGE("Assigning one axis leaves the others alone",
                           !m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Zoom).has_value());

    m_Widget->ClearCellAxis(CellId(1), QmitkMxNSyncAxis::Pan);
    CPPUNIT_ASSERT_MESSAGE("Clearing the axis unlinks it",
                           !m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Pan).has_value());
  }

  void Matrix_SelectionAxisClearReturnsToDefault()
  {
    // Data selection has no unlinked state, so clearing the last axis returns
    // the cell to the default group rather than leaving it in none.
    const auto id = m_Widget->CreateGroup();

    m_Widget->SetCellAxisGroup(CellId(2), QmitkMxNSyncAxis::Selection, id);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The selection axis moves to the named group", id,
                                 m_Editor->GetCellSelectionGroup(CellId(2)));

    m_Widget->ClearCellAxis(CellId(2), QmitkMxNSyncAxis::Selection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Clearing returns the selection to the default group",
                                 std::string("main"), m_Editor->GetCellSelectionGroup(CellId(2)));
  }

  void Matrix_RegroupKeepsTheAuthoredOffset()
  {
    // Moving a cell to another group changes what its offset is measured from,
    // not the relationship the user authored.
    const auto first = m_Widget->CreateGroup();
    const auto second = m_Widget->CreateGroup();

    m_Widget->SetCellAxisGroup(CellId(0), QmitkMxNSyncAxis::Slice, first);
    m_Widget->SetCellDimensionOffset(CellId(0), QmitkMxNSyncDimension::Slice, -3);
    m_Widget->SetCellAxisGroup(CellId(0), QmitkMxNSyncAxis::Slice, second);

    const auto link = m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice);
    CPPUNIT_ASSERT_MESSAGE("The cell is now in the second group",
                           link.has_value() && link->group == second);
    CPPUNIT_ASSERT_MESSAGE("The slice offset survives the move",
                           std::holds_alternative<int>(link->offset)
                             && -3 == std::get<int>(link->offset));
  }

  void Group_LinkActionsKeepAuthoredOffsets()
  {
    // A group-level link action touches only the cells it actually links; a
    // cell already on the group keeps the offset the user authored.
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, id);
    m_Widget->SetCellDimensionOffset(CellId(0), QmitkMxNSyncDimension::Slice, -2);
    m_Widget->SetCellDimensionOffset(CellId(1), QmitkMxNSyncDimension::Zoom, 2.0);
    // A third member on another axis only, so the group's slice slot is partial.
    m_Widget->SetCellAxisGroup(CellId(2), QmitkMxNSyncAxis::Pan, id);

    auto assertOffsetsKept = [this, &id](const char* after)
    {
      const auto slice = m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice);
      CPPUNIT_ASSERT_MESSAGE(std::string("Slice offset survives ") + after,
                             slice.has_value() && slice->group == id
                               && std::holds_alternative<int>(slice->offset)
                               && -2 == std::get<int>(slice->offset));
      const auto zoom = m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Zoom);
      CPPUNIT_ASSERT_MESSAGE(std::string("Zoom offset survives ") + after,
                             zoom.has_value() && zoom->group == id
                               && std::holds_alternative<double>(zoom->offset)
                               && 2.0 == std::get<double>(zoom->offset));
    };

    m_Widget->ToggleGroupAxis(id, QmitkMxNSyncAxis::Slice);
    CPPUNIT_ASSERT_MESSAGE("Linking a partial axis links the missing member",
                           IsLinked(2, QmitkMxNSyncDimension::Slice, id));
    assertOffsetsKept("linking a partial axis");

    m_Widget->LinkNavigationBundle(id);
    assertOffsetsKept("\"Link navigation\"");

    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, id,
                                 QmitkMxNGroupJoinMode::MergeOverwriteCollisions);
    assertOffsetsKept("a merge join onto the same group");
  }

  void Save_NavigationOnlyGroupRoundTripsToItself()
  {
    // A group created on the cards and joined on the navigation axes only has
    // no selection state; save and load must agree on how it is declared.
    m_Editor->ApplyLayout(m_Editor->SerializeLayout());
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, id);
    CPPUNIT_ASSERT_MESSAGE("The group joins on navigation, not selection",
                           m_Editor->GetCellSelectionGroup(CellId(0)) != id);

    const auto saved = m_Editor->SerializeLayout();
    m_Editor->ApplyLayout(saved);
    CPPUNIT_ASSERT_MESSAGE("Save and load are a fixpoint", saved == m_Editor->SerializeLayout());
  }

  void Save_NavigationOnlyGroupStaysRegistered()
  {
    // A group created on the cards stays a card after its last member leaves,
    // and a save and load must not turn it into a group that goes with its
    // last link.
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, id);

    const auto saved = m_Editor->SerializeLayout();
    CPPUNIT_ASSERT_MESSAGE("Save writes the group's select_all",
                           saved.at("groups").at(id).contains("select_all"));

    m_Editor->ApplyLayout(saved);
    m_Widget->RemoveCellsFromGroup(QStringList{ CellId(0) }, id);
    const auto infos = m_Editor->GetSyncGroupInfos();
    CPPUNIT_ASSERT_MESSAGE("The group is still a card without members",
                           std::any_of(infos.begin(), infos.end(),
                                       [&id](const auto& info) { return info.id == id; }));
  }

  void DefaultGroup_SurvivesRoundTripWithoutSelectionMembers()
  {
    // Every window's selection moves to another group; the windows stay linked
    // to the default group on windowing and LUT.
    const auto id = m_Widget->CreateGroup();
    for (std::size_t cell = 0; cell < 3; ++cell)
    {
      m_Editor->SetCellSelectionGroup(CellId(cell), id);
    }

    m_Editor->ApplyLayout(m_Editor->SerializeLayout());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The default group stays the default group",
                                 std::string("main"), m_Editor->GetDefaultSyncGroupName());
  }

  void Matrix_OffsetOnUnlinkedCellIsIgnored()
  {
    // An offset is relative to a group's reference, so a cell with no group has
    // nothing for it to be relative to.
    m_Widget->SetCellDimensionOffset(CellId(1), QmitkMxNSyncDimension::Slice, 5);

    CPPUNIT_ASSERT_MESSAGE("An unlinked cell is not linked by setting an offset",
                           !m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice).has_value());
  }

  void Matrix_SliceRampSpreadsOverCellsInOrder()
  {
    // The movie-frame case: three windows on one slice group showing -1 / 0 / +1.
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1), CellId(2) }, id);

    m_Widget->ApplySliceOffsetRamp(QStringList{ CellId(0), CellId(1), CellId(2) }, -1, 1);

    const std::array<int, 3> expected{ -1, 0, 1 };
    for (std::size_t cell = 0; cell < expected.size(); ++cell)
    {
      const auto link = m_Editor->GetSyncLink(CellId(cell), QmitkMxNSyncDimension::Slice);
      CPPUNIT_ASSERT_MESSAGE("Every ramped cell keeps its group",
                             link.has_value() && link->group == id);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The ramp steps once per cell in the given order",
                                   expected[cell], std::get<int>(link->offset));
    }
  }

  void Matrix_SliceRampShowsAMovieFrame()
  {
    // The ramp's offsets are what the windows show, the group's first window
    // included.
    const auto image = mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 8, 1.0f, 1.0f, 1.0f);
    for (std::size_t cell = 0; cell < 3; ++cell)
    {
      mitk::RenderingManager::GetInstance()->InitializeView(
        m_Editor->GetRenderWindowWidget(CellId(cell))->GetRenderWindow()->GetVtkRenderWindow(),
        image->GetTimeGeometry());
      SliceNavigation(cell)->GetStepper()->SetPos(4);
    }
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1), CellId(2) }, id);

    m_Widget->ApplySliceOffsetRamp(QStringList{ CellId(0), CellId(1), CellId(2) }, -1, 1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The first and the middle window are one slice apart",
                                 ShownSlice(0) + 1, ShownSlice(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The middle and the last window are one slice apart",
                                 ShownSlice(1) + 1, ShownSlice(2));
  }

  mitk::SliceNavigationController* SliceNavigation(std::size_t cell) const
  {
    const auto widget = m_Editor->GetRenderWindowWidget(CellId(cell));
    CPPUNIT_ASSERT(nullptr != widget);
    return widget->GetSliceNavigationController();
  }

  /** The displayed slice index: the one the navigator shows and slice offsets count. */
  unsigned int ShownSlice(std::size_t cell) const
  {
    auto* navigation = SliceNavigation(cell);
    const auto* renderer = mitk::BaseRenderer::GetInstance(
      m_Editor->GetRenderWindowWidget(CellId(cell))->GetRenderWindow()->GetVtkRenderWindow());
    const auto* stepper = navigation->GetStepper();
    const unsigned int last = stepper->GetSteps() - 1;
    const bool inverted = mitk::SliceNavigationHelper::IsSliceIndexInverted(
      navigation->GetInputWorldTimeGeometry()->GetGeometryForTimeStep(0),
      renderer->GetCurrentWorldGeometry(), navigation->GetViewDirection());
    return inverted ? last - stepper->GetPos() : stepper->GetPos();
  }

  void Matrix_TracksAnEditMadeOnTheCards()
  {
    // The two faces are two views of one state: an assignment made through the
    // cards' API shows on the matrix's chips without reopening the face.
    this->RaiseAdvancedFace();

    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(2) }, id);
    Pump();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "The chip follows an edit made on the other face", id,
      m_Widget->AdvancedMatrixCell(CellId(2), QmitkMxNSyncAxis::Slice).group);

    m_Widget->RemoveCellsFromGroup(QStringList{ CellId(2) }, id);
    Pump();

    CPPUNIT_ASSERT_MESSAGE(
      "The chip clears when the cell leaves the group",
      m_Widget->AdvancedMatrixCell(CellId(2), QmitkMxNSyncAxis::Slice)
        .group.empty());
  }

  void Matrix_DetachLeavesNoChipsBehind()
  {
    // Detaching must empty the matrix rather than leave it painting the state of
    // an editor it no longer follows.
    this->RaiseAdvancedFace();

    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, id);
    Pump();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "The attached editor's links are on the chips", id,
      m_Widget->AdvancedMatrixCell(CellId(0), QmitkMxNSyncAxis::Slice).group);

    m_Widget->SetMultiWidget(nullptr);
    Pump();

    CPPUNIT_ASSERT_MESSAGE(
      "A detached editor leaves no chip behind",
      m_Widget->AdvancedMatrixCell(CellId(0), QmitkMxNSyncAxis::Slice)
        .group.empty());
  }

  void Matrix_CtrlClickOnRowHeaderAddsTheRow()
  {
    // A plain header press selects the whole line through QTableView's own
    // handling. Qt resolves that selection without the press's modifiers, so
    // Ctrl would replace the selection; the editor's event filter takes
    // modifier-held presses and adds the line instead. The release must not
    // re-run the selection, or the Ctrl-added row would toggle straight back
    // off.
    this->RaiseAdvancedFace();
    auto* matrix = Matrix();
    auto* header = matrix->verticalHeader();

    const auto pressRow = [header](int row, Qt::KeyboardModifiers modifiers)
    {
      const QPointF pos(5.0, header->sectionViewportPosition(row) + header->sectionSize(row) / 2.0);
      const QPointF global = header->viewport()->mapToGlobal(pos);
      QMouseEvent press(QEvent::MouseButtonPress, pos, global, Qt::LeftButton, Qt::LeftButton,
                        modifiers);
      QCoreApplication::sendEvent(header->viewport(), &press);
      QMouseEvent release(QEvent::MouseButtonRelease, pos, global, Qt::LeftButton, Qt::NoButton,
                          modifiers);
      QCoreApplication::sendEvent(header->viewport(), &release);
      Pump();
    };

    pressRow(0, Qt::NoModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A plain header click takes exactly its own row", 1,
                                 static_cast<int>(matrix->selectionModel()->selectedRows().size()));

    pressRow(2, Qt::ControlModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Ctrl adds the second row rather than replacing the first", 2,
                                 static_cast<int>(matrix->selectionModel()->selectedRows().size()));
  }

  void Matrix_MixedOffsetsReadAsMultiple()
  {
    // Cells that disagree must read as disagreeing, not as whichever one came
    // first - the same contract a text editor gives a mixed font size.
    this->RaiseAdvancedFace();
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, id);
    m_Widget->SetCellDimensionOffset(CellId(0), QmitkMxNSyncDimension::Slice, 1);
    m_Widget->SetCellDimensionOffset(CellId(1), QmitkMxNSyncDimension::Slice, 4);
    Pump();

    auto* sliceOffset = m_Widget->findChild<QSpinBox*>(QStringLiteral("mxnMatrixSliceOffset"));
    CPPUNIT_ASSERT(nullptr != sliceOffset);
    const int sliceAxis = AxisIndexOf(QmitkMxNSyncDimension::Slice);

    SelectMatrixCells({ { 0, sliceAxis } });
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A single cell shows its own offset", 1, sliceOffset->value());

    SelectMatrixCells({ { 0, sliceAxis }, { 1, sliceAxis } });
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Disagreeing cells rest on the 'multiple' marker",
                                 sliceOffset->minimum(), sliceOffset->value());
    CPPUNIT_ASSERT_MESSAGE("The marker is shown as text, not as a number",
                           !sliceOffset->specialValueText().isEmpty());

    m_Widget->SetCellDimensionOffset(CellId(1), QmitkMxNSyncDimension::Slice, 1);
    Pump();
    SelectMatrixCells({ { 0, sliceAxis }, { 1, sliceAxis } });
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Agreeing cells show their shared offset", 1,
                                 sliceOffset->value());
  }

  void Matrix_MixedOffsetMarkerIsNotWritable()
  {
    // Committing the marker would flatten a mixed selection onto a value nobody
    // chose, so it is refused.
    this->RaiseAdvancedFace();
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, id);
    m_Widget->SetCellDimensionOffset(CellId(0), QmitkMxNSyncDimension::Slice, 1);
    m_Widget->SetCellDimensionOffset(CellId(1), QmitkMxNSyncDimension::Slice, 4);
    Pump();

    auto* sliceOffset = m_Widget->findChild<QSpinBox*>(QStringLiteral("mxnMatrixSliceOffset"));
    CPPUNIT_ASSERT(nullptr != sliceOffset);
    SelectMatrixCells({ { 0, AxisIndexOf(QmitkMxNSyncDimension::Slice) },
                        { 1, AxisIndexOf(QmitkMxNSyncDimension::Slice) } });

    emit sliceOffset->editingFinished();
    Pump();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "The first cell keeps its own offset", 1,
      std::get<int>(m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice)->offset));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "The second cell keeps its own offset", 4,
      std::get<int>(m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice)->offset));
  }

  void Matrix_RampHiddenForCellsInDifferentGroups()
  {
    // A ramp lays out positions within one series; offsets in different groups
    // are measured from different references, so spreading across them means nothing.
    this->RaiseAdvancedFace();
    const auto first = m_Widget->CreateGroup();
    const auto second = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, first);
    m_Widget->AssignCellsToGroup(QStringList{ CellId(2) }, second);
    Pump();

    auto* rampApply = m_Widget->findChild<QToolButton*>(QStringLiteral("mxnMatrixRampApply"));
    CPPUNIT_ASSERT(nullptr != rampApply);
    const int sliceAxis = AxisIndexOf(QmitkMxNSyncDimension::Slice);

    SelectMatrixCells({ { 0, sliceAxis }, { 1, sliceAxis } });
    CPPUNIT_ASSERT_MESSAGE("Cells of one group can be ramped",
                           rampApply->isVisibleTo(rampApply->parentWidget()));

    SelectMatrixCells({ { 0, sliceAxis }, { 2, sliceAxis } });
    CPPUNIT_ASSERT_MESSAGE("Cells of different groups cannot",
                           !rampApply->isVisibleTo(rampApply->parentWidget()));
  }

  void Matrix_ColumnGrowsToFitALongGroupName()
  {
    // A group name is only ever shortened by a column width the user chose; a
    // chip that outgrows its column widens it instead of eliding. Renaming an
    // established group is the case a structural rebuild does not cover - the
    // cell and group sets are unchanged, so only the in-place refresh runs.
    this->RaiseAdvancedFace();
    auto* matrix = Matrix();
    const int sliceAxis = AxisIndexOf(QmitkMxNSyncDimension::Slice);

    const auto id = m_Widget->CreateGroup();
    m_Widget->SetCellAxisGroup(CellId(0), QmitkMxNSyncAxis::Slice, id);
    Pump();
    const int before = matrix->columnWidth(sliceAxis);

    m_Editor->SetSyncGroupDisplayName(id, "a-deliberately-long-group-name");
    Pump();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The chip carries the whole group name",
                                 std::string("a-deliberately-long-group-name"),
                                 matrix->item(0, sliceAxis)->text().toStdString());
    CPPUNIT_ASSERT_MESSAGE("The column grows to fit the longer chip",
                           matrix->columnWidth(sliceAxis) > before);
  }

  void Matrix_EditNotifiesTheSyncFurniture()
  {
    // The matrix is not the only surface that reads link state: the per-cell
    // barcodes, frame colors and cell overlays repaint on SyncLinksChanged, so
    // an edit made here has to raise it or they go stale.
    const auto id = m_Widget->CreateGroup();

    {
      SyncNotificationCounter counter(m_Editor.get());
      m_Widget->SetCellAxisGroup(CellId(0), QmitkMxNSyncAxis::Slice, id);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Linking a cell notifies once", 1, counter.Count());
    }
    {
      SyncNotificationCounter counter(m_Editor.get());
      m_Widget->SetCellDimensionOffset(CellId(0), QmitkMxNSyncDimension::Slice, 2);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Changing an offset notifies once", 1, counter.Count());
    }
    {
      SyncNotificationCounter counter(m_Editor.get());
      m_Widget->ClearCellAxis(CellId(0), QmitkMxNSyncAxis::Slice);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Unlinking a cell notifies once", 1, counter.Count());
    }
  }

  void SyncHighlight_MatrixMarksTheSharedCells()
  {
    // The display's rings and the matrix are two views of one synchronization,
    // so a hover resolved on either marks the same set on both.
    this->RaiseAdvancedFace();
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(2) }, id);
    Pump();

    m_Widget->HighlightCellAxis(CellId(0), QmitkMxNSyncAxis::Slice);

    CPPUNIT_ASSERT_MESSAGE("The hovered cell is marked",
                           m_Widget->AdvancedMatrixCell(CellId(0), QmitkMxNSyncAxis::Slice).highlighted);
    CPPUNIT_ASSERT_MESSAGE("Its synchronized partner is marked too",
                           m_Widget->AdvancedMatrixCell(CellId(2), QmitkMxNSyncAxis::Slice).highlighted);
    CPPUNIT_ASSERT_MESSAGE("A window outside the group is not",
                           !m_Widget->AdvancedMatrixCell(CellId(1), QmitkMxNSyncAxis::Slice).highlighted);
    CPPUNIT_ASSERT_MESSAGE("Only the hovered axis is marked, not the whole row",
                           !m_Widget->AdvancedMatrixCell(CellId(0), QmitkMxNSyncAxis::Pan).highlighted);

    QStringList ringed = m_Editor->GetArrangeMode()->GetHighlightedWindowIds();
    ringed.sort();
    QStringList expected{ CellId(0), CellId(2) };
    expected.sort();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The display rings exactly the same windows",
                                 expected.join(QStringLiteral(",")).toStdString(),
                                 ringed.join(QStringLiteral(",")).toStdString());
  }

  void SyncHighlight_MatrixMarkingClearsWithTheMap()
  {
    this->RaiseAdvancedFace();
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(2) }, id);
    Pump();

    m_Widget->HighlightCellAxis(CellId(0), QmitkMxNSyncAxis::Slice);
    CPPUNIT_ASSERT(m_Widget->AdvancedMatrixCell(CellId(0), QmitkMxNSyncAxis::Slice).highlighted);

    m_Widget->ClearSyncHighlight();
    CPPUNIT_ASSERT_MESSAGE("Clearing unmarks the hovered cell",
                           !m_Widget->AdvancedMatrixCell(CellId(0), QmitkMxNSyncAxis::Slice).highlighted);
    CPPUNIT_ASSERT_MESSAGE("Clearing unmarks its partner",
                           !m_Widget->AdvancedMatrixCell(CellId(2), QmitkMxNSyncAxis::Slice).highlighted);

    // A marking left over from before a grid change must not survive onto the
    // items the rebuild puts in its place.
    m_Widget->HighlightCellAxis(CellId(0), QmitkMxNSyncAxis::Slice);
    m_Editor->SetLayout(1, 2);
    Pump();
    CPPUNIT_ASSERT_MESSAGE("A rebuild leaves no stale marking",
                           !m_Widget->AdvancedMatrixCell(CellId(0), QmitkMxNSyncAxis::Slice).highlighted);
  }

  void Offset_CellBarcodeMarksAndWordsTheOffset()
  {
    // An offset is otherwise invisible outside the matrix: a window parked at
    // slice -1 would read exactly like one sitting on its group.
    const auto id = m_Widget->CreateGroup();
    const int sliceAxis = AxisIndexOf(QmitkMxNSyncDimension::Slice);
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, id);

    auto axisSlots = m_Editor->BuildBarcodeSlots(CellId(0));
    CPPUNIT_ASSERT_MESSAGE("An unshifted cell carries no mark", !axisSlots[sliceAxis].hasOffset);
    CPPUNIT_ASSERT_MESSAGE("nor a value", axisSlots[sliceAxis].offsetText.isEmpty());

    m_Widget->SetCellDimensionOffset(CellId(0), QmitkMxNSyncDimension::Slice, -1);
    axisSlots = m_Editor->BuildBarcodeSlots(CellId(0));
    CPPUNIT_ASSERT_MESSAGE("A shifted cell is marked", axisSlots[sliceAxis].hasOffset);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("and words the shift with its sign", std::string("-1"),
                                 axisSlots[sliceAxis].offsetText.toStdString());
    CPPUNIT_ASSERT_MESSAGE("The tooltip says it too",
                           axisSlots[sliceAxis].tooltip.contains(QStringLiteral("-1")));

    const auto partner = m_Editor->BuildBarcodeSlots(CellId(1));
    CPPUNIT_ASSERT_MESSAGE("A partner that was not shifted stays unmarked",
                           !partner[sliceAxis].hasOffset);

    // An offset-free dimension can never take the mark.
    CPPUNIT_ASSERT_MESSAGE("Crosshair carries no offset",
                           !axisSlots[AxisIndexOf(QmitkMxNSyncDimension::Crosshair)].hasOffset);
  }

  void Offset_GroupBarcodeMarksWithoutNumbering()
  {
    // The group perspective speaks for several windows, so it can say that an
    // offset exists but not whose it is.
    const auto id = m_Widget->CreateGroup();
    const int sliceAxis = AxisIndexOf(QmitkMxNSyncDimension::Slice);
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, id);

    auto axisSlots = m_Widget->BuildGroupBarcodeSlots(id);
    CPPUNIT_ASSERT_MESSAGE("A group with no shifted member is unmarked",
                           !axisSlots[sliceAxis].hasOffset);

    m_Widget->SetCellDimensionOffset(CellId(1), QmitkMxNSyncDimension::Slice, 3);
    axisSlots = m_Widget->BuildGroupBarcodeSlots(id);
    CPPUNIT_ASSERT_MESSAGE("One shifted member marks the group's axis",
                           axisSlots[sliceAxis].hasOffset);
    CPPUNIT_ASSERT_MESSAGE("but the group perspective names no number",
                           axisSlots[sliceAxis].offsetText.isEmpty());

    const int selectionAxis = static_cast<int>(QmitkMxNAllSyncDimensions.size());
    CPPUNIT_ASSERT_MESSAGE("Data selection never takes the mark",
                           !axisSlots[selectionAxis].hasOffset);
  }

  void Offset_WordingIsTheSameOnEverySurface()
  {
    // The barcodes, the sync peek and the matrix all read one rule, so a shift
    // cannot be worded one way on the tile and another in the editor.
    this->RaiseAdvancedFace();
    const auto id = m_Widget->CreateGroup();
    const int sliceAxis = AxisIndexOf(QmitkMxNSyncDimension::Slice);
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, id);
    m_Widget->SetCellDimensionOffset(CellId(0), QmitkMxNSyncDimension::Slice, 2);
    Pump();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "The chip and the barcode word the shift identically",
      m_Editor->BuildBarcodeSlots(CellId(0))[sliceAxis].offsetText.toStdString(),
      m_Widget->AdvancedMatrixCell(CellId(0), QmitkMxNSyncAxis::Slice).offset.toStdString());

    // The neutral value of each dimension is not an authored shift.
    CPPUNIT_ASSERT_MESSAGE(
      "Slice 0 is no shift",
      QmitkMxNMultiWidget::FormatSyncOffset(QmitkMxNSyncDimension::Slice, 0).isEmpty());
    CPPUNIT_ASSERT_MESSAGE(
      "Zoom x1 is no shift",
      QmitkMxNMultiWidget::FormatSyncOffset(QmitkMxNSyncDimension::Zoom, 1.0).isEmpty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Zoom reads as a factor", std::string("x2"),
      QmitkMxNMultiWidget::FormatSyncOffset(QmitkMxNSyncDimension::Zoom, 2.0).toStdString());
    CPPUNIT_ASSERT_MESSAGE(
      "Crosshair has no offset to word",
      QmitkMxNMultiWidget::FormatSyncOffset(QmitkMxNSyncDimension::Crosshair, {}).isEmpty());

    // A pan is two numbers, and must not read as one: "5,-3" could pass for a
    // single value with a separator.
    mitk::Vector2D pan;
    pan[0] = 5.0;
    pan[1] = -3.0;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "A pan reads as a pair", std::string("(5|-3)"),
      QmitkMxNMultiWidget::FormatSyncOffset(QmitkMxNSyncDimension::Pan, pan).toStdString());
    CPPUNIT_ASSERT_MESSAGE(
      "A pan of zero is no shift",
      QmitkMxNMultiWidget::FormatSyncOffset(QmitkMxNSyncDimension::Pan,
                                            mitk::Vector2D(0.0)).isEmpty());
  }

  void ActionBar_NamesTheAxisWithItsGlyph()
  {
    // The bar acts on a column of the grid, so it carries that column's glyph -
    // but only when the selection is on one axis, since a glyph for "three
    // dimensions" would name none of them.
    this->RaiseAdvancedFace();
    auto* icon = m_Widget->findChild<QLabel*>(QStringLiteral("mxnMatrixAxisIcon"));
    CPPUNIT_ASSERT_MESSAGE("The action bar has an axis icon", nullptr != icon);

    const int sliceAxis = AxisIndexOf(QmitkMxNSyncDimension::Slice);
    const int panAxis = AxisIndexOf(QmitkMxNSyncDimension::Pan);

    SelectMatrixCells({ { 0, sliceAxis }, { 1, sliceAxis } });
    CPPUNIT_ASSERT_MESSAGE("One axis: the glyph shows", !icon->pixmap().isNull());

    SelectMatrixCells({ { 0, sliceAxis }, { 0, panAxis } });
    CPPUNIT_ASSERT_MESSAGE("Two axes: no glyph names the selection",
                           !icon->isVisibleTo(icon->parentWidget()));

    SelectMatrixCells({});
    CPPUNIT_ASSERT_MESSAGE("Nothing selected: no glyph",
                           !icon->isVisibleTo(icon->parentWidget()));
  }

  void ActionBar_OffsetCommitAfterDetachIsHarmless()
  {
    // Detaching drops the editor and disables the widget, which moves focus out
    // of whichever offset editor holds it - and a spin box losing focus commits.
    // So the commit can arrive with nothing left to write to.
    this->RaiseAdvancedFace();
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0) }, id);
    Pump();
    SelectMatrixCells({ { 0, AxisIndexOf(QmitkMxNSyncDimension::Slice) } });

    auto* sliceOffset = m_Widget->findChild<QSpinBox*>(QStringLiteral("mxnMatrixSliceOffset"));
    auto* zoomOffset = m_Widget->findChild<QDoubleSpinBox*>(QStringLiteral("mxnMatrixZoomOffset"));
    auto* panOffset = m_Widget->findChild<QDoubleSpinBox*>(QStringLiteral("mxnMatrixPanOffsetX"));
    CPPUNIT_ASSERT(nullptr != sliceOffset && nullptr != zoomOffset && nullptr != panOffset);

    m_Widget->SetMultiWidget(nullptr);
    Pump();

    sliceOffset->setValue(3);
    emit sliceOffset->editingFinished();
    zoomOffset->setValue(2.0);
    emit zoomOffset->editingFinished();
    panOffset->setValue(5.0);
    emit panOffset->editingFinished();
    Pump();

    CPPUNIT_ASSERT_MESSAGE("A commit after detaching changes nothing and does not crash",
                           nullptr == m_Widget->GetMultiWidget());
  }

  void Matrix_BatchNotifiesOnceForTheWholeGesture()
  {
    // One gesture, one notification: the furniture must not repaint a
    // half-applied batch, and a ramp over N cells is one edit, not N.
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1), CellId(2) }, id);

    SyncNotificationCounter counter(m_Editor.get());
    m_Widget->ApplySliceOffsetRamp(QStringList{ CellId(0), CellId(1), CellId(2) }, -1, 1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A three-cell ramp notifies once, not three times", 1,
                                 counter.Count());
  }

  void Arrange_BatchRemoveNotifiesOnce()
  {
    // Removing a plate selection from a group is one gesture: the furniture
    // must not repaint once per window, half way through the batch.
    for (std::size_t cell = 0; cell < 3; ++cell)
    {
      m_Editor->SetSyncLink(CellId(cell), QmitkMxNSyncDimension::Slice, "grp");
    }
    auto* arrangeMode = m_Editor->GetArrangeMode();
    arrangeMode->SetSelectedWindowIds(QStringList{ CellId(0), CellId(1) });
    Pump();

    SyncNotificationCounter counter(m_Editor.get());
    arrangeMode->RequestRemove(QStringLiteral("grp"), CellId(0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A two-window removal notifies once", 1, counter.Count());
    CPPUNIT_ASSERT_MESSAGE("Both selected windows leave the group",
                           !IsLinked(0, QmitkMxNSyncDimension::Slice, "grp")
                             && !IsLinked(1, QmitkMxNSyncDimension::Slice, "grp"));
    CPPUNIT_ASSERT_MESSAGE("The unselected member stays",
                           IsLinked(2, QmitkMxNSyncDimension::Slice, "grp"));
  }

  void Matrix_OffsetFocusPassWritesNothing()
  {
    // Focus passing through an offset editor commits it. Without an edit that
    // must not write, or it silently re-converges a member that drifted.
    this->RaiseAdvancedFace();
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, id);
    m_Widget->SetCellDimensionOffset(CellId(1), QmitkMxNSyncDimension::Slice, 2);
    Pump();
    SelectMatrixCells({ { 1, AxisIndexOf(QmitkMxNSyncDimension::Slice) } });

    auto* sliceOffset = m_Widget->findChild<QSpinBox*>(QStringLiteral("mxnMatrixSliceOffset"));
    CPPUNIT_ASSERT(nullptr != sliceOffset);

    SyncNotificationCounter counter(m_Editor.get());
    emit sliceOffset->editingFinished();
    Pump();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A commit without an edit writes nothing", 0, counter.Count());
  }

  void Matrix_OffsetFocusPassKeepsPrecision()
  {
    // An offset loaded with more precision than the editor shows must survive
    // the editor being focused and left.
    this->RaiseAdvancedFace();
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, id);
    mitk::Vector2D pan;
    pan[0] = 12.3456;
    pan[1] = 0.0;
    m_Widget->SetCellDimensionOffset(CellId(1), QmitkMxNSyncDimension::Pan, pan);
    Pump();
    SelectMatrixCells({ { 1, AxisIndexOf(QmitkMxNSyncDimension::Pan) } });

    auto* panOffset = m_Widget->findChild<QDoubleSpinBox*>(QStringLiteral("mxnMatrixPanOffsetX"));
    CPPUNIT_ASSERT(nullptr != panOffset);
    emit panOffset->editingFinished();
    Pump();

    const auto link = m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Pan);
    CPPUNIT_ASSERT(link.has_value() && std::holds_alternative<mitk::Vector2D>(link->offset));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The stored pan offset keeps its precision", 12.3456,
                                 std::get<mitk::Vector2D>(link->offset)[0]);
  }

  void Matrix_OffsetRealEditWrites()
  {
    // The positive control of the focus-pass tests: a value the user dials is
    // written.
    this->RaiseAdvancedFace();
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, id);
    Pump();
    SelectMatrixCells({ { 1, AxisIndexOf(QmitkMxNSyncDimension::Slice) } });

    auto* sliceOffset = m_Widget->findChild<QSpinBox*>(QStringLiteral("mxnMatrixSliceOffset"));
    CPPUNIT_ASSERT(nullptr != sliceOffset);
    sliceOffset->setValue(3);
    emit sliceOffset->editingFinished();
    Pump();

    const auto link = m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice);
    CPPUNIT_ASSERT_MESSAGE("A dialed offset is written",
                           link.has_value() && std::holds_alternative<int>(link->offset)
                             && 3 == std::get<int>(link->offset));
  }

  void Matrix_PanOffsetEditKeepsMillimetreFractions()
  {
    // A pan offset is world mm; a sub-millimetre part the user dials must be
    // written as dialed, not rounded by the editor's display precision.
    this->RaiseAdvancedFace();
    const auto id = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, id);
    Pump();
    SelectMatrixCells({ { 1, AxisIndexOf(QmitkMxNSyncDimension::Pan) } });

    auto* panX = m_Widget->findChild<QDoubleSpinBox*>(QStringLiteral("mxnMatrixPanOffsetX"));
    CPPUNIT_ASSERT(nullptr != panX);
    panX->setValue(12.345);
    emit panX->editingFinished();
    Pump();

    const auto link = m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Pan);
    CPPUNIT_ASSERT(link.has_value() && std::holds_alternative<mitk::Vector2D>(link->offset));
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The dialed millimetre fraction is stored",
                                         12.345, std::get<mitk::Vector2D>(link->offset)[0], 1e-9);
  }

  void EmptyGroupCache_DroppedOnceTheGroupGainsAMember()
  {
    // The intent prepared on an empty group is for its first members. Once the
    // group has members by any route it is spent, and must not come back when
    // the group empties again.
    const auto group = m_Widget->CreateGroup();
    Pump();
    m_Widget->ToggleGroupAxis(group, QmitkMxNSyncAxis::Slice);

    m_Widget->SetCellAxisGroup(CellId(0), QmitkMxNSyncAxis::Pan, group);
    Pump();
    m_Widget->ClearCellAxis(CellId(0), QmitkMxNSyncAxis::Pan);
    Pump();

    CPPUNIT_ASSERT_MESSAGE("The spent intent does not re-appear on the emptied group",
                           !m_Widget->BuildGroupBarcodeSlots(group)
                              [AxisIndexOf(QmitkMxNSyncDimension::Slice)].color.isValid());

    m_Widget->AssignCellsToGroup(QStringList{ CellId(1) }, group);
    for (const auto dimension : { QmitkMxNSyncDimension::Pan, QmitkMxNSyncDimension::Zoom,
                                  QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Crosshair })
    {
      CPPUNIT_ASSERT_MESSAGE("The next join gets the navigation bundle, not the spent intent",
                             IsLinked(1, dimension, group));
    }
  }

  void Matrix_FirstBuildMirrorsThePlateSelection()
  {
    // Windows selected on the plates before the matrix was ever shown are
    // selected in it once it is.
    Pump();
    m_Editor->GetArrangeMode()->SetSelectedWindowIds(QStringList{ CellId(0), CellId(2) });
    Pump();

    this->RaiseAdvancedFace();

    const auto rows = Matrix()->selectionModel()->selectedRows();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Both plate-selected windows are selected rows", 2,
                                 static_cast<int>(rows.size()));
    for (const auto& row : rows)
    {
      CPPUNIT_ASSERT_MESSAGE("Only the rows of the plate-selected windows",
                             0 == row.row() || 2 == row.row());
    }
  }

  void Assign_ReplaceDropStartsTheOffsetFresh()
  {
    // A drop onto a group is a fresh join, measured anew from the target group.
    // Only the matrix, which edits one link, carries an offset over.
    const auto first = m_Widget->CreateGroup();
    const auto second = m_Widget->CreateGroup();
    m_Widget->AssignCellsToGroup(QStringList{ CellId(0), CellId(1) }, first);
    m_Widget->SetCellDimensionOffset(CellId(1), QmitkMxNSyncDimension::Slice, 2);

    m_Widget->AssignCellsToGroup(QStringList{ CellId(1) }, second);

    const auto link = m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice);
    CPPUNIT_ASSERT_MESSAGE("The window joined the second group",
                           link.has_value() && link->group == second);
    CPPUNIT_ASSERT_MESSAGE("A replacing drop starts without an offset",
                           std::holds_alternative<int>(link->offset)
                             && 0 == std::get<int>(link->offset));
  }

  void Refresh_DeferredWhileApplyingLayout()
  {
    // While a layout is applied the cell tree is half built, and the load
    // pumps the event loop. The editor's deferred refresh must wait it out,
    // and still run once the load is over.
    Pump();
    m_Editor->ShowLayoutLoadFeedback();
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "x");
    m_Editor->RefreshSyncControls();
    Pump();
    const bool cardWhileBusy = nullptr != CardFor("x");
    m_Editor->HideLayoutLoadFeedback();

    QElapsedTimer clock;
    clock.start();
    while (nullptr == CardFor("x") && clock.elapsed() < 2000)
    {
      QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }

    CPPUNIT_ASSERT_MESSAGE("No refresh while a layout is being applied", !cardWhileBusy);
    CPPUNIT_ASSERT_MESSAGE("The refresh runs once the load is over", nullptr != CardFor("x"));
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNLayoutEditorWidget)
