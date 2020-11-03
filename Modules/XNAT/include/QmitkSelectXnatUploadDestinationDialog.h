/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSELECTXNATUPLOADDESTINATIONDIALOG_H
#define QMITKSELECTXNATUPLOADDESTINATIONDIALOG_H

#include <MitkXNATExports.h>

#include <QDialog>

namespace Ui
{
  class QmitkSelectXnatUploadDestinationDialog;
}

class ctkXnatObject;
class ctkXnatSession;
class QModelIndex;
class QmitkXnatTreeModel;

class MITKXNAT_EXPORT QmitkSelectXnatUploadDestinationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitkSelectXnatUploadDestinationDialog(ctkXnatSession *session, const QStringList &, QWidget *parent = nullptr);
  ~QmitkSelectXnatUploadDestinationDialog() override;

  ctkXnatObject *GetUploadDestination();
  void SetXnatResourceFolderUrl(const QString &url);

protected slots:

  void OnUpload();
  void OnSelectResource(bool selectResource);
  void OnSelectFromTreeView(bool selectFromTreeView);
  void OnResourceEntered(const QString &resourceEntered);
  void OnResourceSelected(const QString &resource);
  void OnXnatNodeSelected(const QModelIndex &);
  void OnCancel();

private:
  QmitkXnatTreeModel *m_TreeModel;
  QString m_Url;
  QString m_ResourceName;
  bool m_CreateNewFolder;
  Ui::QmitkSelectXnatUploadDestinationDialog *ui;
};

#endif // QMITKSELECTXNATUPLOADDESTINATIONDIALOG_H
