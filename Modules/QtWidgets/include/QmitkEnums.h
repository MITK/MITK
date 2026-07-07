/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkEnums_h
#define QmitkEnums_h

/**
 * \ingroup QmitkModule
 * \brief Custom item model roles for MITK data node access.
 *
 * These roles extend Qt::ItemDataRole and are used by MITK data storage
 * models to return mitk::DataNode objects from QAbstractItemModel::data().
 *
 * \sa QmitkAbstractDataStorageModel
 */
enum QmitkItemModelRole
{
  QmitkDataNodeRole = 64,         ///< Returns a mitk::DataNode::Pointer (smart pointer) wrapped in QVariant.
  QmitkDataNodeRawPointerRole = 65 ///< Returns a raw mitk::DataNode* wrapped in QVariant.
};

#endif
