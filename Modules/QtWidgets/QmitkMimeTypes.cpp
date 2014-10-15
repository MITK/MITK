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

#include "QmitkMimeTypes.h"

const QString QmitkMimeTypes::DataNodePtrs = "application/x-qmitk-datanode-ptrs";
const QString QmitkMimeTypes::DataStorageTreeItemPtrs = "application/x-qmitk-datastorage-treeitem-ptrs";

#include <iostream>

QList<mitk::DataNode *> QmitkMimeTypes::ToDataNodePtrList(const QByteArray &ba)
{
  QDataStream ds(ba);
  QList<mitk::DataNode*> result;
  while(!ds.atEnd())
  {
    quintptr dataNodePtr;
    ds >> dataNodePtr;
    result.push_back(reinterpret_cast<mitk::DataNode*>(dataNodePtr));
  }
  return result;
}

QList<mitk::DataNode *> QmitkMimeTypes::ToDataNodePtrList(const QMimeData *mimeData)
{
  if (mimeData == NULL || !mimeData->hasFormat(QmitkMimeTypes::DataNodePtrs))
  {
    return QList<mitk::DataNode*>();
  }
  return ToDataNodePtrList(mimeData->data(QmitkMimeTypes::DataNodePtrs));
}
