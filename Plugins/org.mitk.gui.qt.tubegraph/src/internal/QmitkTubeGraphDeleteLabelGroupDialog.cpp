/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTubeGraphDeleteLabelGroupDialog.h"

#include <QAbstractItemView>
#include <QPushButton>
#include <QLayout>
#include <QLabel>
#include <QListWidget>



QmitkTubeGraphDeleteLabelGroupDialog::QmitkTubeGraphDeleteLabelGroupDialog(QWidget* parent)
:QDialog(parent)
{

  QDialog::setFixedSize(400, 400);

  auto   layout = new QVBoxLayout(this);
  layout->setMargin(5);
  layout->setSpacing(5);

  descriptionLabel = new QLabel("Which Label Group should be removed?", this);
  layout->addWidget(descriptionLabel);

  labelGroupListWidget = new QListWidget(this);
  labelGroupListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
  layout->addWidget(labelGroupListWidget);

  auto   buttonLayout = new QHBoxLayout();

  deleteButton = new QPushButton("Delete", this);
  buttonLayout->addWidget(deleteButton);
  connect( deleteButton, SIGNAL(clicked()), this, SLOT(OnDeleteLabelGroupClicked()) );

  cancleButton = new QPushButton("Cancle", this);
  buttonLayout->addWidget(cancleButton);
  connect( cancleButton, SIGNAL(clicked()), this, SLOT(reject()) );

  layout->addLayout(buttonLayout);

  deleteButton->setFocus();

}

QmitkTubeGraphDeleteLabelGroupDialog::~QmitkTubeGraphDeleteLabelGroupDialog()
{
    delete descriptionLabel;
    delete labelGroupListWidget;
    delete deleteButton;
    delete cancleButton;
}

void QmitkTubeGraphDeleteLabelGroupDialog::OnDeleteLabelGroupClicked()
{
  //get selected items and save it in vector
  m_LabelGroupList.clear();
  for (int i =0; i < labelGroupListWidget->count(); i++)
  {
    QListWidgetItem* newItem= labelGroupListWidget->item(i);
    if(newItem->isSelected()) //For all checked items
    {
      m_LabelGroupList.push_back(newItem->text());
    }
  }

  this->accept();
}

QStringList QmitkTubeGraphDeleteLabelGroupDialog::GetSelectedLabelGroups()
{
  return m_LabelGroupList;
}

void QmitkTubeGraphDeleteLabelGroupDialog::SetLabelGroups(const QStringList &labelGroups)
{
  for (const auto &labelGroup : labelGroups)
    labelGroupListWidget->addItem(labelGroup);
}
