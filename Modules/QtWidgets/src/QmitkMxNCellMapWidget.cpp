/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNCellMapWidget.h"

#include <QmitkMxNMultiWidget.h>
#include <QmitkMxNSyncBarcodeWidget.h>
#include <QmitkMxNSyncDimension.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkException.h>

#include <QApplication>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>

#include <algorithm>

const char* QmitkMxNCellMapWidget::CellsMimeType = "application/x-mitk-mxn-cells";
const char* QmitkMxNCellMapWidget::GroupMimeType = "application/x-mitk-mxn-group";
const char* QmitkMxNCellMapWidget::AskModeMimeType = "application/x-mitk-mxn-askmode";

namespace
{
  constexpr int TileSpacing = 2;

  QString BareCellLabel(const QmitkMxNMultiWidget::WindowDescriptor& descriptor)
  {
    if (!descriptor.displayName.isEmpty())
    {
      return descriptor.displayName;
    }
    const auto separator = descriptor.id.indexOf(QStringLiteral("__"));
    return separator >= 0 ? descriptor.id.mid(separator + 2) : descriptor.id;
  }

}

QmitkMxNCellMapWidget::QmitkMxNCellMapWidget(QWidget* parent)
  : QWidget(parent)
{
  this->setAcceptDrops(true);
  this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  // Track motion without a pressed button so hovering a tile's axis glyph can
  // report which synchronization the pointer is over.
  this->setMouseTracking(true);
}

QmitkMxNCellMapWidget::~QmitkMxNCellMapWidget()
{
}

void QmitkMxNCellMapWidget::SetMultiWidget(QmitkMxNMultiWidget* multiWidget)
{
  m_MultiWidget = multiWidget;
  this->Rebuild();
}

void QmitkMxNCellMapWidget::Rebuild()
{
  m_Tiles.clear();

  if (!m_MultiWidget.isNull())
  {
    std::vector<QmitkMxNMultiWidget::WindowDescriptor> descriptors;
    try
    {
      descriptors = m_MultiWidget->ListWindowDescriptors();
    }
    catch (const mitk::Exception&)
    {
      descriptors.clear();  // transient mid-layout-change state
    }

    // For a regular grid, lay the tiles out uniformly from the row/column count
    // and the row-major descriptor order. This is robust right after a structural
    // change (e.g. adding a row): the live cell geometry is not yet updated by
    // Qt's layout pass at that moment, so reading it would collapse every tile to
    // the top-left. Irregular (loaded) layouts fall back to mirroring the real
    // on-screen proportions, which are valid once the layout has settled.
    const int rows = m_MultiWidget->GetRowCount();
    const int columns = m_MultiWidget->GetColumnCount();
    const bool uniformGrid = rows > 0 && columns > 0
                             && static_cast<int>(descriptors.size()) == rows * columns;
    const QRect editorRect = m_MultiWidget->rect();
    int index = 0;
    for (const auto& descriptor : descriptors)
    {
      const auto cell = m_MultiWidget->GetRenderWindowWidget(descriptor.id);
      if (nullptr == cell)
      {
        ++index;
        continue;
      }

      Tile tile;
      tile.windowId = descriptor.id;
      tile.label = BareCellLabel(descriptor);

      if (uniformGrid)
      {
        const int row = index / columns;
        const int column = index % columns;
        tile.normalizedRect = QRectF(static_cast<qreal>(column) / columns,
                                     static_cast<qreal>(row) / rows,
                                     1.0 / columns, 1.0 / rows);
      }
      else if (editorRect.width() > 0 && editorRect.height() > 0)
      {
        const QRect cellRect(cell->mapTo(m_MultiWidget, QPoint(0, 0)), cell->size());
        tile.normalizedRect = QRectF(
          static_cast<qreal>(cellRect.x()) / editorRect.width(),
          static_cast<qreal>(cellRect.y()) / editorRect.height(),
          static_cast<qreal>(cellRect.width()) / editorRect.width(),
          static_cast<qreal>(cellRect.height()) / editorRect.height());
      }
      m_Tiles.push_back(std::move(tile));
      ++index;
    }
  }

  // Drop selected ids whose cells are gone.
  QStringList survivingSelection;
  for (const auto& windowId : m_Selection)
  {
    if (std::any_of(m_Tiles.begin(), m_Tiles.end(),
                    [&windowId](const Tile& tile) { return tile.windowId == windowId; }))
    {
      survivingSelection.append(windowId);
    }
  }
  if (survivingSelection.size() != m_Selection.size())
  {
    this->SetSelection(survivingSelection);
  }

  // A range anchor pointing at a removed cell would span from nowhere; dropping
  // it degrades the next Shift-click to a plain one rather than a stale range.
  if (!m_SelectionAnchor.isEmpty()
      && std::none_of(m_Tiles.begin(), m_Tiles.end(),
                      [this](const Tile& tile) { return tile.windowId == m_SelectionAnchor; }))
  {
    m_SelectionAnchor.clear();
  }

  // The sync highlight is keyed by window id (transient hover state), so keep it
  // only for cells that still exist; the tile-index hover state is invalidated
  // by the rebuild and re-establishes on the next move.
  QStringList survivingHighlight;
  for (const auto& windowId : m_HighlightCells)
  {
    if (std::any_of(m_Tiles.begin(), m_Tiles.end(),
                    [&windowId](const Tile& tile) { return tile.windowId == windowId; }))
    {
      survivingHighlight.append(windowId);
    }
  }
  if (survivingHighlight.size() != m_HighlightCells.size())
  {
    m_HighlightCells = survivingHighlight;
    if (m_HighlightCells.isEmpty())
    {
      m_HighlightAxis = -1;
    }
  }
  m_HoverTile = -1;
  m_HoverSlot = -1;

  this->UpdateTileRects();
  this->update();
}

