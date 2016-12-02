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

#include "QmitkVisibilityDelegate.h"

// qt
#include <QPainter>
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QColorDialog>
#include <QMouseEvent>
#include <QCheckBox>

static QRect CheckBoxRect(const QStyleOptionViewItem &view_item_style_options)
{
  QStyleOptionButton check_box_style_option;
  QRect check_box_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &check_box_style_option);
  QPoint check_box_point(view_item_style_options.rect.x() +
    view_item_style_options.rect.width() / 2 -
    check_box_rect.width() / 2,
    view_item_style_options.rect.y() +
    view_item_style_options.rect.height() / 2 -
    check_box_rect.height() / 2);
  return QRect(check_box_point, check_box_rect.size());
}

QmitkVisibilityDelegate::QmitkVisibilityDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
  // nothing here
}

QWidget* QmitkVisibilityDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
  QCheckBox* editor = new QCheckBox(parent);
//  editor->setFrame(false);
//  editor->setMinimum(0);
//  editor->setMaximum(100);
//
  return editor;
}

//void QmitkVisibilityDelegate::setEditorData(QWidget *editor,
//  const QModelIndex &index) const
//{
//  int value = index.model()->data(index, Qt::EditRole).toInt();
//
//  QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
//  spinBox->setValue(value);
//}
//
//void QmitkVisabilityDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
//  const QModelIndex &index) const
//{
//  QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
//  spinBox->interpretText();
//  int value = spinBox->value();
//
//  model->setData(index, value, Qt::EditRole);
//}

void QmitkVisibilityDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
   editor->setGeometry(option.rect);
}

void QmitkVisibilityDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  bool checkValue = index.data(Qt::DisplayRole).toBool();

  QStyleOptionButton BtnStyle;
  BtnStyle.state = QStyle::State_Enabled;

  if (checkValue)
  {
    BtnStyle.state |= QStyle::State_On;
  }
  else
  {
    BtnStyle.state |= QStyle::State_Off;
  }

  BtnStyle.direction = QApplication::layoutDirection();
  BtnStyle.rect = CheckBoxRect(option);
  QApplication::style()->drawControl(QStyle::CE_CheckBox, &BtnStyle, painter);
}

bool QmitkVisibilityDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem &option, const QModelIndex &index)
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
}
