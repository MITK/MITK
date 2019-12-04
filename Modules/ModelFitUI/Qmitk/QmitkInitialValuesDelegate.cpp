/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkInitialValuesDelegate.h"

#include <mitkModelTraitsInterface.h>
#include <QmitkDataStorageComboBox.h>

#include <QPainter>
#include <QApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QListWidget>

QmitkInitialValuesDelegate::QmitkInitialValuesDelegate(QObject* /*parent*/) : m_Storage(nullptr), m_Predicate(nullptr)
{
}

QWidget* QmitkInitialValuesDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option
    , const QModelIndex& index) const
{
  QVariant data = index.data(Qt::EditRole);

  if (data.isValid())
  {
    if (valueType(index)==0)
    {
      return QStyledItemDelegate::createEditor(parent, option, index);
    }
    else
    {
      QmitkDataStorageComboBox* box = new QmitkDataStorageComboBox(m_Storage, m_Predicate, parent);

      return box;
    }

  }
  else
  {
    return new QLabel(data.toString(), parent);
  }
}

int
QmitkInitialValuesDelegate::
valueType(const QModelIndex& index) const
{
  QVariant data = index.data(Qt::UserRole);

  return data.toInt();
}

void QmitkInitialValuesDelegate::setEditorData(QWidget* editor,
    const QModelIndex& index) const
{
  QVariant data = index.data(Qt::EditRole);

  if (data.isValid())
  {
    if (valueType(index) == 0)
    {
      return QStyledItemDelegate::setEditorData(editor, index);
    }
    else
    {
      QmitkDataStorageComboBox* box = qobject_cast<QmitkDataStorageComboBox*>(editor);

      mitk::DataNode *node = static_cast<mitk::DataNode*>(data.value<void*>());

      auto index = box->Find(node);
      box->setCurrentIndex(index);
    }
  }
}

void QmitkInitialValuesDelegate::setModelData(QWidget* editor, QAbstractItemModel* model
    , const QModelIndex& index) const
{
  QVariant data = index.data(Qt::EditRole);

  if (data.isValid() && valueType(index) == 1)
  {
    QmitkDataStorageComboBox* box = qobject_cast<QmitkDataStorageComboBox*>(editor);
    QVariant newNode = qVariantFromValue<void*>(static_cast<void*>(box->GetSelectedNode().GetPointer()));
    model->setData(index, newNode);
  }
  else
  {
    QStyledItemDelegate::setModelData(editor, model, index);
  }
}

void QmitkInitialValuesDelegate::
setDataStorage(mitk::DataStorage* storage)
{
  this->m_Storage = storage;
};

void QmitkInitialValuesDelegate::
setNodePredicate(mitk::NodePredicateBase* predicate)
{
  this->m_Predicate = predicate;
};