QStringList QmitkMxNCellMapWidget::GetSelectedWindowIds() const
{
  return m_Selection;
}

void QmitkMxNCellMapWidget::SetSelectedWindowIds(const QStringList& windowIds)
{
  this->SetSelection(windowIds);
}

void QmitkMxNCellMapWidget::SetHighlightedCells(const QStringList& windowIds, int axisIndex,
                                                const QColor& hue)
{
  if (windowIds == m_HighlightCells && axisIndex == m_HighlightAxis && hue == m_HighlightHue)
  {
    return;
  }
  m_HighlightCells = windowIds;
  m_HighlightAxis = axisIndex;
  m_HighlightHue = hue;
  this->update();
}

QStringList QmitkMxNCellMapWidget::GetHighlightedWindowIds() const
{
  return m_HighlightCells;
}

void QmitkMxNCellMapWidget::UpdateTileRects()
{
  const QRect area = this->rect().adjusted(1, 1, -1, -1);
  for (auto& tile : m_Tiles)
  {
    tile.mapRect = QRect(area.x() + qRound(tile.normalizedRect.x() * area.width()),
                         area.y() + qRound(tile.normalizedRect.y() * area.height()),
                         qRound(tile.normalizedRect.width() * area.width()) - TileSpacing,
                         qRound(tile.normalizedRect.height() * area.height()) - TileSpacing);
  }
}

QRect QmitkMxNCellMapWidget::TileBarcodeRect(const Tile& tile) const
{
  const int barcodeBand = std::clamp(tile.mapRect.height() / 2, 8, 56);
  return QRect(tile.mapRect.left() + 2, tile.mapRect.bottom() - barcodeBand - 1,
               tile.mapRect.width() - 4, barcodeBand);
}

QSize QmitkMxNCellMapWidget::minimumSizeHint() const
{
  return QSize(160, 120);
}

void QmitkMxNCellMapWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  this->UpdateTileRects();
}

