/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestPopupProbe.h"
#include "QmitkTestQApplication.h"

#include <QmitkMxNArrangeMode.h>
#include <QmitkMxNCellOverlay.h>
#include <QmitkMxNGroupJoinMode.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkImageGenerator.h>
#include <mitkRenderingManager.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMouseEvent>

#include <memory>
#include <string>
#include <vector>

/**
 * Arrange mode of the MxN editor: the selection model and drop rule it holds,
 * and the plate input and cell drops the cell overlays route into it.
 *
 * Plate input is delivered to the overlay's render-window event filter
 * directly, which is where the render window's own stream reaches it; its
 * return value is exactly whether the event is kept from VTK. Routing through
 * the overlay's mask while other furniture is active, drag start (a nested
 * QDrag loop) and the BlueBerry visibility wiring are manual checks.
 */
class QmitkMxNArrangeModeTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNArrangeModeTestSuite);

  MITK_TEST(PlainClick_SelectsOnlyTheClickedCell);
  MITK_TEST(PlainClick_CollapsesMultiSelectionToClickedCell);
  MITK_TEST(PlainClick_OnTheSoleSelectedCellDeselectsIt);
  MITK_TEST(ClearSelection_DropsTheAnchor);
  MITK_TEST(ClickDeselect_DropsTheAnchor);
  MITK_TEST(PlainPress_OnASelectedCellHoldsTheSelectionForADrag);
  MITK_TEST(CtrlClick_KeepsTheRestOfTheSelection);
  MITK_TEST(ShiftClick_ReplacesSelectionWithTheRange);
  MITK_TEST(ShiftClick_KeepsTheAnchorForRespanning);
  MITK_TEST(ShiftClick_SpansTheOtherDiagonalToo);
  MITK_TEST(CtrlShiftClick_AddsTheRangeToTheSelection);
  MITK_TEST(ShiftClick_WithoutAnchor_SelectsOnlyTheClickedCell);
  MITK_TEST(RightPress_TakesAnUnselectedCellAndKeepsAMultiSelection);
  MITK_TEST(ExternalSelection_KeepsTheAnchor);
  MITK_TEST(LayoutChange_PrunesRemovedCellsAndTheirAnchor);
  MITK_TEST(RequestAssign_TargetsTheSelectionOnlyWhenDroppedOnIt);
  MITK_TEST(RequestRemove_FollowsTheSameTargetRule);
  MITK_TEST(DragPayload_CarriesTheSelectionAndTheAskModeMarker);
  MITK_TEST(Highlight_ASourceClearsOnlyItsOwn);
  MITK_TEST(JoinModeFromModifiers_MapsKeys);
  MITK_TEST(JoinModeMenuEntries_OfferEveryMode);
  MITK_TEST(ResolveJoinMode_WithoutTheMarker_ReadsModifiers);
  MITK_TEST(PlateInput_APressOnThePlateSelectsAndIsKeptFromVtk);
  MITK_TEST(Idle_ArrangingLeavesTheOverlayTransparentAndUnmasked);
  MITK_TEST(PlateInput_APressOffThePlatePassesThrough);
  MITK_TEST(PlateInput_RightPressTriggerDoesNotOpenTheMenuOnPress);
  MITK_TEST(PlateInput_NothingIsTakenOutsideArrangeMode);
  MITK_TEST(PlateInput_CleanViewKeepsThePlate);
  MITK_TEST(PlateInput_TheMenuButtonSitsLeftOfTheCloseButton);
  MITK_TEST(PlateInput_PointingAtAGlyphRingsItsPartners);
  MITK_TEST(PlateInput_TheCloseButtonAsksToHideTheEditor);
  MITK_TEST(Drops_TheCellAcceptsDropsOnlyWhileArranging);
  MITK_TEST(Drops_AGroupDropAssignsByTheTargetRule);
  MITK_TEST(Drops_OtherDragsAreLeftUnaccepted);

  CPPUNIT_TEST_SUITE_END();

  // Large enough that every cell of a 2x2 grid hosts a plate.
  static constexpr int EditorWidth = 1400;
  static constexpr int EditorHeight = 1000;

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::Image::Pointer m_Image;
  mitk::DataNode::Pointer m_ImageNode;
  std::unique_ptr<QmitkMxNMultiWidget> m_Editor;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Image = mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 8, 1.0f, 1.0f, 1.0f);
    m_ImageNode = mitk::DataNode::New();
    m_ImageNode->SetName("image");
    m_ImageNode->SetData(m_Image);
    m_DataStorage->Add(m_ImageNode);

    m_Editor = std::make_unique<QmitkMxNMultiWidget>();
    m_Editor->SetDataStorage(m_DataStorage);
    m_Editor->InitializeMultiWidget();
    m_Editor->SetLayout(3, 3);  // cells: mxn__widget0 .. mxn__widget8, row-major
    Pump();
  }

  void tearDown() override
  {
    m_Editor.reset();
    m_ImageNode = nullptr;
    m_Image = nullptr;
    m_DataStorage = nullptr;
  }

  static void Pump()
  {
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
  }

  static QString CellId(int index)
  {
    return QStringLiteral("mxn__widget") + QString::number(index);
  }

  QmitkMxNArrangeMode* Arrange() const
  {
    return m_Editor->GetArrangeMode();
  }

  /** A click on 'index''s plate as the arrange mode sees it: a press, then a
   *  release that did not become a drag. */
  void Click(int index, Qt::KeyboardModifiers modifiers = Qt::NoModifier,
             Qt::MouseButton button = Qt::LeftButton) const
  {
    this->Arrange()->PressCell(CellId(index), button, modifiers);
    this->Arrange()->ReleaseCell(false);
  }

  std::string SelectionOf() const
  {
    return this->Arrange()->GetSelectedWindowIds().join(QLatin1Char(',')).toStdString();
  }

  static std::string Ids(std::initializer_list<int> indices)
  {
    QStringList ids;
    for (int index : indices)
    {
      ids.append(CellId(index));
    }
    return ids.join(QLatin1Char(',')).toStdString();
  }

  static QDragEnterEvent DragEnter(const QMimeData* mime)
  {
    // Qt 6.12 added the QPointF constructor and deprecated the QPoint one.
#if QT_VERSION >= QT_VERSION_CHECK(6, 12, 0)
    return QDragEnterEvent(QPointF(10, 10), Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier);
#else
    return QDragEnterEvent(QPoint(10, 10), Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier);
#endif
  }

  /** Show a 2x2 editor large enough for plates and enter arrange mode. */
  void ShowArranging()
  {
    m_Editor->SetLayout(2, 2);
    m_Editor->SetSyncPeekTimings(0, 0);
    m_Editor->resize(EditorWidth, EditorHeight);
    m_Editor->show();
    Pump();
    for (const auto& [windowId, cell] : m_Editor->GetRenderWindowWidgets())
    {
      mitk::RenderingManager::GetInstance()->InitializeView(
        cell->GetRenderWindow()->GetVtkRenderWindow(), m_Image->GetTimeGeometry());
    }
    this->Arrange()->SetActive(true);
    Pump();
    CPPUNIT_ASSERT_MESSAGE("The fixture must host a plate in every cell",
                           this->Overlay(0)->IsSyncPeekVisible() && this->Overlay(0)->SyncPeekGlyphBox() > 0);
  }

  QmitkRenderWindowWidget* Cell(int index) const
  {
    const auto cell = m_Editor->GetRenderWindowWidget(CellId(index));
    CPPUNIT_ASSERT(nullptr != cell);
    return cell.get();
  }

  QmitkMxNCellOverlay* Overlay(int index) const
  {
    auto* overlay = this->Cell(index)->findChild<QmitkMxNCellOverlay*>();
    CPPUNIT_ASSERT(nullptr != overlay);
    return overlay;
  }

  /** Deliver a mouse event to cell 'index''s overlay filter as the render
   *  window's stream does, at 'overlayPosition' in overlay coordinates. Returns
   *  whether the filter kept it from the render window. */
  bool SendToRenderWindow(int index, QEvent::Type type, const QPoint& overlayPosition,
                          Qt::MouseButton button = Qt::LeftButton,
                          Qt::KeyboardModifiers modifiers = Qt::NoModifier) const
  {
    auto* renderWindow = this->Cell(index)->GetRenderWindow();
    const QPointF local(overlayPosition - renderWindow->geometry().topLeft());
    const QPointF global(renderWindow->mapToGlobal(local.toPoint()));
    const Qt::MouseButtons buttons = QEvent::MouseButtonRelease == type || QEvent::MouseMove == type
                                       ? Qt::MouseButtons(Qt::NoButton)
                                       : Qt::MouseButtons(button);
    QMouseEvent event(type, local, local, global, QEvent::MouseMove == type ? Qt::NoButton : button,
                      buttons, modifiers);
    return static_cast<QObject*>(this->Overlay(index))->eventFilter(renderWindow, &event);
  }

  // --- Selection --------------------------------------------------------------

  void PlainClick_SelectsOnlyTheClickedCell()
  {
    this->Click(5);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A click selects exactly the clicked cell", Ids({ 5 }), SelectionOf());
  }

  void PlainClick_CollapsesMultiSelectionToClickedCell()
  {
    this->Arrange()->SetSelectedWindowIds(QStringList{ CellId(0), CellId(1), CellId(2) });
    this->Click(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A plain click collapses a multi-selection to the clicked cell",
                                 Ids({ 1 }), SelectionOf());
  }

  void PlainClick_OnTheSoleSelectedCellDeselectsIt()
  {
    // Plates cover nothing but their own cell, so there is no empty space to
    // click for "select nothing"; clicking the one selected plate again is it.
    this->Click(4);
    this->Click(4);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A second click on the only selected cell deselects it",
                                 std::string(), SelectionOf());

    this->Click(4);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("...and a third selects it again", Ids({ 4 }), SelectionOf());

    this->Arrange()->PressCell(CellId(4), Qt::LeftButton, Qt::NoModifier);
    this->Arrange()->ReleaseCell(true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Dragging the sole selected cell keeps it selected", Ids({ 4 }), SelectionOf());
  }

  void ClickDeselect_DropsTheAnchor()
  {
    // Clicking the sole selected plate is how "select nothing" is done on the
    // plates, so it must leave no anchor behind, like "Clear selection".
    this->Click(4);
    this->Click(4);
    CPPUNIT_ASSERT_EQUAL(std::string(), SelectionOf());

    this->Click(8, Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A deselected cell is no anchor to span from", Ids({ 8 }), SelectionOf());
  }

  void ClearSelection_DropsTheAnchor()
  {
    this->Click(0);
    this->Arrange()->ClearSelection();
    CPPUNIT_ASSERT_EQUAL(std::string(), SelectionOf());

    this->Click(4, Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A cleared selection leaves no anchor to span from", Ids({ 4 }), SelectionOf());
  }

  void PlainPress_OnASelectedCellHoldsTheSelectionForADrag()
  {
    this->Arrange()->SetSelectedWindowIds(QStringList{ CellId(0), CellId(1), CellId(2) });
    const bool mayDrag = this->Arrange()->PressCell(CellId(1), Qt::LeftButton, Qt::NoModifier);
    CPPUNIT_ASSERT(mayDrag);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The press holds the whole selection", Ids({ 0, 1, 2 }), SelectionOf());

    this->Arrange()->ReleaseCell(true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A press that became a drag does not collapse",
                                 Ids({ 0, 1, 2 }), SelectionOf());
  }

  void CtrlClick_KeepsTheRestOfTheSelection()
  {
    this->Arrange()->SetSelectedWindowIds(QStringList{ CellId(0), CellId(1) });
    this->Click(2, Qt::ControlModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Ctrl-click adds to the selection", Ids({ 0, 1, 2 }), SelectionOf());

    this->Click(0, Qt::ControlModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Ctrl-click removes a selected cell without collapsing",
                                 Ids({ 1, 2 }), SelectionOf());
  }

  void ShiftClick_ReplacesSelectionWithTheRange()
  {
    // The range is the rectangle the anchor and the clicked cell span.
    this->Click(0);
    this->Click(4, Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shift-click selects the block the two cells span",
                                 Ids({ 0, 1, 3, 4 }), SelectionOf());

    this->Click(2, Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A second Shift-click replaces the previous range",
                                 Ids({ 0, 1, 2 }), SelectionOf());
  }

  void ShiftClick_KeepsTheAnchorForRespanning()
  {
    this->Click(4);
    this->Click(0, Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL(Ids({ 0, 1, 3, 4 }), SelectionOf());

    this->Click(8, Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The anchor survives a Shift-click", Ids({ 4, 5, 7, 8 }), SelectionOf());
  }

  void ShiftClick_SpansTheOtherDiagonalToo()
  {
    // Top right to bottom left, and back: the block must not depend on which
    // corner the range starts from.
    this->Click(2);
    this->Click(6, Qt::ShiftModifier);
    QStringList selection = this->Arrange()->GetSelectedWindowIds();
    selection.sort();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Top right to bottom left takes the whole block",
                                 Ids({ 0, 1, 2, 3, 4, 5, 6, 7, 8 }), selection.join(QLatin1Char(',')).toStdString());

    this->Click(7);
    this->Click(5, Qt::ShiftModifier);
    selection = this->Arrange()->GetSelectedWindowIds();
    selection.sort();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bottom middle to middle right takes the block between them",
                                 Ids({ 4, 5, 7, 8 }), selection.join(QLatin1Char(',')).toStdString());
  }

  void CtrlShiftClick_AddsTheRangeToTheSelection()
  {
    this->Click(0);
    this->Click(1, Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL(Ids({ 0, 1 }), SelectionOf());

    // Ctrl-click moves the anchor without disturbing the selection, so the
    // Ctrl+Shift range extends from there and adds to what is already selected.
    this->Click(6, Qt::ControlModifier);
    this->Click(7, Qt::ControlModifier | Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Ctrl+Shift adds the range instead of replacing",
                                 Ids({ 0, 1, 6, 7 }), SelectionOf());
  }

  void ShiftClick_WithoutAnchor_SelectsOnlyTheClickedCell()
  {
    this->Arrange()->SetSelectedWindowIds(QStringList{ CellId(8) });
    this->Click(4, Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A Shift-click without an anchor selects just that cell",
                                 Ids({ 4 }), SelectionOf());
  }

  void RightPress_TakesAnUnselectedCellAndKeepsAMultiSelection()
  {
    this->Click(0, Qt::NoModifier, Qt::RightButton);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A right press takes an unselected cell into the selection",
                                 Ids({ 0 }), SelectionOf());

    this->Arrange()->SetSelectedWindowIds(QStringList{ CellId(0), CellId(1), CellId(2) });
    this->Click(1, Qt::NoModifier, Qt::RightButton);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A right press on a selected cell keeps the whole selection",
                                 Ids({ 0, 1, 2 }), SelectionOf());
  }

  void ExternalSelection_KeepsTheAnchor()
  {
    this->Click(4);
    // The mirror from another surface (the map, the matrix) is not a click.
    this->Arrange()->SetSelectedWindowIds(QStringList{ CellId(8) });
    this->Click(0, Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A mirrored selection leaves the anchor where the user put it",
                                 Ids({ 0, 1, 3, 4 }), SelectionOf());
  }

  void LayoutChange_PrunesRemovedCellsAndTheirAnchor()
  {
    this->Click(8);
    this->Arrange()->SetSelectedWindowIds(QStringList{ CellId(0), CellId(8) });

    m_Editor->RemoveGridRow();  // cells 6..8 go
    Pump();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Only cells that still exist stay selected", Ids({ 0 }), SelectionOf());

    this->Click(4, Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("An anchor on a removed cell is gone", Ids({ 4 }), SelectionOf());
  }

  // --- Drop rule and payload --------------------------------------------------

  void RequestAssign_TargetsTheSelectionOnlyWhenDroppedOnIt()
  {
    std::vector<QStringList> assigned;
    QObject::connect(this->Arrange(), &QmitkMxNArrangeMode::AssignRequested,
                     [&assigned](const QString&, const QStringList& windowIds, QmitkMxNGroupJoinMode)
                     { assigned.push_back(windowIds); });

    this->Arrange()->SetSelectedWindowIds(QStringList{ CellId(0), CellId(1) });
    this->Arrange()->RequestAssign(QStringLiteral("g"), CellId(1), QmitkMxNGroupJoinMode::Replace);
    this->Arrange()->RequestAssign(QStringLiteral("g"), CellId(5), QmitkMxNGroupJoinMode::Replace);

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), assigned.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A drop on a selected cell assigns the whole selection",
                                 Ids({ 0, 1 }), assigned[0].join(QLatin1Char(',')).toStdString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A drop on an unselected cell assigns only that cell",
                                 Ids({ 5 }), assigned[1].join(QLatin1Char(',')).toStdString());
  }

  void RequestRemove_FollowsTheSameTargetRule()
  {
    std::vector<QStringList> removed;
    QObject::connect(this->Arrange(), &QmitkMxNArrangeMode::RemoveRequested,
                     [&removed](const QString&, const QStringList& windowIds) { removed.push_back(windowIds); });

    this->Arrange()->SetSelectedWindowIds(QStringList{ CellId(0), CellId(1) });
    this->Arrange()->RequestRemove(QStringLiteral("g"), CellId(0));
    this->Arrange()->RequestRemove(QStringLiteral("g"), CellId(5));

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), removed.size());
    CPPUNIT_ASSERT_EQUAL(Ids({ 0, 1 }), removed[0].join(QLatin1Char(',')).toStdString());
    CPPUNIT_ASSERT_EQUAL(Ids({ 5 }), removed[1].join(QLatin1Char(',')).toStdString());
  }

  void DragPayload_CarriesTheSelectionAndTheAskModeMarker()
  {
    this->Arrange()->SetSelectedWindowIds(QStringList{ CellId(0), CellId(1) });
    this->Arrange()->PressCell(CellId(1), Qt::LeftButton, Qt::NoModifier);
    std::unique_ptr<QMimeData> left(this->Arrange()->CreateDragMimeData());
    CPPUNIT_ASSERT_EQUAL(Ids({ 0, 1 }),
                         QString::fromUtf8(left->data(QmitkMxNCellsMimeType)).replace('\n', ',').toStdString());
    CPPUNIT_ASSERT_MESSAGE("A left drag reads its modifiers", !left->hasFormat(QmitkMxNAskModeMimeType));
    this->Arrange()->ReleaseCell(true);

    this->Arrange()->PressCell(CellId(0), Qt::RightButton, Qt::NoModifier);
    std::unique_ptr<QMimeData> right(this->Arrange()->CreateDragMimeData());
    CPPUNIT_ASSERT_MESSAGE("A right drag asks for its join mode", right->hasFormat(QmitkMxNAskModeMimeType));
  }

  void Highlight_ASourceClearsOnlyItsOwn()
  {
    auto* arrange = this->Arrange();
    arrange->SetHighlight(QmitkMxNArrangeMode::HighlightSource::Plate, QmitkMxNSyncAxis::Slice,
                          QStringList{ CellId(0) }, QColor(Qt::red));
    arrange->SetHighlight(QmitkMxNArrangeMode::HighlightSource::Editor, QmitkMxNSyncAxis::Zoom,
                          QStringList{ CellId(1) }, QColor(Qt::blue));

    arrange->ClearHighlight(QmitkMxNArrangeMode::HighlightSource::Plate);
    CPPUNIT_ASSERT_MESSAGE("The latest source owns the highlight; an earlier one cannot clear it",
                           arrange->GetHighlightAxis() == std::optional<QmitkMxNSyncAxis>(QmitkMxNSyncAxis::Zoom));

    arrange->ClearHighlight(QmitkMxNArrangeMode::HighlightSource::Editor);
    CPPUNIT_ASSERT(!arrange->GetHighlightAxis().has_value());
    CPPUNIT_ASSERT(arrange->GetHighlightedWindowIds().isEmpty());
  }

  // --- The drop vocabulary every target shares --------------------------------

  void JoinModeFromModifiers_MapsKeys()
  {
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::Replace == QmitkMxNJoinModeFromModifiers(Qt::NoModifier));
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::MergeOverwriteCollisions
                   == QmitkMxNJoinModeFromModifiers(Qt::AltModifier));
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::FillEmpty == QmitkMxNJoinModeFromModifiers(Qt::ShiftModifier));
  }

  void JoinModeMenuEntries_OfferEveryMode()
  {
    // The menu is the discoverable face of the modifiers, so it must offer the
    // same three modes - a mode reachable only by a hotkey would defeat it.
    const auto entries = QmitkMxNJoinModeMenuEntries();

    CPPUNIT_ASSERT_EQUAL(std::size_t(3), entries.size());
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::Replace == entries[0].mode);
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::MergeOverwriteCollisions == entries[1].mode);
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::FillEmpty == entries[2].mode);
    for (const auto& entry : entries)
    {
      CPPUNIT_ASSERT_MESSAGE("Every offered mode carries a label", !entry.label.isEmpty());
    }
  }

  void ResolveJoinMode_WithoutTheMarker_ReadsModifiers()
  {
    // A left-button drag carries no marker, so the modifiers decide and the
    // resolution never blocks on a menu.
    QMimeData plain;
    plain.setData(QmitkMxNCellsMimeType, QByteArray("mxn__widget0"));

    const auto replace = QmitkMxNResolveJoinMode(&plain, Qt::NoModifier, QPoint(0, 0));
    CPPUNIT_ASSERT(replace.has_value());
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::Replace == *replace);

    const auto merge = QmitkMxNResolveJoinMode(&plain, Qt::AltModifier, QPoint(0, 0));
    CPPUNIT_ASSERT(merge.has_value());
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::MergeOverwriteCollisions == *merge);
  }

  // --- Plate input ------------------------------------------------------------

  /** Whether every cell's overlay is in its idle input state: transparent for
   *  the mouse and without a mask, so the render window gets every event. */
  bool AllOverlaysIdle() const
  {
    for (const auto& [windowId, cell] : m_Editor->GetRenderWindowWidgets())
    {
      auto* overlay = cell->findChild<QmitkMxNCellOverlay*>();
      if (nullptr == overlay || !overlay->isTransparentForMouseEvents() || !overlay->mask().isEmpty())
      {
        return false;
      }
    }
    return true;
  }

  void Idle_ArrangingLeavesTheOverlayTransparentAndUnmasked()
  {
    // Arrange mode takes plate input from the render window's own stream; the
    // overlay must not start taking input itself, or VTK would lose the pointer
    // wherever the overlay's mask reaches.
    this->ShowArranging();
    this->Arrange()->SetActive(false);
    Pump();
    CPPUNIT_ASSERT_MESSAGE("Without arrange mode the overlays are idle", this->AllOverlaysIdle());

    this->Arrange()->SetActive(true);
    Pump();
    CPPUNIT_ASSERT(this->Overlay(0)->IsSyncPeekVisible());
    CPPUNIT_ASSERT_MESSAGE("Plates up, the overlays are exactly as idle", this->AllOverlaysIdle());

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "idleGroup");
    m_Editor->SetSyncLink(CellId(3), QmitkMxNSyncDimension::Slice, "idleGroup");
    this->Arrange()->SetSelectedWindowIds(QStringList{ CellId(1) });
    this->Arrange()->SetHighlight(QmitkMxNArrangeMode::HighlightSource::Editor, QmitkMxNSyncAxis::Slice,
                                  QStringList{ CellId(0), CellId(3) }, QColor(Qt::red));
    Pump();
    CPPUNIT_ASSERT(this->Overlay(0)->IsArrangeFrameBumped());
    CPPUNIT_ASSERT_MESSAGE("A selection and bumped frames keep them idle too", this->AllOverlaysIdle());
  }

  void PlateInput_APressOnThePlateSelectsAndIsKeptFromVtk()
  {
    this->ShowArranging();
    const QPoint onPlate = this->Overlay(1)->SyncPeekPlateRect().center();

    CPPUNIT_ASSERT_MESSAGE("A press on the plate is kept from VTK",
                           this->SendToRenderWindow(1, QEvent::MouseButtonPress, onPlate));
    CPPUNIT_ASSERT_MESSAGE("...and so is its release",
                           this->SendToRenderWindow(1, QEvent::MouseButtonRelease, onPlate));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The click selected the cell", Ids({ 1 }), SelectionOf());
  }

  void PlateInput_APressOffThePlatePassesThrough()
  {
    this->ShowArranging();
    const QRect plate = this->Overlay(1)->SyncPeekPlateRect();
    const QPoint offPlate(plate.left() - 20, plate.top() - 20);
    CPPUNIT_ASSERT(this->Cell(1)->GetRenderWindow()->geometry().contains(offPlate));

    CPPUNIT_ASSERT_MESSAGE("A press off the plate goes to VTK",
                           !this->SendToRenderWindow(1, QEvent::MouseButtonPress, offPlate));
    CPPUNIT_ASSERT_MESSAGE("...and so does its release",
                           !this->SendToRenderWindow(1, QEvent::MouseButtonRelease, offPlate));
    CPPUNIT_ASSERT_MESSAGE("A free move over the plate is never taken",
                           !this->SendToRenderWindow(1, QEvent::MouseMove, plate.center()));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No selection came of it", std::string(), SelectionOf());
  }

  /** Deliver a mouse context-menu request to cell 'index''s overlay filter as
   *  the render window's stream does. Returns whether the filter consumed it. */
  bool SendContextMenuToRenderWindow(int index, const QPoint& overlayPosition) const
  {
    auto* renderWindow = this->Cell(index)->GetRenderWindow();
    const QPoint local = overlayPosition - renderWindow->geometry().topLeft();
    QContextMenuEvent request(QContextMenuEvent::Mouse, local, renderWindow->mapToGlobal(local));
    return static_cast<QObject*>(this->Overlay(index))->eventFilter(renderWindow, &request);
  }

  void PlateInput_RightPressTriggerDoesNotOpenTheMenuOnPress()
  {
    // A right press on a plate may still become the ask-mode drag; a request
    // synthesized on the press (UNIX) must not open the menu over it.
    this->ShowArranging();
    const QPoint onPlate = this->Overlay(1)->SyncPeekPlateRect().center();
    QmitkTestPopupProbe probe;

    CPPUNIT_ASSERT(this->SendToRenderWindow(1, QEvent::MouseButtonPress, onPlate, Qt::RightButton));
    CPPUNIT_ASSERT_MESSAGE("The request is consumed", this->SendContextMenuToRenderWindow(1, onPlate));
    QmitkTestPopupProbe::Pump();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No plate menu opens on the press", 0, probe.Count());

    CPPUNIT_ASSERT_MESSAGE("The release on the plate is taken",
                           this->SendToRenderWindow(1, QEvent::MouseButtonRelease, onPlate, Qt::RightButton));
    QmitkTestPopupProbe::Pump();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The click opens the plate menu on its release", 1, probe.Count());
    CPPUNIT_ASSERT_MESSAGE("The press is over: a free move over the plate is not taken",
                           !this->SendToRenderWindow(1, QEvent::MouseMove, onPlate));
  }

  void PlateInput_NothingIsTakenOutsideArrangeMode()
  {
    this->ShowArranging();
    const QPoint onPlate = this->Overlay(1)->SyncPeekPlateRect().center();
    this->Arrange()->SetActive(false);

    CPPUNIT_ASSERT_MESSAGE("Without arrange mode the plate's area belongs to VTK",
                           !this->SendToRenderWindow(1, QEvent::MouseButtonPress, onPlate));
    this->SendToRenderWindow(1, QEvent::MouseButtonRelease, onPlate);
    CPPUNIT_ASSERT_EQUAL(std::string(), SelectionOf());
  }

  void PlateInput_CleanViewKeepsThePlate()
  {
    // Clean view hides the furniture, not the arrangement: the plate stays up,
    // is painted, and takes its clicks.
    this->ShowArranging();
    const QPoint onPlate = this->Overlay(1)->SyncPeekPlateRect().center();
    m_Editor->SetCleanView(true);
    CPPUNIT_ASSERT(this->Overlay(1)->IsSyncPeekVisible());

    CPPUNIT_ASSERT_MESSAGE("A press on the plate is taken in clean view too",
                           this->SendToRenderWindow(1, QEvent::MouseButtonPress, onPlate));
    this->SendToRenderWindow(1, QEvent::MouseButtonRelease, onPlate);
    CPPUNIT_ASSERT_EQUAL(Ids({ 1 }), SelectionOf());
    m_Editor->SetCleanView(false);
  }

  void PlateInput_TheMenuButtonSitsLeftOfTheCloseButton()
  {
    this->ShowArranging();
    CPPUNIT_ASSERT(!this->Overlay(1)->PlateMenuButtonRect().isValid());
    this->SendToRenderWindow(1, QEvent::MouseMove, this->Overlay(1)->SyncPeekPlateRect().center());

    const QRect plate = this->Overlay(1)->SyncPeekPlateRect();
    const QRect close = this->Overlay(1)->PlateCloseButtonRect();
    const QRect menu = this->Overlay(1)->PlateMenuButtonRect();
    CPPUNIT_ASSERT(menu.isValid());
    CPPUNIT_ASSERT_MESSAGE("Both buttons lie on the plate", plate.contains(menu) && plate.contains(close));
    CPPUNIT_ASSERT_MESSAGE("The menu button is left of the close button, apart from it",
                           menu.right() < close.left() && menu.top() == close.top());
  }

  void PlateInput_PointingAtAGlyphRingsItsPartners()
  {
    this->ShowArranging();
    // Cells 0 and 3 share a slice synchronization; cell 1 joins nothing.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "sliceGroup");
    m_Editor->SetSyncLink(CellId(3), QmitkMxNSyncDimension::Slice, "sliceGroup");
    Pump();

    const auto slot = QmitkMxNSyncAxisToSlot(QmitkMxNSyncAxis::Slice);
    const auto layout = QmitkMxNCellOverlay::ComputePeekPlate(
      this->Cell(0)->GetRenderWindow()->geometry().size(), this->Overlay(0)->SyncPeekGlyphBox(), -1,
      QmitkMxNCellOverlay::PeekTextLineHeight(this->Overlay(0)->font()), this->Overlay(0)->SyncPeekRows());
    const QPoint onSliceGlyph =
      layout.glyphs[slot].center() + this->Cell(0)->GetRenderWindow()->geometry().topLeft();

    this->SendToRenderWindow(0, QEvent::MouseMove, onSliceGlyph);
    QStringList ringed = this->Arrange()->GetHighlightedWindowIds();
    ringed.sort();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Pointing at the slice glyph rings every cell sharing it",
                                 Ids({ 0, 3 }), ringed.join(QLatin1Char(',')).toStdString());
    CPPUNIT_ASSERT_MESSAGE("...and emphasises the axis on every plate",
                           std::optional<QmitkMxNSyncAxis>(QmitkMxNSyncAxis::Slice) == this->Overlay(2)->SyncPeekAxis());
    CPPUNIT_ASSERT_MESSAGE("The partners' frames are bumped",
                           this->Overlay(0)->IsArrangeFrameBumped() && this->Overlay(3)->IsArrangeFrameBumped());
    CPPUNIT_ASSERT_MESSAGE("...and nobody else's",
                           !this->Overlay(1)->IsArrangeFrameBumped() && !this->Overlay(2)->IsArrangeFrameBumped());

    // Off the plate the plate's highlight goes.
    this->SendToRenderWindow(0, QEvent::MouseMove, this->Cell(0)->GetRenderWindow()->geometry().topLeft() + QPoint(5, 5));
    CPPUNIT_ASSERT(this->Arrange()->GetHighlightedWindowIds().isEmpty());
  }

  void PlateInput_TheCloseButtonAsksToHideTheEditor()
  {
    this->ShowArranging();
    std::vector<QmitkMxNMultiWidget::LayoutEditorRequest> requests;
    QObject::connect(m_Editor.get(), &QmitkMxNMultiWidget::LayoutEditorRequested,
                     [&requests](QmitkMxNMultiWidget::LayoutEditorRequest request) { requests.push_back(request); });

    CPPUNIT_ASSERT_MESSAGE("The button shows only on the plate under the pointer",
                           !this->Overlay(1)->PlateCloseButtonRect().isValid());
    this->SendToRenderWindow(1, QEvent::MouseMove, this->Overlay(1)->SyncPeekPlateRect().center());
    const QRect button = this->Overlay(1)->PlateCloseButtonRect();
    CPPUNIT_ASSERT(button.isValid());

    CPPUNIT_ASSERT(this->SendToRenderWindow(1, QEvent::MouseButtonPress, button.center()));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), requests.size());
    CPPUNIT_ASSERT_MESSAGE("The close button asks to hide the layout editor, never to toggle it",
                           QmitkMxNMultiWidget::LayoutEditorRequest::Hide == requests[0]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Closing selects nothing", std::string(), SelectionOf());
  }

  // --- Drops ------------------------------------------------------------------

  void Drops_TheCellAcceptsDropsOnlyWhileArranging()
  {
    CPPUNIT_ASSERT(!this->Cell(0)->acceptDrops());
    this->Arrange()->SetActive(true);
    CPPUNIT_ASSERT(this->Cell(0)->acceptDrops());
    this->Arrange()->SetActive(false);
    CPPUNIT_ASSERT(!this->Cell(0)->acceptDrops());
    CPPUNIT_ASSERT_MESSAGE("The render window's own drop handling stays off",
                           !this->Cell(0)->GetRenderWindow()->acceptDrops());
  }

  void Drops_AGroupDropAssignsByTheTargetRule()
  {
    this->Arrange()->SetActive(true);
    std::vector<QStringList> assigned;
    QObject::connect(this->Arrange(), &QmitkMxNArrangeMode::AssignRequested,
                     [&assigned](const QString& group, const QStringList& windowIds, QmitkMxNGroupJoinMode mode)
                     {
                       CPPUNIT_ASSERT(QStringLiteral("g1") == group);
                       CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::Replace == mode);
                       assigned.push_back(windowIds);
                     });

    QMimeData mime;
    mime.setData(QmitkMxNGroupMimeType, QByteArray("g1"));
    auto* filter = static_cast<QObject*>(this->Overlay(4));

    auto enter = DragEnter(&mime);
    CPPUNIT_ASSERT(filter->eventFilter(this->Cell(4), &enter));
    CPPUNIT_ASSERT_MESSAGE("A group drag is accepted anywhere on the cell", enter.isAccepted());
    CPPUNIT_ASSERT_MESSAGE("...and marks the cell's frame as the target", this->Overlay(4)->IsArrangeFrameBumped());

    this->Arrange()->SetSelectedWindowIds(QStringList{ CellId(4), CellId(5) });
    QDropEvent drop(QPointF(10, 10), Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier);
    CPPUNIT_ASSERT(filter->eventFilter(this->Cell(4), &drop));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), assigned.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A drop on a selected cell assigns the selection",
                                 Ids({ 4, 5 }), assigned[0].join(QLatin1Char(',')).toStdString());
  }

  void Drops_OtherDragsAreLeftUnaccepted()
  {
    this->Arrange()->SetActive(true);
    auto* filter = static_cast<QObject*>(this->Overlay(0));

    // Unaccepted, Qt offers the drag to the cell's ancestors: a file drag still
    // reaches the editor area, a data-node drag still finds no taker here.
    for (const auto& format : { QStringLiteral("application/x-mitk-datanodes"), QStringLiteral("text/uri-list") })
    {
      QMimeData mime;
      mime.setData(format, QByteArray("x"));
      auto enter = DragEnter(&mime);
      filter->eventFilter(this->Cell(0), &enter);
      CPPUNIT_ASSERT_MESSAGE("Only group drags are taken on a cell", !enter.isAccepted());
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNArrangeMode)
