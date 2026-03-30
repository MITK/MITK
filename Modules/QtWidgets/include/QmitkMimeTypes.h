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
 * \brief Provides MIME type constants and conversion utilities for MITK data node drag-and-drop.
 *
 * This class defines MIME type strings used for encoding mitk::DataNode pointers
 * in drag-and-drop operations and clipboard transfers within MITK Qt widgets.
 *
 * \sa QmitkDnDDataNodeWidget
 * \sa QmitkDataStorageTreeModel
 */
class MITKQTWIDGETS_EXPORT QmitkMimeTypes
{
public:
  /** \brief MIME type for serialized data node pointers ("application/x-qmitk-datanode-ptrs"). */
  static const QString DataNodePtrs;

  /** \brief MIME type for serialized data storage tree item pointers ("application/x-qmitk-datastorage-treeitem-ptrs"). */
  static const QString DataStorageTreeItemPtrs;

  /**
   * \brief Deserializes a QByteArray into a list of data node raw pointers.
   * \param[in] ba The byte array containing serialized data node pointers.
   * \return A list of raw data node pointers.
   */
  static QList<mitk::DataNode *> ToDataNodePtrList(const QByteArray &ba);

  /**
   * \brief Extracts data node pointers from QMimeData.
   * \param[in] mimeData The MIME data to extract from.
   * \return A list of raw data node pointers, or an empty list if the MIME type is not present.
   */
  static QList<mitk::DataNode *> ToDataNodePtrList(const QMimeData *mimeData);
};

#endif
