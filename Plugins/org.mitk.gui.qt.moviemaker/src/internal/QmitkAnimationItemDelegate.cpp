/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAnimationItem.h"
#include "QmitkAnimationItemDelegate.h"
#include <QApplication>
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
  painter->save();

  if (option.state & QStyle::State_Selected)
    painter->fillRect(option.rect, option.palette.highlight());

  if (index.column() == 0)
  {
    QStyleOptionViewItem opt = option;
    this->initStyleOption(&opt, index);

    QRect rect = QRect(opt.rect.x() + 3, opt.rect.y(), opt.rect.width() - 6, opt.rect.height());
    QString text = option.fontMetrics.elidedText(index.data().toString(), Qt::ElideRight, rect.width());
    QPalette::ColorRole colorRole = (option.state & QStyle::State_Selected)
      ? QPalette::HighlightedText
      : QPalette::Text;

    QApplication::style()->drawItemText(painter, rect, 0, option.palette, true, text, colorRole);
  }
  else if (index.column() == 1)
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

    const QRect& rect = option.rect;
    const double widthPerSecond = rect.width() / totalDuration;

    QColor color = thisItem->GetStartWithPrevious()
      ? QColor("DarkGray")
      : QColor("DimGray");

    painter->setBrush(color);
    painter->setPen(Qt::NoPen);

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
  }

  painter->restore();
}
