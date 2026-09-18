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
#include <QmitkMxNMultiWidget.h>

#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QCoreApplication>
#include <QMimeData>
#include <QMouseEvent>
#include <QPointF>

#include <string>

/**
 * Drives QmitkMxNCellMapWidget directly - its selection gestures and the drop
 * vocabulary it owns for both drop targets. The editor's use of the map (the
 * active-window mirror, group-card drops, sync-highlight resolution) is covered
 * by QmitkMxNLayoutEditorWidgetTest.
 *
 * The map is sized explicitly so its tiles have real geometry: the gestures go
 * through the widget's own hit-test, and the uniform-grid path derives tile rects
 * from the row/column count, so the editor itself needs no on-screen size.
 */
class QmitkMxNCellMapWidgetTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNCellMapWidgetTestSuite);

  MITK_TEST(PlainClick_SelectsOnlyTheClickedTile);
  MITK_TEST(PlainClick_CollapsesMultiSelectionToClickedTile);
  MITK_TEST(CtrlClick_KeepsTheRestOfTheSelection);
  MITK_TEST(ShiftClick_ReplacesSelectionWithTheRange);
  MITK_TEST(ShiftClick_KeepsTheAnchorForRespanning);
  MITK_TEST(CtrlShiftClick_AddsTheRangeToTheSelection);
  MITK_TEST(ShiftClick_WithoutAnchor_SelectsOnlyTheClickedTile);
  MITK_TEST(EmptyAreaPress_ClearsSelectionAndAnchor);
  MITK_TEST(RightPress_TakesAnUnselectedTileAndKeepsAMultiSelection);
  MITK_TEST(JoinModeFromModifiers_MapsKeys);
  MITK_TEST(JoinModeMenuEntries_OfferEveryMode);
  MITK_TEST(ResolveJoinMode_WithoutTheMarker_ReadsModifiers);

  CPPUNIT_TEST_SUITE_END();

  static constexpr int MapExtent = 300;  // a 3x3 map of 100 px tiles

  mitk::DataStorage::Pointer m_DataStorage;
  std::unique_ptr<QmitkMxNMultiWidget> m_Editor;
  std::unique_ptr<QmitkMxNCellMapWidget> m_CellMap;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();

    m_Editor = std::make_unique<QmitkMxNMultiWidget>();
    m_Editor->SetDataStorage(m_DataStorage);
    m_Editor->InitializeMultiWidget();
    m_Editor->SetLayout(3, 3);  // cells: mxn__widget0 .. mxn__widget8, row-major

    m_CellMap = std::make_unique<QmitkMxNCellMapWidget>();
    m_CellMap->SetMultiWidget(m_Editor.get());
    m_CellMap->resize(MapExtent, MapExtent);
    // A hidden, parentless widget is not reliably delivered a resize event, so
    // re-derive the tile rects at the new size explicitly rather than wait for one.
    m_CellMap->Rebuild();
    Pump();
  }

  void tearDown() override
  {
    m_CellMap.reset();
    m_Editor.reset();
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

  /** The centre of the tile at 'row'/'column' of the 3x3 map. */
  static QPoint TileCentre(int row, int column)
  {
    const int step = MapExtent / 3;
    return QPoint(column * step + step / 2, row * step + step / 2);
  }

  /** A click: press and release at the same spot, dispatched to the widget's own
   *  handlers. */
  void ClickAt(const QPoint& position, Qt::KeyboardModifiers modifiers = Qt::NoModifier,
               Qt::MouseButton button = Qt::LeftButton) const
  {
    const QPointF local(position);
    QMouseEvent press(QEvent::MouseButtonPress, local, local, local, button, button, modifiers);
    QMouseEvent release(QEvent::MouseButtonRelease, local, local, local, button, Qt::NoButton,
                        modifiers);
    auto* target = static_cast<QObject*>(m_CellMap.get());
    target->event(&press);
    target->event(&release);
  }

  /** The selection as a printable string; CppUnit cannot stringify a QStringList,
   *  and the joined form keeps the failure message readable. */
  std::string SelectionOf() const
  {
    return m_CellMap->GetSelectedWindowIds().join(QLatin1Char(',')).toStdString();
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

  // --- Selection gestures -----------------------------------------------------

  void PlainClick_SelectsOnlyTheClickedTile()
  {
    // Also the guard for every gesture below: it only holds when the hit-test
    // resolves tiles, so a geometry failure surfaces here rather than as a
    // vacuously passing range assertion.
    ClickAt(TileCentre(1, 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A click selects exactly the tile under the pointer",
                                 Ids({ 5 }), SelectionOf());
  }

  void PlainClick_CollapsesMultiSelectionToClickedTile()
  {
    // Tiles cover the whole map bar a 2 px gutter, so a plain click is the only
    // practical way back to a single-cell selection. The press holds the wider
    // selection for a possible drag; the release resolves it.
    m_CellMap->SetSelectedWindowIds(QStringList{ CellId(0), CellId(1), CellId(2) });
    Pump();

    ClickAt(TileCentre(0, 1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A plain click collapses a multi-selection to the clicked tile",
                                 Ids({ 1 }), SelectionOf());
  }

  void CtrlClick_KeepsTheRestOfTheSelection()
  {
    m_CellMap->SetSelectedWindowIds(QStringList{ CellId(0), CellId(1) });
    Pump();

    ClickAt(TileCentre(0, 2), Qt::ControlModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Ctrl-click adds to the selection", Ids({ 0, 1, 2 }), SelectionOf());

    ClickAt(TileCentre(0, 0), Qt::ControlModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Ctrl-click removes a selected tile without collapsing",
                                 Ids({ 1, 2 }), SelectionOf());
  }

  void ShiftClick_ReplacesSelectionWithTheRange()
  {
    // The range is the rectangle the anchor and the clicked tile span, so a
    // diagonal Shift-click takes the whole block between them, not a row run.
    ClickAt(TileCentre(0, 0));
    ClickAt(TileCentre(1, 1), Qt::ShiftModifier);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shift-click selects the block the two tiles span",
                                 Ids({ 0, 1, 3, 4 }), SelectionOf());

    // Replace, not accumulate: a second Shift-click from the same anchor yields
    // only the new range.
    ClickAt(TileCentre(0, 2), Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A second Shift-click replaces the previous range",
                                 Ids({ 0, 1, 2 }), SelectionOf());
  }

  void ShiftClick_KeepsTheAnchorForRespanning()
  {
    ClickAt(TileCentre(1, 1));
    ClickAt(TileCentre(0, 0), Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL(Ids({ 0, 1, 3, 4 }), SelectionOf());

    // The anchor is still the plain-clicked centre tile, so spanning the other
    // way round works without re-establishing it.
    ClickAt(TileCentre(2, 2), Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The anchor survives a Shift-click", Ids({ 4, 5, 7, 8 }),
                                 SelectionOf());
  }

  void CtrlShiftClick_AddsTheRangeToTheSelection()
  {
    ClickAt(TileCentre(0, 0));
    ClickAt(TileCentre(0, 1), Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL(Ids({ 0, 1 }), SelectionOf());

    // Ctrl-click moves the anchor without disturbing the selection, so the
    // Ctrl+Shift range extends from there and adds to what is already selected.
    ClickAt(TileCentre(2, 0), Qt::ControlModifier);
    ClickAt(TileCentre(2, 1), Qt::ControlModifier | Qt::ShiftModifier);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Ctrl+Shift adds the range instead of replacing",
                                 Ids({ 0, 1, 6, 7 }), SelectionOf());
  }

  void ShiftClick_WithoutAnchor_SelectsOnlyTheClickedTile()
  {
    // Nothing has been clicked yet, so there is no origin to span from; the
    // gesture degrades to a plain click rather than guessing one.
    m_CellMap->SetSelectedWindowIds(QStringList{ CellId(8) });
    Pump();

    ClickAt(TileCentre(1, 1), Qt::ShiftModifier);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A Shift-click without an anchor selects just that tile",
                                 Ids({ 4 }), SelectionOf());
  }

  void EmptyAreaPress_ClearsSelectionAndAnchor()
  {
    ClickAt(TileCentre(0, 0));
    CPPUNIT_ASSERT_EQUAL(Ids({ 0 }), SelectionOf());

    // The 2 px gutter between the first two tile columns is the only empty area
    // in a filled grid.
    ClickAt(QPoint(99, 50));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A press on empty space clears the selection", std::string(),
                                 SelectionOf());

    // The anchor went with it, so the next Shift-click cannot span from a tile
    // the user can no longer see selected.
    ClickAt(TileCentre(1, 1), Qt::ShiftModifier);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Clearing the selection drops the range anchor", Ids({ 4 }),
                                 SelectionOf());
  }

  void RightPress_TakesAnUnselectedTileAndKeepsAMultiSelection()
  {
    // A right press arms the join-mode-asking drag. It mirrors the left button's
    // selection handling so the drag carries what the user pointed at, but never
    // collapses on release - that is the plain left click's contract.
    ClickAt(TileCentre(0, 0), Qt::NoModifier, Qt::RightButton);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A right press takes an unselected tile into the selection",
                                 Ids({ 0 }), SelectionOf());

    m_CellMap->SetSelectedWindowIds(QStringList{ CellId(0), CellId(1), CellId(2) });
    Pump();
    ClickAt(TileCentre(0, 1), Qt::NoModifier, Qt::RightButton);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A right press on a selected tile keeps the whole selection",
                                 Ids({ 0, 1, 2 }), SelectionOf());
  }

  // --- The drop vocabulary both targets share ---------------------------------

  void JoinModeFromModifiers_MapsKeys()
  {
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::Replace
                   == QmitkMxNCellMapWidget::JoinModeFromModifiers(Qt::NoModifier));
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::MergeOverwriteCollisions
                   == QmitkMxNCellMapWidget::JoinModeFromModifiers(Qt::AltModifier));
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::FillEmpty
                   == QmitkMxNCellMapWidget::JoinModeFromModifiers(Qt::ShiftModifier));
  }

  void JoinModeMenuEntries_OfferEveryMode()
  {
    // The menu is the discoverable face of the modifiers, so it must offer the
    // same three modes - a mode reachable only by a hotkey would defeat it.
    const auto entries = QmitkMxNCellMapWidget::JoinModeMenuEntries();

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
    // A left-button drag carries no marker, so the modifiers still decide and the
    // resolution never blocks on a menu.
    QMimeData plain;
    plain.setData(QmitkMxNCellMapWidget::CellsMimeType, QByteArray("mxn__widget0"));

    const auto replace = QmitkMxNCellMapWidget::ResolveJoinMode(&plain, Qt::NoModifier, nullptr,
                                                                QPoint(0, 0));
    CPPUNIT_ASSERT(replace.has_value());
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::Replace == *replace);

    const auto merge = QmitkMxNCellMapWidget::ResolveJoinMode(&plain, Qt::AltModifier, nullptr,
                                                              QPoint(0, 0));
    CPPUNIT_ASSERT(merge.has_value());
    CPPUNIT_ASSERT(QmitkMxNGroupJoinMode::MergeOverwriteCollisions == *merge);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNCellMapWidget)
