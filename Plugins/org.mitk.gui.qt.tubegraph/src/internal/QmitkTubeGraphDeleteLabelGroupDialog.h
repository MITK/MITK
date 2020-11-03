/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _QMITKTUBEGRAPHDELETELABELGROUPDIALOG_H_INCLUDED
#define _QMITKTUBEGRAPHDELETELABELGROUPDIALOG_H_INCLUDED


#include <QDialog>

class QPushButton;
class QListWidget;
class QLabel;
class QWidget;

class QmitkTubeGraphDeleteLabelGroupDialog : public QDialog
{
  Q_OBJECT

public:

  QmitkTubeGraphDeleteLabelGroupDialog(QWidget* parent = nullptr);
  ~QmitkTubeGraphDeleteLabelGroupDialog() override;

  QStringList GetSelectedLabelGroups();
  void SetLabelGroups(const QStringList &labelGroups);


 protected slots:
   void OnDeleteLabelGroupClicked();

protected:
    QLabel*  descriptionLabel;
    QListWidget* labelGroupListWidget;
    QPushButton* deleteButton;
    QPushButton* cancleButton;

    QStringList m_LabelGroupList;
};

#endif
