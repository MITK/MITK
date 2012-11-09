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
    QStandardItem* item = new QStandardItem();
    item->setData(QVariant::fromValue<mitk::DataNode::Pointer>(node), QmitkDataNodeRole);
    items << item;
  }
  this->appendRow(items);
}
