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

#include "QmitkSimpleBarrierParametersDelegate.h"

#include <QPainter>
#include <QApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QListWidget>

QmitkSimpleBarrierParametersDelegate::QmitkSimpleBarrierParametersDelegate(QObject* /*parent*/)
{
}

void QmitkSimpleBarrierParametersDelegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QVariant data = index.data(Qt::DisplayRole);
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  QStyle* style = QApplication::style();

  QString text = "Invalid data";

  if (data.isValid())
  {
    QStringList names = data.toStringList();
    text.clear();

    for (QStringList::const_iterator pos = names.begin(); pos != names.end(); ++pos)
    {
      if (pos != names.begin())
      {
        text.append(", ");
      }

      text.append(*pos);
    }
  }

  style->drawItemText(painter, opt.rect.adjusted(0, 0, -5, 0), Qt::AlignRight | Qt::AlignVCenter,
                      opt.palette, true, text);
}

QWidget* QmitkSimpleBarrierParametersDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem&
    , const QModelIndex& index) const
{
  QVariant data = index.data(Qt::EditRole);
  QVariant displayData = index.data(Qt::DisplayRole);

  if (data.isValid())
  {
    QListWidget* list = new QListWidget(parent);
    list->setFixedSize(100, 100);

    list->installEventFilter(const_cast<QmitkSimpleBarrierParametersDelegate*>(this));

    return list;

  }
  else
  {
    return new QLabel(displayData.toString(), parent);
  }
}

void QmitkSimpleBarrierParametersDelegate::setEditorData(QWidget* editor,
    const QModelIndex& index) const
{
  QVariant data = index.data(Qt::EditRole);
  QVariant displayData = index.data(Qt::DisplayRole);

  if (data.isValid() && displayData.isValid())
  {

    QListWidget* list = qobject_cast<QListWidget*>(editor);

    if (list)
    {
      list->clear();

      QStringList names = data.toStringList();
      QStringList checkedNames = displayData.toStringList();

      for (QStringList::const_iterator pos = names.begin(); pos != names.end(); ++pos)
      {
        QListWidgetItem* item = new QListWidgetItem(*pos, list);
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

        if (checkedNames.contains(*pos))
        {
          item->setCheckState(Qt::Checked);
        }
        else
        {
          item->setCheckState(Qt::Unchecked);
        }
      }
    }
    else
    {
      QStyledItemDelegate::setEditorData(editor, index);
    }
  }
}

void QmitkSimpleBarrierParametersDelegate::setModelData(QWidget* editor, QAbstractItemModel* model
    , const QModelIndex& index) const
{
  QVariant data = index.data(Qt::EditRole);

  if (data.isValid())
  {
    QListWidget* list = qobject_cast<QListWidget*>(editor);
    QStringList selectedItems;

    if (list)
    {
      for (int row = 0; row < list->count(); ++row)
      {
        QListWidgetItem* item = list->item(row);

        if (item->checkState() == Qt::Checked)
        {
          selectedItems.append(item->text());
        }
      }
    }

    QVariant selectedVariant(selectedItems);
    model->setData(index, selectedVariant);
  }
  else
  {
    QStyledItemDelegate::setModelData(editor, model, index);
  }
}
