/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)
Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef _QMITKTUBEGRAPHDELETELABELGROUPDIALOG_H_INCLUDED
#define _QMITKTUBEGRAPHDELETELABELGROUPDIALOG_H_INCLUDED


#include <qdialog.h>

class QPushButton;
class QListWidget;
class QLabel;
class QWidget;

class QmitkTubeGraphDeleteLabelGroupDialog : public QDialog
{
  Q_OBJECT

public:

  QmitkTubeGraphDeleteLabelGroupDialog(QWidget* parent = nullptr);
  virtual ~QmitkTubeGraphDeleteLabelGroupDialog();

  const QStringList GetSelectedLabelGroups();
  void SetLabelGroups(QStringList labelGroups);


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
