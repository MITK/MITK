/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDoseVisualStyleDelegate.h"

#include <QPainter>
#include <QApplication>
#include <QCheckBox>
#include <QLabel>
#include <QMouseEvent>

static QRect CheckBoxRect(const QStyleOptionViewItem &view_item_style_options) {
  QStyleOptionButton check_box_style_option;
  QRect check_box_rect = QApplication::style()->subElementRect(
    QStyle::SE_CheckBoxIndicator,
    &check_box_style_option);
  QPoint check_box_point(view_item_style_options.rect.x() +
    view_item_style_options.rect.width() / 2 -
    check_box_rect.width() / 2,
    view_item_style_options.rect.y() +
    view_item_style_options.rect.height() / 2 -
    check_box_rect.height() / 2);
  return QRect(check_box_point, check_box_rect.size());
}

QmitkDoseVisualStyleDelegate::QmitkDoseVisualStyleDelegate(QObject * /*parent*/)
{
}

void QmitkDoseVisualStyleDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  bool checkValue = index.data(Qt::DisplayRole).toBool();

  QStyleOptionButton BtnStyle;
  BtnStyle.state = QStyle::State_Enabled;

  if(checkValue)
  {
    BtnStyle.state |= QStyle::State_On;
  }
  else
  {
    BtnStyle.state |= QStyle::State_Off;
  }

  BtnStyle.direction = QApplication::layoutDirection();
  BtnStyle.rect = CheckBoxRect(option);
  QApplication::style()->drawControl(QStyle::CE_CheckBox,&BtnStyle,painter );
}

bool QmitkDoseVisualStyleDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &,
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

  bool newState = !(index.data(Qt::EditRole).toBool());

  return model->setData(index, QVariant(newState), Qt::EditRole);
};
