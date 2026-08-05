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
#include <QmitkRenderWindowWidget.h>

#include <mitkException.h>

#include <QApplication>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>

#include <algorithm>

const char* QmitkMxNCellMapWidget::CellsMimeType = "application/x-mitk-mxn-cells";
const char* QmitkMxNCellMapWidget::GroupMimeType = "application/x-mitk-mxn-group";

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

    // A barcode band in the tile's lower portion, tall enough to wrap glyphs
    // when the tile has the room and collapsing to color slots when it does not.
    const int barcodeBand = std::clamp(tile.mapRect.height() / 2, 8, 56);

    // The cell label, above the barcode band.
    painter.setPen(this->palette().color(QPalette::Text));
    const QRect textRect = tile.mapRect.adjusted(3, 2, -3, -barcodeBand - 3);
    painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap, tile.label);

    // Sync barcode via the shared renderer, so the tile tells the same story as
    // the per-cell strip: eight axes (the seven dimensions plus data selection)
    // as wrapping glyphs, or color slots when the band is too small. Built from
    // this cell's window perspective (each axis linked or not).
    const QRect barcodeRect(tile.mapRect.left() + 2, tile.mapRect.bottom() - barcodeBand - 1,
                            tile.mapRect.width() - 4, barcodeBand);
    QmitkMxNSyncBarcodeWidget::PaintInto(painter, barcodeRect,
                                         m_MultiWidget->BuildBarcodeSlots(tile.windowId),
                                         false, this->palette().color(QPalette::Mid));
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
  if (event->button() != Qt::LeftButton)
  {
    event->ignore();
    return;
  }

  m_PressPosition = event->pos();
  const int index = this->TileAt(event->pos());

  if (index < 0)
  {
    // Empty area: clear the selection. Multi-select is by Ctrl-click.
    m_DragCandidate = false;
    this->SetSelection(QStringList());
    event->accept();
    return;
  }

  const auto& windowId = m_Tiles[static_cast<std::size_t>(index)].windowId;
  if (event->modifiers().testFlag(Qt::ControlModifier))
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
    m_DragCandidate = false;
  }
  else
  {
    // Pressing an already-selected tile keeps the selection so it can be
    // dragged as a whole; pressing an unselected tile selects it.
    if (!m_Selection.contains(windowId))
    {
      this->SetSelection(QStringList{ windowId });
    }
    m_DragCandidate = true;
  }

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

  event->ignore();
}

void QmitkMxNCellMapWidget::mouseReleaseEvent(QMouseEvent* event)
{
  m_DragCandidate = false;
  event->ignore();
}

void QmitkMxNCellMapWidget::StartCellDrag()
{
  if (m_Selection.isEmpty())
  {
    return;
  }

  auto* mimeData = new QMimeData();
  mimeData->setData(CellsMimeType, m_Selection.join(QStringLiteral("\n")).toUtf8());

  auto* drag = new QDrag(this);
  drag->setMimeData(mimeData);
  drag->exec(Qt::CopyAction);
}

void QmitkMxNCellMapWidget::dragEnterEvent(QDragEnterEvent* event)
{
  if (event->mimeData()->hasFormat(GroupMimeType))
  {
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

  const auto group = QString::fromUtf8(event->mimeData()->data(GroupMimeType));
  const auto& windowId = m_Tiles[static_cast<std::size_t>(index)].windowId;

  // Dropping onto a selected tile targets the whole selection; onto an
  // unselected tile just that cell.
  const QStringList targets = m_Selection.contains(windowId) ? m_Selection
                                                             : QStringList{ windowId };
  emit AssignRequested(group, targets, JoinModeFromModifiers(event->modifiers()));
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
