/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTubeGraphNewLabelGroupDialog_h
#define QmitkTubeGraphNewLabelGroupDialog_h

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
  ~QmitkTubeGraphNewLabelGroupDialog() override;

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
