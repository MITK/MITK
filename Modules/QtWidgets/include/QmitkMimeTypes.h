/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMimeTypes_h
#define QmitkMimeTypes_h

#include <MitkQtWidgetsExports.h>

#include <QByteArray>
#include <QList>
#include <QMimeData>
#include <QString>

namespace mitk
{
  class DataNode;
}

/**
 * @brief The QmitkMimeTypes class prov
 */
class MITKQTWIDGETS_EXPORT QmitkMimeTypes
{
public:
  static const QString DataNodePtrs;            // = "application/x-qmitk-datanode-ptrs";
  static const QString DataStorageTreeItemPtrs; // = "application/x-qmitk-datastorage-treeitem-ptrs";

  static QList<mitk::DataNode *> ToDataNodePtrList(const QByteArray &ba);

  static QList<mitk::DataNode *> ToDataNodePtrList(const QMimeData *mimeData);
};

#endif
