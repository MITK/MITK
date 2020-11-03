/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKDATASTORAGELISTINSPECTOR_H
#define QMITKDATASTORAGELISTINSPECTOR_H

#include <MitkQtWidgetsExports.h>

#include "QmitkAbstractDataStorageInspector.h"
#include "QmitkSimpleTextOverlayWidget.h"

#include "ui_QmitkDataStorageListInspector.h"

/*
* @brief This is an inspector that offers a simple list view on a data storage.
*/
class MITKQTWIDGETS_EXPORT QmitkDataStorageListInspector : public QmitkAbstractDataStorageInspector
{
  Q_OBJECT

public:
  QmitkDataStorageListInspector(QWidget* parent = nullptr);

  QAbstractItemView* GetView() override;
  const QAbstractItemView* GetView() const override;

  void SetSelectionMode(SelectionMode mode) override;
  SelectionMode GetSelectionMode() const override;

protected:
  void Initialize() override;
  void OnModelReset();

  QmitkAbstractDataStorageModel* m_StorageModel;
  Ui_QmitkDataStorageListInspector m_Controls;
  QmitkSimpleTextOverlayWidget* m_Overlay;
};

#endif // QMITKDATASTORAGELISTINSPECTOR_H
