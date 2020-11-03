/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDoseColorDelegate.h"

#include <QPainter>
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QColorDialog>
#include <QMouseEvent>

QmitkDoseColorDelegate::QmitkDoseColorDelegate(QObject * /*parent*/)
{
}

void QmitkDoseColorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
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

bool QmitkDoseColorDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &,
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
  if (!mouseEvent)
  {
    return false;
  }
  else
  {
    if (mouseEvent->type() != QEvent::MouseButtonRelease || mouseEvent->button() != Qt::LeftButton)
    {
      return false;
    }
  }

  QColor oldcolor = index.data(Qt::EditRole).value<QColor>();
  QColor newColor = QColorDialog::getColor(oldcolor, nullptr);

  if (newColor.isValid())
  {
    return model->setData(index, QVariant(newColor), Qt::EditRole);
  }

  return false;
};
