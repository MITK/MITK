/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNCellMapWidget.h"

#include <QmitkMxNMultiWidget.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkException.h>

#include <QApplication>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QRubberBand>

#include <algorithm>
#include <array>
#include <optional>

const char* QmitkMxNCellMapWidget::CellsMimeType = "application/x-mitk-mxn-cells";
const char* QmitkMxNCellMapWidget::GroupMimeType = "application/x-mitk-mxn-group";

namespace
{
  constexpr int BarcodeHeight = 7;
  constexpr int TileSpacing = 2;

  const std::array<QmitkMxNSyncDimension, 4> NavigationBundle{
    QmitkMxNSyncDimension::Pan, QmitkMxNSyncDimension::Zoom,
    QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Crosshair
  };

  QString BareCellLabel(const QmitkMxNMultiWidget::WindowDescriptor& descriptor)
  {
    if (!descriptor.displayName.isEmpty())
    {
      return descriptor.displayName;
    }
    const auto separator = descriptor.id.indexOf(QStringLiteral("__"));
    return separator >= 0 ? descriptor.id.mid(separator + 2) : descriptor.id;
  }

  bool OffsetIsIdentity(const QmitkMxNMultiWidget::SyncOffset& offset)
  {
    if (std::holds_alternative<int>(offset))
    {
      return std::get<int>(offset) == 0;
    }
    if (std::holds_alternative<double>(offset))
    {
      return std::get<double>(offset) == 1.0;
    }
    if (std::holds_alternative<mitk::Vector2D>(offset))
    {
      const auto& pan = std::get<mitk::Vector2D>(offset);
      return pan[0] == 0.0 && pan[1] == 0.0;
    }
    return true;
  }
}

QmitkMxNCellMapWidget::QmitkMxNCellMapWidget(QWidget* parent)
  : QWidget(parent)
  , m_RubberBand(new QRubberBand(QRubberBand::Rectangle, this))
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

    const QRect editorRect = m_MultiWidget->rect();
    for (const auto& descriptor : descriptors)
    {
      const auto cell = m_MultiWidget->GetRenderWindowWidget(descriptor.id);
      if (nullptr == cell)
      {
        continue;
      }

      Tile tile;
      tile.windowId = descriptor.id;
      tile.label = BareCellLabel(descriptor);

      // Mirror the real on-screen proportions so the map reads as "my
      // layout, shrunk", whatever the splitter nesting looks like.
      const QRect cellRect(cell->mapTo(m_MultiWidget, QPoint(0, 0)), cell->size());
      if (editorRect.width() > 0 && editorRect.height() > 0)
      {
        tile.normalizedRect = QRectF(
          static_cast<qreal>(cellRect.x()) / editorRect.width(),
          static_cast<qreal>(cellRect.y()) / editorRect.height(),
          static_cast<qreal>(cellRect.width()) / editorRect.width(),
          static_cast<qreal>(cellRect.height()) / editorRect.height());
      }
      m_Tiles.push_back(std::move(tile));
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

  for (const auto& tile : m_Tiles)
  {
    if (!tile.mapRect.isValid())
    {
      continue;
    }

    // Navigation consensus: one hue when all linked navigation dimensions
    // agree on a group, a neutral "hybrid" fill when they disagree. The
    // barcode below always tells the exact story.
    std::optional<std::string> navGroup;
    bool navLinked = false;
    bool hybrid = false;
    for (const auto dimension : NavigationBundle)
    {
      const auto link = m_MultiWidget->GetSyncLink(tile.windowId, dimension);
      if (!link.has_value())
      {
        continue;
      }
      if (navLinked && link->group != *navGroup)
      {
        hybrid = true;
      }
      navGroup = link->group;
      navLinked = true;
    }

    QColor fill = this->palette().color(QPalette::Base);
    QString groupLabel;
    if (navLinked && !hybrid)
    {
      try
      {
        fill = m_MultiWidget->GetSyncGroupColor(*navGroup);
        fill.setAlpha(110);
        groupLabel = QString::fromStdString(m_MultiWidget->GetSyncGroupDisplayName(*navGroup));
      }
      catch (const mitk::Exception&)
      {
      }
    }
    else if (hybrid)
    {
      fill = this->palette().color(QPalette::AlternateBase);
      groupLabel = tr("(mixed)");
    }

    painter.fillRect(tile.mapRect, fill);

    const bool selected = m_Selection.contains(tile.windowId);
    painter.setPen(QPen(selected ? this->palette().color(QPalette::Highlight)
                                 : this->palette().color(QPalette::Mid),
                        selected ? 2 : 1));
    painter.drawRect(tile.mapRect.adjusted(0, 0, -1, -1));

    // Labels: cell name, group name beneath.
    painter.setPen(this->palette().color(QPalette::Text));
    const QRect textRect = tile.mapRect.adjusted(3, 2, -3, -BarcodeHeight - 3);
    painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap,
                     groupLabel.isEmpty() ? tile.label
                                          : tile.label + QStringLiteral("\n") + groupLabel);

    // Sync barcode: fixed dimension order, hue = linked group, gap =
    // unsynced, notch = link carries an offset.
    const int slotCount = static_cast<int>(QmitkMxNAllSyncDimensions.size());
    const QRect barcodeRect(tile.mapRect.left() + 2, tile.mapRect.bottom() - BarcodeHeight - 1,
                            tile.mapRect.width() - 4, BarcodeHeight);
    const double slotWidth = static_cast<double>(barcodeRect.width()) / slotCount;
    for (int slot = 0; slot < slotCount; ++slot)
    {
      const auto dimension = QmitkMxNAllSyncDimensions[static_cast<std::size_t>(slot)];
      const auto link = m_MultiWidget->GetSyncLink(tile.windowId, dimension);
      if (!link.has_value())
      {
        continue;
      }

      QColor slotColor = this->palette().color(QPalette::Mid);
      try
      {
        slotColor = m_MultiWidget->GetSyncGroupColor(link->group);
      }
      catch (const mitk::Exception&)
      {
      }

      const QRect slotRect(barcodeRect.left() + qRound(slot * slotWidth), barcodeRect.top(),
                           std::max(1, qRound(slotWidth) - 1), barcodeRect.height());
      painter.fillRect(slotRect, slotColor);

      if (!OffsetIsIdentity(link->offset))
      {
        painter.fillRect(QRect(slotRect.center().x(), slotRect.top(), 2, 2),
                         this->palette().color(QPalette::BrightText));
      }
    }
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
    // Empty area: begin a rubber-band selection.
    m_RubberBandActive = true;
    m_DragCandidate = false;
    m_RubberBand->setGeometry(QRect(m_PressPosition, QSize()));
    m_RubberBand->show();
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
  if (m_RubberBandActive)
  {
    m_RubberBand->setGeometry(QRect(m_PressPosition, event->pos()).normalized());
    event->accept();
    return;
  }

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
  if (m_RubberBandActive)
  {
    m_RubberBandActive = false;
    m_RubberBand->hide();

    const QRect band = QRect(m_PressPosition, event->pos()).normalized();
    QStringList selection = event->modifiers().testFlag(Qt::ControlModifier) ? m_Selection
                                                                             : QStringList();
    for (const auto& tile : m_Tiles)
    {
      if (tile.mapRect.intersects(band) && !selection.contains(tile.windowId))
      {
        selection.append(tile.windowId);
      }
    }
    this->SetSelection(selection);
    event->accept();
    return;
  }

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

void QmitkMxNCellMapWidget::dropEvent(QDropEvent* event)
{
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
  emit AssignRequested(group, targets);
  event->acceptProposedAction();
}