void QmitkMxNCellMapWidget::paintEvent(QPaintEvent* /*event*/)
{
  if (m_MultiWidget.isNull())
  {
    return;
  }

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, false);

  for (std::size_t tileIndex = 0; tileIndex < m_Tiles.size(); ++tileIndex)
  {
    const auto& tile = m_Tiles[tileIndex];
    if (!tile.mapRect.isValid())
    {
      continue;
    }

    // Neutral tile background so the glyphs below stay legible; the group hue
    // (when the cell is cleanly in one group, the same ResolveCellGroupIdentity
    // rule as the render-window frame) rides a thin top bar - the group cards'
    // header idiom - rather than flooding the whole tile.
    painter.fillRect(tile.mapRect, this->palette().color(QPalette::Base));

    const auto identity = m_MultiWidget->ResolveCellGroupIdentity(tile.windowId);
    if (identity.kind == QmitkMxNMultiWidget::CellGroupIdentityKind::Mono && identity.hue.isValid())
    {
      const int barHeight = std::min(5, tile.mapRect.height());
      painter.fillRect(QRect(tile.mapRect.left(), tile.mapRect.top(), tile.mapRect.width(), barHeight),
                       identity.hue);
    }

    // Highlight the tile a dragged group would drop onto.
    const bool dropTarget = static_cast<int>(tileIndex) == m_DropTargetTile;
    if (dropTarget)
    {
      QColor tint = this->palette().color(QPalette::Highlight);
      tint.setAlpha(70);
      painter.fillRect(tile.mapRect, tint);
    }

    const bool selected = m_Selection.contains(tile.windowId);
    painter.setPen(QPen(dropTarget || selected ? this->palette().color(QPalette::Highlight)
                                               : this->palette().color(QPalette::Mid),
                        dropTarget || selected ? 2 : 1));
    painter.drawRect(tile.mapRect.adjusted(0, 0, -1, -1));

    // Sync-highlight ring: a cell sharing the hovered (group, axis) is ringed in
    // that group's hue, inset from the border so it reads distinctly from the
    // blue selection/drop-target border a cell may also carry.
    const bool highlighted = m_HighlightCells.contains(tile.windowId);
    if (highlighted && m_HighlightHue.isValid())
    {
      painter.setPen(QPen(m_HighlightHue, 2));
      painter.drawRect(tile.mapRect.adjusted(1, 1, -2, -2));
    }

    // A barcode band in the tile's lower portion, tall enough to wrap glyphs
    // when the tile has the room and collapsing to color slots when it does not.
    const QRect barcodeRect = this->TileBarcodeRect(tile);
    const int barcodeBand = barcodeRect.height();

    // The cell label, above the barcode band.
    painter.setPen(this->palette().color(QPalette::Text));
    const QRect textRect = tile.mapRect.adjusted(3, 2, -3, -barcodeBand - 3);
    painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap, tile.label);

    // Sync barcode via the shared renderer, so the tile tells the same story as
    // the per-cell strip: eight axes (the seven dimensions plus data selection)
    // as wrapping glyphs, or color slots when the band is too small. Built from
    // this cell's window perspective (each axis linked or not). Brighten the
    // shared axis on a highlighted cell, else the locally hovered glyph, so the
    // hovered synchronization pops on every cell that shares it.
    const int litSlot = highlighted ? m_HighlightAxis
                        : (static_cast<int>(tileIndex) == m_HoverTile ? m_HoverSlot : -1);
    QmitkMxNSyncBarcodeWidget::PaintInto(painter, barcodeRect,
                                         m_MultiWidget->BuildBarcodeSlots(tile.windowId),
                                         false, this->palette().color(QPalette::Mid), litSlot);
  }
}

int QmitkMxNCellMapWidget::TileAt(const QPoint& position) const
{
  for (std::size_t i = 0; i < m_Tiles.size(); ++i)
  {
    if (m_Tiles[i].mapRect.contains(position))
    {
      return static_cast<int>(i);
    }
  }
  return -1;
}

void QmitkMxNCellMapWidget::SetSelection(const QStringList& windowIds)
{
  if (windowIds == m_Selection)
  {
    return;
  }
  m_Selection = windowIds;
  emit SelectionChanged(m_Selection);
  this->update();
}

