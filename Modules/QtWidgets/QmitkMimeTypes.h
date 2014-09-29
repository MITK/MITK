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

#ifndef QMITKMIMETYPES_H
#define QMITKMIMETYPES_H

#include <MitkQtWidgetsExports.h>

#include <QString>
#include <QMimeData>
#include <QByteArray>
#include <QList>

namespace mitk {
class DataNode;
}

/**
 * @brief The QmitkMimeTypes class prov
 */
class QMITK_EXPORT QmitkMimeTypes
{
public:

  static const QString DataNodePtrs; // = "application/x-qmitk-datanode-ptrs";
  static const QString DataStorageTreeItemPtrs; // = "application/x-qmitk-datastorage-treeitem-ptrs";

  static QList<mitk::DataNode*> ToDataNodePtrList(const QByteArray& ba);

  static QList<mitk::DataNode*> ToDataNodePtrList(const QMimeData *mimeData);
};

#endif // QMITKMIMETYPES_H
