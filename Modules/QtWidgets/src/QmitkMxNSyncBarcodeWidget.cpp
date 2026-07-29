/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNSyncBarcodeWidget.h"

#include <QApplication>
#include <QHelpEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QToolTip>

#include <algorithm>

namespace
{
  constexpr int ColorSlotWidth = 7;   // compact, positional color-slot look
  constexpr int BarcodeHeight = 16;   // color-slot mode + minimum size hint
  constexpr int SlotGap = 1;
  constexpr int IconGap = 2;          // gap between framed glyph boxes
  constexpr int IconInset = 3;        // glyph padding inside its framed box

  // Furniture neutral: a faint white, matching the hairline tokens used across
  // the MxN viewport furniture over its translucent dark backing.
  const QColor HintColor(255, 255, 255, 110);
}

QmitkMxNSyncBarcodeWidget::QmitkMxNSyncBarcodeWidget(QWidget* parent)
  : QWidget(parent)
{
  // Mouse tracking so hover (and the pointing cursor) can be gated to the glyph
  // area even without a button pressed.
  this->setMouseTracking(true);
  this->setToolTip(tr("Open the layout editor"));
}

QmitkMxNSyncBarcodeWidget::~QmitkMxNSyncBarcodeWidget()
{
}

void QmitkMxNSyncBarcodeWidget::SetSlots(const QList<AxisSlot>& axisSlots)
{
  m_Slots = axisSlots;
  this->updateGeometry();
  this->update();
}

QList<QmitkMxNSyncBarcodeWidget::AxisSlot> QmitkMxNSyncBarcodeWidget::Slots() const
{
  return m_Slots;
}

bool QmitkMxNSyncBarcodeWidget::IsEmptyState() const
{
  return std::none_of(m_Slots.begin(), m_Slots.end(),
                      [](const AxisSlot& slot) { return slot.color.isValid(); });
}

QSize QmitkMxNSyncBarcodeWidget::sizeHint() const
{
  // The compact color-slot size is the minimum the layout must grant; extra
  // width lets the paint switch to the wider icon rendering.
  const int slotCount = std::max(1, static_cast<int>(m_Slots.size()));
  return QSize(slotCount * ColorSlotWidth + (slotCount - 1) * SlotGap, BarcodeHeight);
}

QSize QmitkMxNSyncBarcodeWidget::minimumSizeHint() const
{
  return this->sizeHint();
}

bool QmitkMxNSyncBarcodeWidget::UseIconMode() const
{
  if (m_Slots.isEmpty())
  {
    return false;
  }
  // Draw glyphs only when the granted width fits them; otherwise collapse to
  // the compact color slots.
  const int box = this->IconBox();
  const int needed = m_Slots.size() * box + (m_Slots.size() - 1) * IconGap;
  return this->width() >= needed;
}

int QmitkMxNSyncBarcodeWidget::IconBox() const
{
  // Fill the strip height (minus a hair of padding) so the glyphs are legible
  // and not lost in whitespace, clamped to a sane square range.
  return std::clamp(this->height() - 2, 14, 24);
}

int QmitkMxNSyncBarcodeWidget::ContentWidth() const
{
  const int n = m_Slots.size();
  if (n == 0)
  {
    return 0;
  }
  if (this->UseIconMode())
  {
    const int box = this->IconBox();
    return 1 + n * box + (n - 1) * IconGap;  // 1 = the left inset used when painting
  }
  return n * ColorSlotWidth + (n - 1) * SlotGap;
}

int QmitkMxNSyncBarcodeWidget::SlotAtX(int x) const
{
  if (m_Slots.isEmpty())
  {
    return -1;
  }
  const bool icon = this->UseIconMode();
  const int stride = (icon ? this->IconBox() : ColorSlotWidth) + (icon ? IconGap : SlotGap);
  const int slot = x / std::max(1, stride);
  return (slot >= 0 && slot < m_Slots.size()) ? slot : -1;
}