void QmitkMxNCellMapWidget::mousePressEvent(QMouseEvent* event)
{
  const bool leftButton = Qt::LeftButton == event->button();
  const bool rightButton = Qt::RightButton == event->button();
  if (!leftButton && !rightButton)
  {
    event->ignore();
    return;
  }

  m_PressPosition = event->pos();
  const int index = this->TileAt(event->pos());

  if (index < 0)
  {
    // Empty area: clear the selection. Multi-select is by Ctrl- and Shift-click.
    m_DragCandidate = false;
    m_PressedWindowId.clear();
    m_SelectionAnchor.clear();
    this->SetSelection(QStringList());
    event->accept();
    return;
  }

  const auto& windowId = m_Tiles[static_cast<std::size_t>(index)].windowId;

  // A right press arms a drag that will ask for its join mode on drop, and
  // otherwise behaves like the left one: it takes an unselected tile into the
  // selection so the drag carries what the user pointed at.
  if (rightButton)
  {
    if (!m_Selection.contains(windowId))
    {
      this->SetSelection(QStringList{ windowId });
      m_SelectionAnchor = windowId;
    }
    m_PressedWindowId.clear();  // only a left click collapses on release
    m_DragCandidate = true;
    m_DragAsksMode = true;
    event->accept();
    return;
  }

  if (event->modifiers().testFlag(Qt::ShiftModifier))
  {
    // Range select from the anchor, the file-explorer way: Shift replaces the
    // selection with the range, Ctrl+Shift adds it. The anchor stays put so the
    // range can be re-spanned from the same origin. A range press never becomes a
    // drag - Shift is also the FillEmpty drop modifier, and dragging out of a
    // range press would conflate choosing cells with choosing a join mode.
    const QStringList range = this->TilesBetween(m_SelectionAnchor, windowId);
    if (event->modifiers().testFlag(Qt::ControlModifier))
    {
      auto selection = m_Selection;
      for (const auto& id : range)
      {
        if (!selection.contains(id))
        {
          selection.append(id);
        }
      }
      this->SetSelection(selection);
    }
    else
    {
      this->SetSelection(range);
    }
    if (m_SelectionAnchor.isEmpty())
    {
      m_SelectionAnchor = windowId;
    }
    m_DragCandidate = false;
    m_PressedWindowId.clear();
  }
  else if (event->modifiers().testFlag(Qt::ControlModifier))
  {
    auto selection = m_Selection;
    if (selection.contains(windowId))
    {
      selection.removeAll(windowId);
    }
    else
    {
      selection.append(windowId);
    }
    this->SetSelection(selection);
    m_SelectionAnchor = windowId;
    m_DragCandidate = false;
    m_PressedWindowId.clear();
  }
  else
  {
    // Pressing an already-selected tile holds the selection so a multi-tile drag
    // can carry all of it; the release collapses to the pressed tile when no drag
    // follows. Tiles cover the map bar a 2 px gutter, so without that collapse a
    // full selection would have no reachable way back to a single cell.
    if (!m_Selection.contains(windowId))
    {
      this->SetSelection(QStringList{ windowId });
    }
    m_PressedWindowId = windowId;
    m_SelectionAnchor = windowId;
    m_DragCandidate = true;
  }

  m_DragAsksMode = false;
  event->accept();
}

void QmitkMxNCellMapWidget::mouseMoveEvent(QMouseEvent* event)
{
  if (m_DragCandidate
      && (event->pos() - m_PressPosition).manhattanLength() >= QApplication::startDragDistance())
  {
    m_DragCandidate = false;
    this->StartCellDrag();
    event->accept();
    return;
  }

  // Plain hover (no drag in progress): report the axis glyph under the pointer
  // so the editor can light up every cell sharing that synchronization.
  if (!m_DragCandidate)
  {
    this->UpdateGlyphHover(event->pos());
  }

  event->ignore();
}

void QmitkMxNCellMapWidget::mouseReleaseEvent(QMouseEvent* event)
{
  // A plain press that held a wider selection (see mousePressEvent) and never
  // grew into a drag resolves here: the click meant "select just this tile".
  const bool collapseToPressedTile = m_DragCandidate
                                     && Qt::LeftButton == event->button()
                                     && !m_PressedWindowId.isEmpty();

  m_DragCandidate = false;
  m_DragAsksMode = false;

  if (collapseToPressedTile)
  {
    this->SetSelection(QStringList{ m_PressedWindowId });
  }
  m_PressedWindowId.clear();

  event->ignore();
}

