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

#ifndef QMITKDATASTORAGETREEINSPECTOR_H
#define QMITKDATASTORAGETREEINSPECTOR_H

#include <MitkQtWidgetsExports.h>

#include <QmitkAbstractDataStorageInspector.h>
#include <QSortFilterProxyModel>

#include "ui_QmitkDataStorageTreeInspector.h"

/*
* @brief This is an inspector that offers a simple tree view on a data storage.
* Something like the "data manager plugin", but in simple/light (with less functionality)
* It uses the QmitkDataStorageSimpleTreeModel.
*/
class MITKQTWIDGETS_EXPORT QmitkDataStorageTreeInspector : public QmitkAbstractDataStorageInspector
{
  Q_OBJECT

public:
  QmitkDataStorageTreeInspector(QWidget* parent = nullptr);

  virtual QAbstractItemView* GetView() override;
  virtual const QAbstractItemView* GetView() const override;

  virtual void SetSelectionMode(SelectionMode mode) override;
  virtual SelectionMode GetSelectionMode() const override;

protected:
  virtual void Initialize() override;

  QmitkAbstractDataStorageModel* m_StorageModel;
  Ui_QmitkDataStorageTreeInspector m_Controls;
};

#endif // QMITKDATASTORAGETREEINSPECTOR_H
