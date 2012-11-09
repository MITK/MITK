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


#ifndef QMITKDATANODEITEMMODEL_H
#define QMITKDATANODEITEMMODEL_H

#include <QStandardItemModel>

#include <mitkDataNode.h>

class QmitkDataNodeItemModel : public QStandardItemModel
{

public:

  QmitkDataNodeItemModel(QObject* parent = 0);

  void AddDataNode(mitk::DataNode::Pointer node);
  void AddDataNodes(const QList<mitk::DataNode::Pointer>& nodes);
};

#endif // QMITKDATANODEITEMMODEL_H