void QmitkMxNCellMapWidget::leaveEvent(QEvent* event)
{
  if (m_HoverTile != -1 || m_HoverSlot != -1)
  {
    m_HoverTile = -1;
    m_HoverSlot = -1;
    emit GlyphHoverCleared();
    this->update();
  }
  QWidget::leaveEvent(event);
}

void QmitkMxNCellMapWidget::UpdateGlyphHover(const QPoint& position)
{
  // The tile barcode always carries the eight sync axes (the seven dimensions
  // plus data selection); hit-test against the same rect and slot count the
  // paint uses.
  static constexpr int SyncAxisCount = static_cast<int>(QmitkMxNAllSyncDimensions.size()) + 1;

  int tile = this->TileAt(position);
  int slot = -1;
  if (tile >= 0)
  {
    const QRect barcodeRect = this->TileBarcodeRect(m_Tiles[static_cast<std::size_t>(tile)]);
    slot = QmitkMxNSyncBarcodeWidget::SlotAtIn(barcodeRect, SyncAxisCount, position);
    if (slot < 0)
    {
      tile = -1;  // over a tile but not its barcode: nothing to highlight
    }
  }

  if (tile == m_HoverTile && slot == m_HoverSlot)
  {
    return;
  }
  m_HoverTile = tile;
  m_HoverSlot = slot;

  if (tile >= 0 && slot >= 0)
  {
    emit GlyphHovered(m_Tiles[static_cast<std::size_t>(tile)].windowId, slot);
  }
  else
  {
    emit GlyphHoverCleared();
  }
  this->update();
}

void QmitkMxNCellMapWidget::DiscardHoverHighlight()
{
  m_HoverTile = -1;
  m_HoverSlot = -1;
  if (!m_HighlightCells.isEmpty() || m_HighlightAxis != -1)
  {
    m_HighlightCells.clear();
    m_HighlightAxis = -1;
    this->update();
  }
}

void QmitkMxNCellMapWidget::StartCellDrag()
{
  if (m_Selection.isEmpty())
  {
    return;
  }

  // A drag and the hover highlight must not co-paint with the drop-target
  // treatment; drop out of the highlight before the drag begins.
  this->DiscardHoverHighlight();

  auto* mimeData = new QMimeData();
  mimeData->setData(CellsMimeType, m_Selection.join(QStringLiteral("\n")).toUtf8());
  if (m_DragAsksMode)
  {
    mimeData->setData(AskModeMimeType, QByteArray());
  }

  auto* drag = new QDrag(this);
  drag->setMimeData(mimeData);
  drag->exec(Qt::CopyAction);
  m_DragAsksMode = false;
}

void QmitkMxNCellMapWidget::dragEnterEvent(QDragEnterEvent* event)
{
  if (event->mimeData()->hasFormat(GroupMimeType))
  {
    // An incoming group drag replaces the hover highlight with the drop-target
    // treatment; drop the highlight so the two never co-paint.
    this->DiscardHoverHighlight();
    event->acceptProposedAction();
  }
}

void QmitkMxNCellMapWidget::dragMoveEvent(QDragMoveEvent* event)
{
  if (!event->mimeData()->hasFormat(GroupMimeType))
  {
    return;
  }
  // Highlight the tile the group would drop onto so the target is obvious.
  const int tile = this->TileAt(event->position().toPoint());
  if (tile != m_DropTargetTile)
  {
    m_DropTargetTile = tile;
    this->update();
  }
  event->acceptProposedAction();
}

void QmitkMxNCellMapWidget::dragLeaveEvent(QDragLeaveEvent* /*event*/)
{
  if (m_DropTargetTile != -1)
  {
    m_DropTargetTile = -1;
    this->update();
  }
}

