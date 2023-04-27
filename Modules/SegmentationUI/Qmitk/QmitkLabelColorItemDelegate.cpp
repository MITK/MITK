/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkLabelColorItemDelegate.h"

#include <QPainter>
#include <QColorDialog>
#include <QMouseEvent>

QmitkLabelColorItemDelegate::QmitkLabelColorItemDelegate(QObject * /*parent*/)
{
}

void QmitkLabelColorItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
  const QModelIndex &index) const
{
  QVariant data = index.data(Qt::EditRole);

  if (data.canConvert<QColor>())
  {
    QColor color = data.value<QColor>();

    painter->fillRect(option.rect, color);
  }
  else
  {
    QStyledItemDelegate::paint(painter, option, index);
  }
}

bool QmitkLabelColorItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &,
  const QModelIndex &index)
{
  Q_ASSERT(event);
  Q_ASSERT(model);

  // make sure that the item is checkable
  Qt::ItemFlags flags = model->flags(index);
  if (!(flags & Qt::ItemIsEditable) || !(flags & Qt::ItemIsEnabled))
  {
    return false;
  }

  // make sure that we have the right event type
  QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
  if (nullptr == mouseEvent || mouseEvent->type() != QEvent::MouseButtonRelease || mouseEvent->button() != Qt::LeftButton)
  {
    return false;
  }

  QColor oldColor = index.data(Qt::EditRole).value<QColor>();
  QColor newColor = QColorDialog::getColor(oldColor, nullptr);

  if (newColor.isValid())
  {
    return model->setData(index, QVariant(newColor), Qt::EditRole);
  }

  return false;
};
