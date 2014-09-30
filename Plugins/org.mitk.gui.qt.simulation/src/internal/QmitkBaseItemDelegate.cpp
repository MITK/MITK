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

#include "QmitkBaseItemDelegate.h"
#include <sofa/core/objectmodel/BaseData.h>

using namespace sofa::defaulttype;
using namespace sofa::core::objectmodel;

static inline BaseData* GetBaseData(const QModelIndex& index)
{
  return index.data(Qt::UserRole).value<BaseData*>();
}

QmitkBaseItemDelegate::QmitkBaseItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QmitkBaseItemDelegate::~QmitkBaseItemDelegate()
{
}

QWidget* QmitkBaseItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  BaseData* baseData = GetBaseData(index);
  const AbstractTypeInfo* typeInfo = baseData->getValueTypeInfo();

  if (typeInfo->name() == "bool")
  {
    assert(false && "Bool type is directly handled by QmitkBaseTreeWidget!");
  }
  else if (typeInfo->size() == 1)
  {
    if (typeInfo->Integer() || typeInfo->Scalar() || typeInfo->Text())
    {
      // TODO: TagSet
      return QStyledItemDelegate::createEditor(parent, option, index);
    }

    // TODO
  }
  else
  {
    // TODO
  }

  return NULL;
}

void QmitkBaseItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyledItemDelegate::paint(painter, option, index);
}

void QmitkBaseItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QStyledItemDelegate::setEditorData(editor, index);
}

void QmitkBaseItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QStyledItemDelegate::setModelData(editor, model, index);
}
