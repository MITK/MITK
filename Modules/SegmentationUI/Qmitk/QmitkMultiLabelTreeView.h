/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMultiLabelTreeView_h
#define QmitkMultiLabelTreeView_h

#include <MitkSegmentationUIExports.h>

#include <QTreeView>

/*
* @brief This is an inspector that offers a simple list view on a data storage.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkMultiLabelTreeView : public QTreeView
{
  Q_OBJECT

public:
  QmitkMultiLabelTreeView(QWidget* parent = nullptr);

protected:
  QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex& index, const QEvent* event = nullptr) const override;
};

#endif
