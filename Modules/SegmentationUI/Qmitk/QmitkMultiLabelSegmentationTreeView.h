/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKMULTILABELSEGMENTATIONTREEVIEW_H
#define QMITKMULTILABELSEGMENTATIONTREEVIEW_H

#include <MitkSegmentationUIExports.h>

#include <QTreeView>

/*
* @brief This is an inspector that offers a simple list view on a data storage.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkMultiLabelSegmentationTreeView : public QTreeView
{
  Q_OBJECT

public:
  QmitkMultiLabelSegmentationTreeView(QWidget* parent = nullptr);

protected:
  QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex& index, const QEvent* event = nullptr) const override;
};

#endif // QMITKDATASTORAGELISTINSPECTOR_H
