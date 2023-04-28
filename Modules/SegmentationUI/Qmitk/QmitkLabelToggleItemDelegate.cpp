/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkLabelToggleItemDelegate.h"



#include <QPainter>
#include <QMouseEvent>

QmitkLabelToggleItemDelegate::QmitkLabelToggleItemDelegate(const QIcon& onIcon, const QIcon& offIcon, QObject * parent) 
  :QStyledItemDelegate(parent),
   m_OnIcon(onIcon),
   m_OffIcon(offIcon)
{
}

void QmitkLabelToggleItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
  const QModelIndex &index) const
{
  QVariant data = index.data(Qt::EditRole);

  if (data.canConvert<bool>())
  {
    if (data.toBool())
    {
      m_OnIcon.paint(painter, option.rect);
    }
    else
    {
      m_OffIcon.paint(painter, option.rect);
    }
  }
  else
  {
    QStyledItemDelegate::paint(painter, option, index);
  }
}

QSize QmitkLabelToggleItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  auto defaultSize = QStyledItemDelegate::sizeHint(option, index);
  return QSize(defaultSize.height(), defaultSize.height());
}

bool QmitkLabelToggleItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &,
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

  auto visVar = index.data(Qt::EditRole);
  auto data = visVar.isValid()
     ? QVariant(!(visVar.toBool()))
     : QVariant(true);
  return model->setData(index, data, Qt::EditRole);

  return false;
};
