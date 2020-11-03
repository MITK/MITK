/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKXNATUPLOADFROMDATASTORAGEDIALOG_H
#define QMITKXNATUPLOADFROMDATASTORAGEDIALOG_H

#include <QDialog>

#include "MitkXNATExports.h"
#include <mitkDataNode.h>

namespace Ui
{
  class QmitkXnatUploadFromDataStorageDialog;
}

namespace mitk
{
  class DataStorage;
}

class MITKXNAT_EXPORT QmitkXnatUploadFromDataStorageDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitkXnatUploadFromDataStorageDialog(QWidget *parent = nullptr);
  ~QmitkXnatUploadFromDataStorageDialog() override;

  void SetDataStorage(mitk::DataStorage *ds);
  mitk::DataNode::Pointer GetSelectedNode();

protected slots:

  void OnUpload();
  void OnUploadSceneChecked();
  void OnCancel();

  void OnMITKProjectFileNameEntered(const QString &text);
  void OnDataSelected(const mitk::DataNode *);

private:
  Ui::QmitkXnatUploadFromDataStorageDialog *ui;

  mitk::DataNode::Pointer m_SelectedNode;
};

#endif // QMITKXNATUPLOADFROMDATASTORAGEDIALOG_H
