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

#include "QmitkTubeGraphDeleteLabelGroupDialog.h"

#include <qabstractitemview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlistwidget.h>



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

const QStringList QmitkTubeGraphDeleteLabelGroupDialog::GetSelectedLabelGroups()
{
  return m_LabelGroupList;
}

void QmitkTubeGraphDeleteLabelGroupDialog::SetLabelGroups(QStringList labelGroups)
{
  for (QStringList::iterator iterator = labelGroups.begin(); iterator != labelGroups.end(); iterator++)
  labelGroupListWidget->addItem(*iterator);
}
