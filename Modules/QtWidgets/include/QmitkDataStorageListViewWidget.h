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
#include <QSortFilterProxyModel>

#include "ui_QmitkDataStorageListViewWidget.h"

/*
* @brief This is a widget to offer a simple list view on a data storage.
*/
class MITKQTWIDGETS_EXPORT QmitkDataStorageListViewWidget : public QmitkAbstractDataStorageViewWidget
{
  Q_OBJECT

public:
  QmitkDataStorageListViewWidget(QWidget* parent = nullptr);

  virtual QAbstractItemView* GetView() override;
  virtual QAbstractItemView* GetView() const override;

  virtual void SetSelectionMode(SelectionMode mode) override;
  virtual SelectionMode GetSelectionMode() const override;

protected:
  virtual void Initialize() override;

  QmitkAbstractDataStorageModel* m_StorageModel;
  Ui_QmitkDataStorageListViewWidget m_Controls;
};

#endif // QMITKABSTRACTDATASTORAGEMODEL_H