void QmitkMxNCellMapWidget::dropEvent(QDropEvent* event)
{
  m_DropTargetTile = -1;
  this->update();

  if (!event->mimeData()->hasFormat(GroupMimeType))
  {
    return;
  }

  const int index = this->TileAt(event->position().toPoint());
  if (index < 0)
  {
    return;
  }

  const auto mode = ResolveJoinMode(event->mimeData(), event->modifiers(), this,
                                    this->mapToGlobal(event->position().toPoint()));
  if (!mode.has_value())
  {
    return;
  }

  const auto group = QString::fromUtf8(event->mimeData()->data(GroupMimeType));
  const auto& windowId = m_Tiles[static_cast<std::size_t>(index)].windowId;

  // Dropping onto a selected tile targets the whole selection; onto an
  // unselected tile just that cell.
  const QStringList targets = m_Selection.contains(windowId) ? m_Selection
                                                             : QStringList{ windowId };
  emit AssignRequested(group, targets, *mode);
  event->acceptProposedAction();
}

QmitkMxNGroupJoinMode QmitkMxNCellMapWidget::JoinModeFromModifiers(Qt::KeyboardModifiers modifiers)
{
  // Alt merges (overwriting collisions), Shift fills only empty axes; a plain
  // drop replaces. Ctrl is deliberately not used - the map already binds it to
  // multi-select, so it must keep its selection meaning during a drag.
  if (modifiers.testFlag(Qt::AltModifier))
  {
    return QmitkMxNGroupJoinMode::MergeOverwriteCollisions;
  }
  if (modifiers.testFlag(Qt::ShiftModifier))
  {
    return QmitkMxNGroupJoinMode::FillEmpty;
  }
  return QmitkMxNGroupJoinMode::Replace;
}

std::vector<QmitkMxNCellMapWidget::JoinModeEntry> QmitkMxNCellMapWidget::JoinModeMenuEntries()
{
  return { { QmitkMxNGroupJoinMode::Replace, tr("Replace the cells' synchronization") },
           { QmitkMxNGroupJoinMode::MergeOverwriteCollisions, tr("Merge, overwriting collisions") },
           { QmitkMxNGroupJoinMode::FillEmpty, tr("Fill only unsynchronized axes") } };
}

std::optional<QmitkMxNGroupJoinMode> QmitkMxNCellMapWidget::ResolveJoinMode(
  const QMimeData* mimeData, Qt::KeyboardModifiers modifiers, QWidget* parent,
  const QPoint& globalPosition)
{
  if (nullptr == mimeData || !mimeData->hasFormat(AskModeMimeType))
  {
    return JoinModeFromModifiers(modifiers);
  }

  QMenu menu(parent);
  std::vector<QAction*> actions;
  const auto entries = JoinModeMenuEntries();
  actions.reserve(entries.size());
  for (const auto& entry : entries)
  {
    actions.push_back(menu.addAction(entry.label));
  }
  menu.addSeparator();
  menu.addAction(tr("Cancel"));

  const QAction* chosen = menu.exec(globalPosition);
  for (std::size_t i = 0; i < actions.size(); ++i)
  {
    if (chosen == actions[i])
    {
      return entries[i].mode;
    }
  }
  return std::nullopt;
}

QStringList QmitkMxNCellMapWidget::TilesBetween(const QString& anchor, const QString& target) const
{
  const auto tileOf = [this](const QString& windowId) -> const Tile* {
    const auto it = std::find_if(m_Tiles.begin(), m_Tiles.end(),
                                 [&windowId](const Tile& tile) { return tile.windowId == windowId; });
    return it != m_Tiles.end() ? &*it : nullptr;
  };

  const Tile* anchorTile = tileOf(anchor);
  const Tile* targetTile = tileOf(target);
  if (nullptr == targetTile)
  {
    return QStringList();
  }
  if (nullptr == anchorTile || !anchorTile->mapRect.isValid() || !targetTile->mapRect.isValid())
  {
    return QStringList{ target };
  }

  const QRect span = anchorTile->mapRect.united(targetTile->mapRect);
  QStringList range;
  for (const auto& tile : m_Tiles)
  {
    if (tile.mapRect.isValid() && span.intersects(tile.mapRect))
    {
      range.append(tile.windowId);
    }
  }
  return range;
}
