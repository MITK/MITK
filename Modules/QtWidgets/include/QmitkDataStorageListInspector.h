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

#ifndef QMITKDATASTORAGELISTINSPECTOR_H
#define QMITKDATASTORAGELISTINSPECTOR_H

#include <MitkQtWidgetsExports.h>

#include <QmitkAbstractDataStorageInspector.h>
#include <QSortFilterProxyModel>

#include "ui_QmitkDataStorageListInspector.h"

/*
* @brief This is an inspector that offers a simple list view on a data storage.
*/
class MITKQTWIDGETS_EXPORT QmitkDataStorageListInspector : public QmitkAbstractDataStorageInspector
{
  Q_OBJECT

public:
  QmitkDataStorageListInspector(QWidget* parent = nullptr);

  virtual QAbstractItemView* GetView() override;
  virtual const QAbstractItemView* GetView() const override;

  virtual void SetSelectionMode(SelectionMode mode) override;
  virtual SelectionMode GetSelectionMode() const override;

protected:
  virtual void Initialize() override;

  QmitkAbstractDataStorageModel* m_StorageModel;
  Ui_QmitkDataStorageListInspector m_Controls;
};

#endif // QMITKDATASTORAGELISTINSPECTOR_H
