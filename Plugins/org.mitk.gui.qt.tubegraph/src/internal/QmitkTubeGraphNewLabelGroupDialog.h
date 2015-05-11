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

#ifndef _QMITKTUBEGRAPHNEWLABELGROUPDIALOG_H_INCLUDED
#define _QMITKTUBEGRAPHNEWLABELGROUPDIALOG_H_INCLUDED

#include <qdialog.h>
#include <vector>
#include "mitkTubeGraphProperty.h"

class QSpacerItem;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QLineEdit;
class QLabel;
class QListWidget;
class QTreeWidget;

class QmitkTubeGraphNewLabelGroupDialog : public QDialog
{
  Q_OBJECT

    typedef mitk::TubeGraphProperty::LabelGroup  LabelGroupType;
  typedef LabelGroupType::Label                LabelType;

public:

  QmitkTubeGraphNewLabelGroupDialog(QWidget* parent = nullptr);
  virtual ~QmitkTubeGraphNewLabelGroupDialog();

  mitk::TubeGraphProperty::LabelGroup* GetLabelGroup();

  protected slots:

    void OnCreateNewLabelGroup();
    void OnAddStandardLabelGroup();
    void OnAddingNewLabelGroup();
    void OnAddingStandardLabelGroup();
    void OnAddingLabel();

protected:

  QVBoxLayout* layout;
  QHBoxLayout* buttonLayout;
  QSpacerItem* spacer;

  QPushButton* newLabelGroupButton;
  QPushButton* standardLabelGroupButton;

  QPushButton* okButton;
  QPushButton* cancleButton;

  QLabel*      labelGroupDescriptionLabel;
  QLineEdit*   labelGroupLineEdit;

  QLabel*      labelDescriptionLabel;
  QLineEdit*   labelLineEdit;
  QPushButton* addLabelButton;
  QListWidget* labelListWidget;

  QTreeWidget* labelGroupTreeWidget;

  LabelGroupType* m_NewLabelGroup;
  std::vector<LabelGroupType*> m_LabelGroupsLiver;
  std::vector<LabelGroupType*> m_LabelGroupsLung;
};

#endif