void QmitkMxNSyncBarcodeWidget::paintEvent(QPaintEvent* /*event*/)
{
  QPainter painter(this);

  if (this->IsEmptyState())
  {
    painter.setPen(HintColor);
    QFont font = painter.font();
    font.setPointSizeF(std::max(7.0, font.pointSizeF() - 1.0));
    painter.setFont(font);
    painter.drawText(this->rect(), Qt::AlignVCenter | Qt::AlignLeft, tr("not synchronized"));
    return;
  }

  const QColor gap = this->palette().color(QPalette::Mid);

  if (this->UseIconMode())
  {
    // A framed glyph per axis (the seam idiom): a 1 px box in the axis color
    // with the glyph inside. Hover brightens every frame to white so the whole
    // strip reads as one button that opens the layout editor. A 1 px left inset
    // keeps the leftmost frame off the widget edge (a pixel-aligned pen there
    // would otherwise be clipped).
    const int box = this->IconBox();
    const int inner = std::max(1, box - 2 * IconInset);
    const int top = (this->height() - box) / 2;
    const qreal dpr = this->devicePixelRatioF();

    for (int slot = 0; slot < m_Slots.size(); ++slot)
    {
      const AxisSlot& s = m_Slots[slot];
      const bool synced = s.color.isValid();
      const QColor color = synced ? s.color : gap;
      const QRect boxRect(1 + slot * (box + IconGap), top, box, box);

      const QColor frameColor = m_Hovered ? QColor(255, 255, 255, 210) : color;
      painter.setOpacity(synced || m_Hovered ? 1.0 : 0.5);
      painter.setPen(QPen(frameColor, 1));
      painter.setBrush(Qt::NoBrush);
      painter.drawRect(boxRect.adjusted(0, 0, -1, -1));

      painter.setOpacity(synced ? 1.0 : (m_Hovered ? 0.8 : 0.4));
      QPixmap glyph = QmitkMxNRenderAxisGlyph(s.glyph, color, qRound(inner * dpr));
      if (!glyph.isNull())
      {
        glyph.setDevicePixelRatio(dpr);
        painter.drawPixmap(QPoint(boxRect.center().x() - inner / 2, boxRect.center().y() - inner / 2), glyph);
      }
      painter.setOpacity(1.0);
    }
    return;
  }

  // Narrow: the compact positional color slots.
  const int top = (this->height() - BarcodeHeight) / 2;
  for (int slot = 0; slot < m_Slots.size(); ++slot)
  {
    const QRect slotRect(slot * (ColorSlotWidth + SlotGap), top, ColorSlotWidth, BarcodeHeight);
    const AxisSlot& s = m_Slots[slot];
    if (s.color.isValid())
    {
      painter.fillRect(slotRect, s.color);
    }
    else
    {
      // Unsynced: a faint hairline outline, no fill, so gaps read as absence.
      painter.setPen(QPen(gap, 1));
      painter.drawLine(slotRect.left(), slotRect.bottom(), slotRect.right(), slotRect.bottom());
    }
  }
}

void QmitkMxNSyncBarcodeWidget::mouseMoveEvent(QMouseEvent* event)
{
  // Only the glyph area is interactive; the strip can be laid out wider than its
  // glyphs, and that trailing space must not highlight or read as clickable.
  const bool over = event->pos().x() >= 0 && event->pos().x() < this->ContentWidth();
  if (over != m_Hovered)
  {
    m_Hovered = over;
    this->update();
  }
  this->setCursor(over ? Qt::PointingHandCursor : Qt::ArrowCursor);
  QWidget::mouseMoveEvent(event);
}

void QmitkMxNSyncBarcodeWidget::leaveEvent(QEvent* event)
{
  if (m_Hovered)
  {
    m_Hovered = false;
    this->update();
  }
  QWidget::leaveEvent(event);
}

void QmitkMxNSyncBarcodeWidget::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
  {
    m_PressPos = event->pos();
  }
  QWidget::mousePressEvent(event);
}

void QmitkMxNSyncBarcodeWidget::mouseReleaseEvent(QMouseEvent* event)
{
  // A click (not a drag) on the glyph area opens the editor; the trailing space
  // is inert.
  if (event->button() == Qt::LeftButton
      && (event->pos() - m_PressPos).manhattanLength() < QApplication::startDragDistance()
      && event->pos().x() < this->ContentWidth())
  {
    emit Clicked();
  }
  QWidget::mouseReleaseEvent(event);
}

bool QmitkMxNSyncBarcodeWidget::event(QEvent* event)
{
  if (event->type() == QEvent::ToolTip)
  {
    auto* helpEvent = static_cast<QHelpEvent*>(event);
    const int slot = this->SlotAtX(helpEvent->pos().x());
    if (slot >= 0 && !m_Slots[slot].tooltip.isEmpty())
    {
      QToolTip::showText(helpEvent->globalPos(), m_Slots[slot].tooltip, this);
      return true;
    }
  }
  return QWidget::event(event);
}
