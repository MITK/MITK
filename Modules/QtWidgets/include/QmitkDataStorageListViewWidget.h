/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKDATASTORAGELISTVIEWWIDGET_H
#define QMITKDATASTORAGELISTVIEWWIDGET_H

#include <MitkQtWidgetsExports.h>

#include <QmitkAbstractDataStorageViewWidget.h>

#include "ui_QmitkDataStorageListViewWidget.h"

/*
* @brief This abstract class extends the 'QAbstractItemModel' to accept an 'mitk::DataStorage' and a 'mitk::NodePredicateBase'.,
*   It registers itself as a node event listener of the data storage.
*   The 'QmitkDataStorageListViewWidget' provides three empty functions, 'NodeAdded', 'NodeChanged' and 'NodeRemoved', that
*   may be implemented by the subclasses. These functions allow to react to the 'AddNodeEvent', 'ChangedNodeEvent' and
*   'RemoveNodeEvent' of the data storage. This might be useful to force an update on the custom view to correctly
*   represent the content of the data storage.
*
*   A concrete implementations of this class is used to store the temporarily shown data nodes of the data storage.
*   These nodes may be a subset of all the nodes inside the data storage, if a specific node predicate is set.
*
*   A model that implements this class has to return mitk::DataNode::Pointer objects for model indexes when the
*   role is QmitkDataNodeRole.
*/
class MITKQTWIDGETS_EXPORT QmitkDataStorageListViewWidget : public QmitkAbstractDataStorageViewWidget
{

public:
  QmitkDataStorageListViewWidget(QWidget* parent = nullptr);

  virtual QAbstractItemView* GetView() override;
  virtual QAbstractItemView* GetView() const override;

protected:
  virtual void Initialize() override;

  QmitkAbstractDataStorageModel* m_StorageModel;

  Ui_QmitkDataStorageListViewWidget m_Controls;
};

#endif // QMITKABSTRACTDATASTORAGEMODEL_H
