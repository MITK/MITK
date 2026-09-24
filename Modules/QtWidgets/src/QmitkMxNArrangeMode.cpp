/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNArrangeMode.h"

#include <QmitkMxNMultiWidget.h>

#include <QMimeData>

QmitkMxNArrangeMode::QmitkMxNArrangeMode(QmitkMxNMultiWidget* multiWidget)
  : QObject(multiWidget),
    m_MultiWidget(multiWidget)
{
}

void QmitkMxNArrangeMode::SetActive(bool active)
{
  if (active == m_Active)
  {
    return;
  }
  m_Active = active;
  emit ActiveChanged(m_Active);
}

bool QmitkMxNArrangeMode::IsActive() const
{
  return m_Active;
}

QStringList QmitkMxNArrangeMode::GetSelectedWindowIds() const
{
  return m_Selection;
}

void QmitkMxNArrangeMode::SetSelectedWindowIds(const QStringList& windowIds)
{
  this->SetSelection(windowIds);
}

bool QmitkMxNArrangeMode::PressCell(const QString& windowId, Qt::MouseButton button,
                                    Qt::KeyboardModifiers modifiers)
{
  m_PressedWindowId.clear();
  m_DragAsksMode = false;

  if (Qt::RightButton == button)
  {
    // The drag carries what the user pointed at: an unselected cell replaces
    // the selection, a selected one keeps all of it.
    if (!m_Selection.contains(windowId))
    {
      this->SetSelection(QStringList{ windowId });
      m_Anchor = windowId;
    }
    m_DragAsksMode = true;
    return true;
  }
  if (Qt::LeftButton != button)
  {
    return false;
  }

  if (modifiers.testFlag(Qt::ShiftModifier))
  {
    // The anchor stays put so the range can be re-spanned from the same origin.
    const QStringList range = this->CellsBetween(m_Anchor, windowId);
    if (modifiers.testFlag(Qt::ControlModifier))
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
    if (m_Anchor.isEmpty())
    {
      m_Anchor = windowId;
    }
    return false;
  }

  if (modifiers.testFlag(Qt::ControlModifier))
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
    m_Anchor = windowId;
    return false;
  }

  // Pressing a selected cell holds the selection so a drag can carry all of it;
  // without the collapse on release a full selection would have no way back to
  // a single cell.
  if (!m_Selection.contains(windowId))
  {
    this->SetSelection(QStringList{ windowId });
  }
  m_PressedWindowId = windowId;
  m_Anchor = windowId;
  return true;
}

void QmitkMxNArrangeMode::ReleaseCell(bool dragged)
{
  if (!dragged && !m_PressedWindowId.isEmpty())
  {
    this->SetSelection(QStringList{ m_PressedWindowId });
  }
  m_PressedWindowId.clear();
  m_DragAsksMode = false;
}

QMimeData* QmitkMxNArrangeMode::CreateDragMimeData() const
{
  return QmitkMxNCreateCellsMimeData(m_Selection, m_DragAsksMode);
}

void QmitkMxNArrangeMode::PruneToExistingCells()
{
  if (m_MultiWidget.isNull())
  {
    return;
  }
  const auto exists = [this](const QString& windowId)
  {
    return nullptr != m_MultiWidget->GetRenderWindowWidget(windowId);
  };

  if (!m_Anchor.isEmpty() && !exists(m_Anchor))
  {
    m_Anchor.clear();
  }
  if (!m_PressedWindowId.isEmpty() && !exists(m_PressedWindowId))
  {
    m_PressedWindowId.clear();
  }
  QStringList kept;
  for (const auto& windowId : m_Selection)
  {
    if (exists(windowId))
    {
      kept.append(windowId);
    }
  }
  this->SetSelection(kept);
}

void QmitkMxNArrangeMode::RequestAssign(const QString& group, const QString& targetWindowId,
                                        QmitkMxNGroupJoinMode mode)
{
  const QStringList targets = m_Selection.contains(targetWindowId) ? m_Selection
                                                                  : QStringList{ targetWindowId };
  emit AssignRequested(group, targets, mode);
}

void QmitkMxNArrangeMode::SetHighlight(HighlightSource source, std::optional<QmitkMxNSyncAxis> axis,
                                       const QStringList& windowIds, const QColor& hue)
{
  if (m_HighlightSource == source && m_HighlightAxis == axis
      && m_HighlightedWindowIds == windowIds && m_HighlightHue == hue)
  {
    return;
  }
  m_HighlightSource = source;
  m_HighlightAxis = axis;
  m_HighlightedWindowIds = windowIds;
  m_HighlightHue = hue;
  emit HighlightChanged();
}

void QmitkMxNArrangeMode::ClearHighlight(HighlightSource source)
{
  if (m_HighlightSource != source)
  {
    return;
  }
  m_HighlightSource.reset();
  m_HighlightAxis.reset();
  m_HighlightedWindowIds.clear();
  m_HighlightHue = QColor();
  emit HighlightChanged();
}

std::optional<QmitkMxNSyncAxis> QmitkMxNArrangeMode::GetHighlightAxis() const
{
  return m_HighlightAxis;
}

QStringList QmitkMxNArrangeMode::GetHighlightedWindowIds() const
{
  return m_HighlightedWindowIds;
}

QColor QmitkMxNArrangeMode::GetHighlightHue() const
{
  return m_HighlightHue;
}

void QmitkMxNArrangeMode::SetSelection(const QStringList& windowIds)
{
  if (windowIds == m_Selection)
  {
    return;
  }
  m_Selection = windowIds;
  emit SelectionChanged(m_Selection);
}

QStringList QmitkMxNArrangeMode::CellsBetween(const QString& anchor, const QString& target) const
{
  if (m_MultiWidget.isNull())
  {
    return QStringList{ target };
  }

  const auto rects = m_MultiWidget->GetNormalizedCellRects();
  const auto rectOf = [&rects](const QString& windowId) -> std::optional<QRectF>
  {
    for (const auto& [id, rect] : rects)
    {
      if (id == windowId)
      {
        return rect;
      }
    }
    return std::nullopt;
  };

  const auto targetRect = rectOf(target);
  if (!targetRect.has_value())
  {
    return QStringList();
  }
  const auto anchorRect = rectOf(anchor);
  if (!anchorRect.has_value())
  {
    return QStringList{ target };
  }

  // Adjacent cells share an edge, which QRectF does not count as intersecting,
  // so the span picks up exactly the cells it overlaps.
  const QRectF span = anchorRect->united(*targetRect);
  QStringList range;
  for (const auto& [id, rect] : rects)
  {
    if (span.intersects(rect))
    {
      range.append(id);
    }
  }
  return range;
}
