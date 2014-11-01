/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkAnimationItem.h"
#include "QmitkAnimationItemDelegate.h"
#include <QPainter>
#include <QStandardItemModel>
#include <limits>

QmitkAnimationItemDelegate::QmitkAnimationItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QmitkAnimationItemDelegate::~QmitkAnimationItemDelegate()
{
}

void QmitkAnimationItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  const QStandardItemModel* model = dynamic_cast<const QStandardItemModel*>(index.model());
  const QmitkAnimationItem* thisItem = dynamic_cast<QmitkAnimationItem*>(model->item(index.row(), 1));
  QList<const QmitkAnimationItem*> items;

  const int rowCount = model->rowCount();

  for (int i = 0; i < rowCount; ++i)
    items << dynamic_cast<QmitkAnimationItem*>(model->item(i, 1));

  double totalDuration = 0.0;
  double previousStart = 0.0;
  double thisStart = 0.0;

  Q_FOREACH(const QmitkAnimationItem* item, items)
  {
    if (item->GetStartWithPrevious())
    {
      totalDuration = std::max(totalDuration, previousStart + item->GetDelay() + item->GetDuration());
    }
    else
    {
      previousStart = totalDuration;
      totalDuration += item->GetDelay() + item->GetDuration();
    }

    if (item == thisItem)
      thisStart = previousStart;
  }

  QColor color("DarkKhaki");

  painter->setBrush(color);
  painter->setPen(Qt::NoPen);

  const QRect& rect = option.rect;
  const double widthPerSecond = rect.width() / totalDuration;

  painter->drawRect(
    rect.x() + static_cast<int>(widthPerSecond * (thisStart + thisItem->GetDelay())),
    rect.y() + 1,
    static_cast<int>(widthPerSecond * thisItem->GetDuration()),
    rect.height() - 2);

  if (thisItem->GetDelay() > std::numeric_limits<double>::min())
  {
    QPen pen(color);
    painter->setPen(pen);

    painter->drawLine(
      rect.x() + static_cast<int>(widthPerSecond * thisStart),
      rect.y() + 1,
      rect.x() + static_cast<int>(widthPerSecond * thisStart),
      rect.y() + rect.height() - 2);

    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);

    painter->drawLine(
      rect.x() + static_cast<int>(widthPerSecond * thisStart),
      rect.y() + rect.height() / 2,
      rect.x() + static_cast<int>(widthPerSecond * (thisStart + thisItem->GetDelay())) - 1,
      rect.y() + rect.height() / 2);
  }

  QStyledItemDelegate::paint(painter, option, index);
}
