/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNSyncBarcodeWidget.h"

#include <QPainter>

#include <algorithm>

namespace
{
  constexpr int SlotWidth = 7;
  constexpr int BarcodeHeight = 14;
  constexpr int SlotGap = 1;
}

QmitkMxNSyncBarcodeWidget::QmitkMxNSyncBarcodeWidget(QWidget* parent)
  : QWidget(parent)
{
  this->setToolTip(tr("Per-dimension synchronization: a colored slot marks a "
                      "dimension this cell is linked on, in the group's hue."));
}

QmitkMxNSyncBarcodeWidget::~QmitkMxNSyncBarcodeWidget()
{
}

void QmitkMxNSyncBarcodeWidget::SetSlots(const QList<QColor>& slotColors)
{
  if (slotColors == m_Slots)
  {
    return;
  }
  m_Slots = slotColors;
  this->updateGeometry();
  this->update();
}

QList<QColor> QmitkMxNSyncBarcodeWidget::Slots() const
{
  return m_Slots;
}

QSize QmitkMxNSyncBarcodeWidget::sizeHint() const
{
  const int slotCount = std::max(1, static_cast<int>(m_Slots.size()));
  return QSize(slotCount * SlotWidth + (slotCount - 1) * SlotGap, BarcodeHeight);
}

QSize QmitkMxNSyncBarcodeWidget::minimumSizeHint() const
{
  return this->sizeHint();
}

void QmitkMxNSyncBarcodeWidget::paintEvent(QPaintEvent* /*event*/)
{
  if (m_Slots.isEmpty())
  {
    return;
  }

  QPainter painter(this);
  const QColor gap = this->palette().color(QPalette::Mid);

  const int top = (this->height() - BarcodeHeight) / 2;
  for (int slot = 0; slot < m_Slots.size(); ++slot)
  {
    const QRect slotRect(slot * (SlotWidth + SlotGap), top, SlotWidth, BarcodeHeight);
    if (m_Slots[slot].isValid())
    {
      painter.fillRect(slotRect, m_Slots[slot]);
    }
    else
    {
      // Unsynced: a faint hairline outline, no fill, so gaps read as absence.
      painter.setPen(QPen(gap, 1));
      painter.drawLine(slotRect.left(), slotRect.bottom(), slotRect.right(), slotRect.bottom());
    }
  }
}
