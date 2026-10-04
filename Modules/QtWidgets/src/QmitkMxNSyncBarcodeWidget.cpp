/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNSyncBarcodeWidget.h"

#include <QApplication>
#include <QFontMetrics>
#include <QHelpEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
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
  // A box's frame is part of the box, so it needs room of its own: without it
  // the grid grows until the frame is stroked along the host's very edge, and
  // whether that line survives is then at the mercy of the backing store, the
  // device pixel ratio and whatever paints behind the strip. One pixel top and
  // bottom keeps every edge of every box inside the canvas.
  constexpr int FrameRoom = 1;
  constexpr int MinGlyphBox = 12;     // smallest box that still reads as a glyph
  // The box the width hint asks for, tied to the host's text so the strip
  // follows font scaling. Only a request: the paint takes whatever height the
  // host actually grants, which is how the strip fills its chrome rather than
  // sitting inside it with slack above and below.
  constexpr int HintGlyphBox = 24;
}

QmitkMxNSyncBarcodeWidget::QmitkMxNSyncBarcodeWidget(QWidget* parent)
  : QWidget(parent)
{
  // Mouse tracking so hover (and the pointing cursor) can be gated to the glyph
  // area even without a button pressed.
  this->setMouseTracking(true);
  this->setToolTip(tr("Show or hide the layout editor"));
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

void QmitkMxNSyncBarcodeWidget::SetAxisClickable(bool clickable)
{
  m_AxisClickable = clickable;
}

void QmitkMxNSyncBarcodeWidget::SetPreferGlyphWidth(bool prefer)
{
  if (prefer != m_PreferGlyphWidth)
  {
    m_PreferGlyphWidth = prefer;
    this->updateGeometry();
  }
}

QSize QmitkMxNSyncBarcodeWidget::sizeHint() const
{
  const int slotCount = std::max(1, static_cast<int>(m_Slots.size()));
  if (m_PreferGlyphWidth)
  {
    // What the glyph rendering needs: one row of square boxes. The box is
    // the height the host has actually granted, not the text's - a width asked
    // for font-sized boxes caps the glyphs at that size however tall the row
    // is, which is why the strip's glyphs stayed smaller than its chrome. The
    // height hint stays the color bar's: the host row sets the height, and
    // asking for more here would only make the row taller.
    const QFontMetrics metrics(this->font());
    const int box =
      std::max(std::clamp(metrics.height(), MinGlyphBox, HintGlyphBox), this->height());
    const int glyphWidth = 1 + slotCount * box + (slotCount - 1) * IconGap;
    return QSize(glyphWidth, BarcodeHeight);
  }
  // The compact color-slot size is the minimum the layout must grant; extra
  // width lets the paint switch to the wider icon rendering.
  return QSize(slotCount * ColorSlotWidth + (slotCount - 1) * SlotGap, BarcodeHeight);
}

QSize QmitkMxNSyncBarcodeWidget::minimumSizeHint() const
{
  // Always the compact color bar: a strip that prefers the glyph width must
  // still be allowed to collapse to slots when its host runs out of room.
  const int slotCount = std::max(1, static_cast<int>(m_Slots.size()));
  return QSize(slotCount * ColorSlotWidth + (slotCount - 1) * SlotGap, BarcodeHeight);
}

QmitkMxNSyncBarcodeWidget::BarcodeLayout
QmitkMxNSyncBarcodeWidget::ComputeLayout(int width, int height, int slotCount)
{
  BarcodeLayout layout;
  if (slotCount <= 0)
  {
    return layout;  // ColorBar, but nothing to draw
  }

  // The box takes the height on offer, bounded by the width the row has to
  // share; below the legible size the strip collapses to the color slots.
  const int boxByWidth = (width - 1 - (slotCount - 1) * IconGap) / slotCount;  // 1 = left inset
  const int boxByHeight = height - 2 * FrameRoom;
  const int box = std::min(boxByWidth, boxByHeight);
  if (box >= MinGlyphBox)
  {
    layout.mode = BarcodeLayout::Mode::Glyphs;
    layout.box = box;
  }
  return layout;
}

QRect QmitkMxNSyncBarcodeWidget::ContentRectIn(const QRect& target, const BarcodeLayout& layout,
                                               int slotCount)
{
  if (slotCount == 0)
  {
    return QRect();
  }
  if (layout.mode == BarcodeLayout::Mode::Glyphs)
  {
    const int rowWidth = slotCount * layout.box + (slotCount - 1) * IconGap;
    // Centered vertically in what is left once the frames have their room;
    // left-aligned (with the 1 px inset) horizontally so the strip reads
    // left-to-right and any trailing space stays inert. Derived from the same
    // budget ComputeLayout sized the box against, so the paint and the hit-test
    // cannot disagree about where a glyph is.
    const int top = target.top() + FrameRoom
                    + std::max(0, (target.height() - 2 * FrameRoom - layout.box) / 2);
    return QRect(target.left() + 1, top, rowWidth, layout.box);
  }
  const int barWidth = slotCount * ColorSlotWidth + (slotCount - 1) * SlotGap;
  const int top = target.top() + (target.height() - BarcodeHeight) / 2;
  return QRect(target.left(), top, barWidth, BarcodeHeight);
}

QRect QmitkMxNSyncBarcodeWidget::GlyphBoxRectIn(const QRect& contentRect, const BarcodeLayout& layout,
                                                int index)
{
  return QRect(contentRect.left() + index * (layout.box + IconGap), contentRect.top(), layout.box, layout.box);
}

QRect QmitkMxNSyncBarcodeWidget::ContentRect(const BarcodeLayout& layout) const
{
  return ContentRectIn(this->rect(), layout, m_Slots.size());
}

int QmitkMxNSyncBarcodeWidget::SlotAt(const QPoint& pos) const
{
  const int slotCount = static_cast<int>(m_Slots.size());
  if (slotCount <= 0)
  {
    return -1;
  }
  const BarcodeLayout layout = ComputeLayout(this->width(), this->height(), slotCount);
  const QRect content = this->ContentRect(layout);
  if (!content.contains(pos))
  {
    return -1;
  }
  if (layout.mode == BarcodeLayout::Mode::Glyphs)
  {
    for (int slot = 0; slot < slotCount; ++slot)
    {
      if (GlyphBoxRectIn(content, layout, slot).contains(pos))
      {
        return slot;
      }
    }
    return -1;
  }
  const int slot = (pos.x() - content.left()) / std::max(1, ColorSlotWidth + SlotGap);
  return (slot >= 0 && slot < slotCount) ? slot : -1;
}

void QmitkMxNSyncBarcodeWidget::PaintInto(QPainter& painter, const QRect& target,
                                          const QList<AxisSlot>& axisSlots, bool hovered,
                                          const QColor& gapColor, int hoveredSlot)
{
  const int n = axisSlots.size();
  if (n == 0)
  {
    return;
  }
  const BarcodeLayout layout = ComputeLayout(target.width(), target.height(), n);
  const QRect content = ContentRectIn(target, layout, n);

  if (layout.mode == BarcodeLayout::Mode::Glyphs)
  {
    // A framed glyph per axis (the seam idiom): a 1 px box in the axis color
    // with the glyph inside. A lit frame brightens to white: 'hovered' lights the
    // whole strip (the passive per-cell strip is one button), 'hoveredSlot'
    // lights a single axis (the editor targets axes individually).
    const int inner = std::max(1, layout.box - 2 * IconInset);
    const qreal dpr = nullptr != painter.device() ? painter.device()->devicePixelRatioF() : 1.0;

    // A box may sit flush against the target's edge - it does whenever the strip
    // fills the height of its chrome - so the frame is stroked half a pixel
    // inside the box rather than along it. A pen centred on the boundary puts
    // half its width outside the canvas, which costs the top line the moment
    // the grid grows to the full height, and costs it most when the frame is
    // lit and so drawn part-transparent. Antialiasing keeps that half-pixel
    // stroke crisp and the corner mark smooth; it is restored afterwards
    // because the painter belongs to the caller.
    const bool hadAntialiasing = painter.testRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::Antialiasing, true);

    for (int slot = 0; slot < n; ++slot)
    {
      const AxisSlot& s = axisSlots[slot];
      const bool synced = s.color.isValid();
      const bool lit = hovered || slot == hoveredSlot;
      const QColor color = synced ? s.color : gapColor;
      const QRect boxRect = GlyphBoxRectIn(content, layout, slot);

      const QColor frameColor = lit ? QColor(255, 255, 255, 210) : color;
      painter.setOpacity(synced || lit ? 1.0 : 0.5);
      // A dashed frame marks the heterogeneous group state (some members linked,
      // not all); solid means all members, gap means none.
      QPen framePen(frameColor, 1);
      if (s.partial)
      {
        framePen.setStyle(Qt::DashLine);
      }
      painter.setPen(framePen);
      painter.setBrush(Qt::NoBrush);
      painter.drawRect(QRectF(boxRect).adjusted(0.5, 0.5, -0.5, -0.5));

      painter.setOpacity(synced ? 1.0 : (lit ? 0.8 : 0.4));
      const QPixmap glyph = QmitkMxNRenderAxisGlyph(s.glyph, color, qRound(inner * dpr), dpr);
      if (!glyph.isNull())
      {
        painter.drawPixmap(QPoint(boxRect.center().x() - inner / 2, boxRect.center().y() - inner / 2), glyph);
      }

      // The offset footnote: the box's upper-right corner filled. At the sizes
      // the boxes now reach it carries further than an outline does, and a
      // glyph is still recognisable with one corner covered.
      if (s.hasOffset && synced)
      {
        painter.setOpacity(1.0);
        const QRectF frameRect = QRectF(boxRect).adjusted(0.5, 0.5, -0.5, -0.5);
        const qreal mark = std::max(4, layout.box / 3);
        QPainterPath corner;
        corner.moveTo(frameRect.right() - mark, frameRect.top());
        corner.lineTo(frameRect.right(), frameRect.top());
        corner.lineTo(frameRect.right(), frameRect.top() + mark);
        corner.closeSubpath();
        painter.setPen(Qt::NoPen);
        painter.setBrush(frameColor);
        painter.drawPath(corner);
        painter.setBrush(Qt::NoBrush);
      }
      painter.setOpacity(1.0);
    }
    painter.setRenderHint(QPainter::Antialiasing, hadAntialiasing);
    return;
  }

  // Narrow: the compact positional color slots.
  for (int slot = 0; slot < n; ++slot)
  {
    const QRect slotRect(content.left() + slot * (ColorSlotWidth + SlotGap), content.top(),
                         ColorSlotWidth, BarcodeHeight);
    const AxisSlot& s = axisSlots[slot];
    if (s.color.isValid() && !s.partial)
    {
      painter.fillRect(slotRect, s.color);
    }
    else if (s.color.isValid())
    {
      // Heterogeneous group state: fill only the lower half so "some" reads
      // distinctly from the solid "all" and the empty "none".
      const int half = slotRect.height() / 2;
      painter.fillRect(slotRect.adjusted(0, half, 0, 0), s.color);
    }
    else
    {
      // Unsynced: a faint hairline outline, no fill, so gaps read as absence.
      painter.setPen(QPen(gapColor, 1));
      painter.drawLine(slotRect.left(), slotRect.bottom(), slotRect.right(), slotRect.bottom());
    }

    // Lit slot: a white outline mirrors the Glyphs-mode frame brightening so the
    // editor's single-axis highlight still reads once the tile is too small for
    // glyphs and collapses to color slots. Keyed on the per-axis hoveredSlot
    // only (not whole-strip 'hovered'), so the passive per-cell strip - which
    // never sets a hovered slot - is unaffected.
    if (slot == hoveredSlot)
    {
      painter.setPen(QPen(QColor(255, 255, 255, 210), 1));
      painter.setBrush(Qt::NoBrush);
      painter.drawRect(slotRect.adjusted(0, 0, -1, -1));
    }
  }
}

void QmitkMxNSyncBarcodeWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  // The width the glyphs need is derived from the granted height, so a change
  // of height has to re-ask for width - otherwise the strip keeps the width it
  // wanted when it was shorter and the boxes stay bound by that instead.
  if (m_PreferGlyphWidth && event->oldSize().height() != event->size().height())
  {
    this->updateGeometry();
  }
}

void QmitkMxNSyncBarcodeWidget::paintEvent(QPaintEvent* /*event*/)
{
  QPainter painter(this);
  PaintInto(painter, this->rect(), m_Slots, m_Hovered, this->palette().color(QPalette::Mid), m_HoveredSlot);
}

void QmitkMxNSyncBarcodeWidget::mouseMoveEvent(QMouseEvent* event)
{
  // Only the drawn slots are interactive; the strip can be laid out larger than
  // its content, and that surrounding space must not highlight or read as
  // clickable. Axis-clickable mode highlights the single axis under the pointer;
  // the passive strip highlights as a whole.
  const BarcodeLayout layout = ComputeLayout(this->width(), this->height(), m_Slots.size());
  if (m_AxisClickable)
  {
    const int slot = this->SlotAt(event->pos());
    if (slot != m_HoveredSlot)
    {
      m_HoveredSlot = slot;
      emit AxisHovered(slot);
      this->update();
    }
    this->setCursor(slot >= 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
  }
  else
  {
    const bool over = this->ContentRect(layout).contains(event->pos());
    if (over != m_Hovered)
    {
      m_Hovered = over;
      this->update();
    }
    this->setCursor(over ? Qt::PointingHandCursor : Qt::ArrowCursor);

    // Reporting is separate from the whole-strip hover above: it tells a receiver
    // that answers editor-wide (the sync peek) where the pointer is, and touches
    // nothing this strip paints. Being on the strip is reported in either render
    // mode, because the answer is about the whole window and not only about one
    // axis. Only the emphasis needs a glyph to point at: a collapsed colour-bar
    // slot is a featureless 7 px column, so it names no axis and the receiver
    // answers with none emphasised - the same state as resting between two
    // glyphs.
    const int reportedSlot = over && layout.mode == BarcodeLayout::Mode::Glyphs
                               ? this->SlotAt(event->pos())
                               : -1;
    if (over != m_ReportedOverStrip || reportedSlot != m_ReportedSlot)
    {
      m_ReportedOverStrip = over;
      m_ReportedSlot = reportedSlot;
      emit PeekHovered(over, reportedSlot);
    }
  }
  QWidget::mouseMoveEvent(event);
}

void QmitkMxNSyncBarcodeWidget::leaveEvent(QEvent* event)
{
  const bool hadHoveredSlot = m_HoveredSlot != -1;
  const bool hadReport = m_ReportedOverStrip;
  if (m_Hovered || m_HoveredSlot != -1)
  {
    m_Hovered = false;
    m_HoveredSlot = -1;
    this->update();
  }
  m_ReportedOverStrip = false;
  m_ReportedSlot = -1;
  if (hadHoveredSlot)
  {
    emit AxisHovered(-1);
  }
  if (hadReport)
  {
    emit PeekHovered(false, -1);
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
  // A click (not a drag) on the drawn slots is a button; the surrounding space
  // is inert. In axis-clickable mode the click targets the axis under the
  // pointer (AxisClicked); otherwise the whole strip opens the editor (Clicked).
  const BarcodeLayout layout = ComputeLayout(this->width(), this->height(), m_Slots.size());
  if (event->button() == Qt::LeftButton
      && (event->pos() - m_PressPos).manhattanLength() < QApplication::startDragDistance()
      && this->ContentRect(layout).contains(event->pos()))
  {
    if (m_AxisClickable)
    {
      if (const int slot = this->SlotAt(event->pos()); slot >= 0)
      {
        emit AxisClicked(slot);
      }
    }
    else
    {
      emit Clicked();
    }
  }
  QWidget::mouseReleaseEvent(event);
}

bool QmitkMxNSyncBarcodeWidget::event(QEvent* event)
{
  // The interactive strip explains an axis in words; the passive strip's axes
  // are explained by the editor's sync peek, and two popups over the same image
  // would say overlapping things.
  if (m_AxisClickable && event->type() == QEvent::ToolTip)
  {
    auto* helpEvent = static_cast<QHelpEvent*>(event);
    const int slot = this->SlotAt(helpEvent->pos());
    if (slot >= 0 && !m_Slots[slot].tooltip.isEmpty())
    {
      QToolTip::showText(helpEvent->globalPos(), m_Slots[slot].tooltip, this);
      return true;
    }
  }
  return QWidget::event(event);
}
