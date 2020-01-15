/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITKDATANODEITEMMODEL_H
#define QMITKDATANODEITEMMODEL_H

#include <QStandardItemModel>

#include <mitkDataNode.h>

class QmitkDataNodeItemModel : public QStandardItemModel
{

public:

  QmitkDataNodeItemModel(QObject* parent = nullptr);

  void AddDataNode(mitk::DataNode::Pointer node);
  void AddDataNodes(const QList<mitk::DataNode::Pointer>& nodes);
};

#endif // QMITKDATANODEITEMMODEL_H
