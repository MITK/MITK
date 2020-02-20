/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMimeTypes.h"

const QString QmitkMimeTypes::DataNodePtrs = "application/x-qmitk-datanode-ptrs";
const QString QmitkMimeTypes::DataStorageTreeItemPtrs = "application/x-qmitk-datastorage-treeitem-ptrs";

#include <iostream>

#include <QDataStream>

QList<mitk::DataNode *> QmitkMimeTypes::ToDataNodePtrList(const QByteArray &ba)
{
  QDataStream ds(ba);
  QList<mitk::DataNode *> result;
  while (!ds.atEnd())
  {
    quintptr dataNodePtr;
    ds >> dataNodePtr;
    result.push_back(reinterpret_cast<mitk::DataNode *>(dataNodePtr));
  }
  return result;
}

QList<mitk::DataNode *> QmitkMimeTypes::ToDataNodePtrList(const QMimeData *mimeData)
{
  if (mimeData == nullptr || !mimeData->hasFormat(QmitkMimeTypes::DataNodePtrs))
  {
    return QList<mitk::DataNode *>();
  }
  return ToDataNodePtrList(mimeData->data(QmitkMimeTypes::DataNodePtrs));
}
