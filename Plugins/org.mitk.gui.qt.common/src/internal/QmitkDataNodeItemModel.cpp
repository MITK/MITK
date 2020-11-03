/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkDataNodeItemModel.h"

#include <QmitkEnums.h>
#include <QmitkCustomVariants.h>

QmitkDataNodeItemModel::QmitkDataNodeItemModel(QObject *parent)
  : QStandardItemModel(parent)
{
}

void QmitkDataNodeItemModel::AddDataNode(mitk::DataNode::Pointer node)
{
  QList<mitk::DataNode::Pointer> nodes;
  nodes << node;
  this->AddDataNodes(nodes);
}

void QmitkDataNodeItemModel::AddDataNodes(const QList<mitk::DataNode::Pointer> &nodes)
{
  QList<QStandardItem*> items;
  foreach(mitk::DataNode::Pointer node, nodes)
  {
    auto   item = new QStandardItem();
    item->setData(QVariant::fromValue<mitk::DataNode::Pointer>(node), QmitkDataNodeRole);
    items << item;
  }
  this->appendRow(items);
}
